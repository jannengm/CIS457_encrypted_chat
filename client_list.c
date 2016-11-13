//
// Created by jannengm on 11/5/16.
//

#include "client_list.h"
#include "encrypt.h"

void init_list(client_list_t * list){
    list->head = NULL;
    list->tail = NULL;
}

void push_back(client_list_t * list, client_node_t * node){
    if(list->head == NULL){
        list->head = node;
        list->tail = node;
    }
    else {
        list->tail->next = node;
        node->prev = list->tail;
        list->tail = node;
    }
}

void free_list(client_list_t * list){
    client_node_t * tmp = list->head;
    client_node_t * to_delete;

    while(tmp != NULL){
        to_delete = tmp;
        close(tmp->data.fd);
        tmp = tmp->next;
        free(to_delete);
    }

    list->head = NULL;
    list->tail = NULL;
}

client_node_t * init_node(client_t * data){
    client_node_t * node = malloc( sizeof(client_node_t) );
    node->data.id = data->id;
    node->data.fd = data->fd;
    memcpy(node->data.key, data->key, KEY_LEN);
    memcpy(node->data.iv, data->iv, IV_LEN);
    node->next = NULL;
    node->prev = NULL;
    return node;
}

client_node_t * find_client_id(client_list_t * list, int id){
    client_node_t * tmp;
    for(tmp = list->head; tmp != NULL; tmp = tmp->next){
        if(tmp->data.id == id)
            break;
    }
    return tmp;
}

client_node_t * find_client_fd(client_list_t * list, int fd){
    client_node_t * tmp;
    for(tmp = list->head; tmp != NULL; tmp = tmp->next){
        if(tmp->data.fd == fd)
            break;
    }
    return tmp;
}

void send_to_target(client_list_t * list, client_t * sender,
                    int target, const char * line){
    unsigned char encrypt_text[LINE_SIZE];
    int encrypt_len;
    client_node_t * tmp;

    for(tmp = list->head; tmp != NULL; tmp = tmp->next){
        if(tmp->data.id != sender->id &&
                (tmp->data.id == target || target == BROADCAST) ){
//            printf("Sending to client #%d from client #%d\n", tmp->data.id, sender->id);
//            printf("Unencrypted message:\n%s\n", line);
            memset(encrypt_text, 0, LINE_SIZE);

//            printf("Encrypting with key:\nKEY:\n");
//            BIO_dump_fp(stdout, (const char *)tmp->data.key, KEY_LEN);
//            printf("\n");

            encrypt_len = encrypt( (unsigned char*)line, (int)strlen(line),
                                   tmp->data.key, tmp->data.iv, encrypt_text);
//            send(tmp->data.fd, &encrypt_len, sizeof(int), 0);
//            printf("ENCRYPTED MESSAGE:\n");
//            BIO_dump_fp(stdout, (const char *)encrypt_text, encrypt_len);

            send(tmp->data.fd, encrypt_text, (size_t)encrypt_len, 0);
        }
    }
}

int disconnect_client(client_list_t * list, int id){
    client_node_t *tmp, *to_delete;

    /*Remove node from a one element list*/
    if(list->head->data.id == id && list->tail == list->head){
        //close(list->head->data.fd);
        free_list(list);
        return REM_SUCCESS;
    }

    /*Remove node at head of list*/
    if(list->head->data.id == id){
        close(list->head->data.fd);
        to_delete = list->head;
        list->head = list->head->next;
        list->head->prev = NULL;
        free(to_delete);
        return REM_SUCCESS;
    }

    /*Remove node at tail of list*/
    if(list->tail->data.id == id){
        close(list->tail->data.fd);
        to_delete = list->tail;
        list->tail = list->tail->prev;
        list->tail->next = NULL;
        free(to_delete);
        return REM_SUCCESS;
    }

    /*Remove node in the middle of the list*/
    for(tmp = list->head; tmp != NULL; tmp = tmp->next){
        if(tmp->data.id == id){
            close(tmp->data.fd);
            tmp->prev->next = tmp->next;
            tmp->next->prev = tmp->prev;
            free(tmp);
            return REM_SUCCESS;
        }
    }

    return REM_ERROR;
}

void to_string(client_list_t * list, char * list_str, size_t size){
    char line[LINE_SIZE];
    char * ptr = list_str;
    client_node_t * tmp;

    memset(list_str, 0, size);
    memset(line, 0, LINE_SIZE);
    sprintf(line, "List of clients:\n");
    memcpy( list_str, line, strlen(line) );
    ptr += strlen(line);

    for(tmp = list->head; tmp != NULL; tmp = tmp->next){
        memset(line, 0, LINE_SIZE);
        sprintf(line, "Client #%d\n", tmp->data.id);
        if(strlen(list_str) + strlen(line) >= size){
            break;
        }
        memcpy( ptr, line, strlen(line) );
        ptr += strlen(line);
    }
}