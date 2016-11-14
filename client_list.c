/*******************************************************************************
 * CIS 457 - Project 3: TCP Encrypted Chat Program
 * client_list.c source code
 * @author Mark Jannenga
 *
 * Implements functions declared in client_list.h
 ******************************************************************************/

#include "client_list.h"

/*******************************************************************************
 * Initializes a linked list of clients to be an empty list
 *
 * @param list - A pointer to the linked list struct to initialize
 ******************************************************************************/
void init_list(client_list_t * list){
    list->head = NULL;
    list->tail = NULL;
}

/*******************************************************************************
 * Adds a node to the end of the linked list of clients. Updates the tail
 * appropriately. If the list was previously empty, sets the node to be both
 * head and tail.
 *
 * @param list - A pointer to the linked list of clients
 * @param node - The node to add to the end of the list
 ******************************************************************************/
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

/*******************************************************************************
 * Iterates through the linked list and deallocates all dynamically allocated
 * memory for each node, as well as closing all open file descriptors. Sets the
 * list to be empty.
 *
 * @param list - A pointer to the linked list to clear
 ******************************************************************************/
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

/*******************************************************************************
 * Dynamically allocates memory for a new client_node_t, and initializes its
 * data to be equal to that of the passed client_t. Returns a pointer to the
 * new node.
 *
 * @param data - The data to use to initialize the node
 * @return node - The pointer to the new node
 ******************************************************************************/
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

/*******************************************************************************
 * Iterates through a linked list of clients and returns a pointer to the first
 * client found with an ID matching the passed id argument, else a NULL pointer.
 *
 * @param list - The linked list to search through
 * @param id - The ID number to look for
 * @return tmp - The pointer to the node with the desired ID number
 ******************************************************************************/
client_node_t * find_client_id(client_list_t * list, int id){
    client_node_t * tmp;
    for(tmp = list->head; tmp != NULL; tmp = tmp->next){
        if(tmp->data.id == id)
            break;
    }
    return tmp;
}

/*******************************************************************************
 * Iterates through a linked list of clients and returns a pointer to the first
 * client found with a file descriptor matching the passed fd argument, else a
 * NULL pointer.
 *
 * @param list - The linked list to search through
 * @param fd - The file descriptor to look for
 * @return tmp - The pointer to the node with the desired file descriptor
 ******************************************************************************/
client_node_t * find_client_fd(client_list_t * list, int fd){
    client_node_t * tmp;
    for(tmp = list->head; tmp != NULL; tmp = tmp->next){
        if(tmp->data.fd == fd)
            break;
    }
    return tmp;
}

/*******************************************************************************
 * Takes a message string (line), encrypts it, and sends it to the client
 * designated by the target parameter. If the target parameter is set to
 * BROADCAST, sends the message to all clients except the original sender.
 *
 * @param list - The list of clients to send messages to
 * @param sender - The sender of the message
 * @param target - The intended target of the message
 * @param line - The message to encrypt and send
 ******************************************************************************/
void send_to_target(client_list_t * list, client_t * sender, int target,
                    const char * line){
    unsigned char encrypt_text[LINE_SIZE];
    int encrypt_len;
    client_node_t * tmp;

    for(tmp = list->head; tmp != NULL; tmp = tmp->next){
        if(tmp->data.id != sender->id &&
                (tmp->data.id == target || target == BROADCAST) ){
            memset(encrypt_text, 0, LINE_SIZE);

            encrypt_len = encrypt( (unsigned char*)line, (int)strlen(line),
                                   tmp->data.key, tmp->data.iv, encrypt_text);

            send(tmp->data.fd, encrypt_text, (size_t)encrypt_len, 0);
        }
    }
}

/*******************************************************************************
 * Removes a client, specified by the id parameter, from the list of clients,
 * frees the dynamically allocated memory for that client, and closes the file
 * descriptor associated with it. Retuns REM_SUCCESS if the removal was
 * successful, else REM_ERROR.
 *
 * @param list - The list of clients
 * @param id - The ID of the client to remove
 * @return - REM_SUCCESS or REM_ERROR
 ******************************************************************************/
int disconnect_client(client_list_t * list, int id){
    client_node_t *tmp, *to_delete;

    /*Remove node from a one element list*/
    if(list->head->data.id == id && list->tail == list->head){
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

/*******************************************************************************
 * Prints the IDs of all the connected clients to a formatted string using
 * sprintf().
 *
 * @param list - The list of clients
 * @param list_str - The location to store the formatted output string
 * @param size - The size of the passed character array
 ******************************************************************************/
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