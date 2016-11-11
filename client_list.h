//
// Created by jannengm on 11/5/16.
//

/*******************************************************************************
 *
 ******************************************************************************/

#ifndef CIS457_ENCRYPTED_CHAT_CLIENT_LIST_H
#define CIS457_ENCRYPTED_CHAT_CLIENT_LIST_H

#include "tcp_chat.h"
#include "encrypt.h"

#define REM_SUCCESS 20
#define REM_ERROR -20

struct client_list_t{
    struct client_node_t * head;
    struct client_node_t * tail;
};

struct client_t{
    int id;
    int fd;
    unsigned char key[KEY_LEN];
    unsigned char iv[IV_LEN];
};

struct client_node_t{
    struct client_node_t * next;
    struct client_node_t * prev;
    struct client_t data;
};

typedef struct client_list_t client_list_t;
typedef struct client_node_t client_node_t;
typedef struct client_t client_t;

void init_list(client_list_t * list);
void free_list(client_list_t * list);
void push_back(client_list_t * list, client_node_t * node);

client_node_t * find_client_id(client_list_t * list, int id);
client_node_t * find_client_fd(client_list_t * list, int fd);
client_node_t * init_node(client_t * data);

void send_to_target(client_list_t * list, client_t * sender,
                    int target, const char * line);

int disconnect_client(client_list_t * list, int id);

void to_string(client_list_t * list, char * list_str, size_t size);

#endif //CIS457_ENCRYPTED_CHAT_CLIENT_LIST_H
