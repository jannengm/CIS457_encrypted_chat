/*******************************************************************************
 * CIS 457 - Project 3: TCP Encrypted Chat Program
 * client_list.h header file
 * @author Mark Jannenga
 *
 * Defines constants and declares functions used in implementing a linked list
 * of clients. Also defines functions used for operations on individual client
 * connections.
 ******************************************************************************/

#ifndef CIS457_ENCRYPTED_CHAT_CLIENT_LIST_H
#define CIS457_ENCRYPTED_CHAT_CLIENT_LIST_H

#include "tcp_chat.h"
#include "encrypt.h"

#define REM_SUCCESS 20  /*Successful client disconnect*/
#define REM_ERROR -20   /*Error in client disconnect*/

/*Linked list struct*/
struct client_list_t{
    struct client_node_t * head;    /*Linked list head pointer*/
    struct client_node_t * tail;    /*Linked list tail pointer*/
};

/*A struct to hold information about an individual client connection*/
struct client_t{
    int id;                     /*Client ID#*/
    int fd;                     /*File descriptor*/
    unsigned char key[KEY_LEN]; /*Symmetric key*/
    unsigned char iv[IV_LEN];   /*Initialization vector*/
};

/*Struct for each node in the linked list*/
struct client_node_t{
    struct client_node_t * next;    /*Pointer to the next element*/
    struct client_node_t * prev;    /*Pointer to the previous element*/
    struct client_t data;           /*Data about the client connection*/
};

typedef struct client_list_t client_list_t;
typedef struct client_node_t client_node_t;
typedef struct client_t client_t;

/*******************************************************************************
 * Initializes a linked list of clients to be an empty list
 *
 * @param list - A pointer to the linked list struct to initialize
 ******************************************************************************/
void init_list(client_list_t * list);

/*******************************************************************************
 * Adds a node to the end of the linked list of clients. Updates the tail
 * appropriately. If the list was previously empty, sets the node to be both
 * head and tail.
 *
 * @param list - A pointer to the linked list of clients
 * @param node - The node to add to the end of the list
 ******************************************************************************/
void push_back(client_list_t * list, client_node_t * node);

/*******************************************************************************
 * Iterates through the linked list and deallocates all dynamically allocated
 * memory for each node, as well as closing all open file descriptors. Sets the
 * list to be empty.
 *
 * @param list - A pointer to the linked list to clear
 ******************************************************************************/
void free_list(client_list_t * list);

/*******************************************************************************
 * Iterates through a linked list of clients and returns a pointer to the first
 * client found with an ID matching the passed id argument, else a NULL pointer.
 *
 * @param list - The linked list to search through
 * @param id - The ID number to look for
 * @return tmp - The pointer to the node with the desired ID number
 ******************************************************************************/
client_node_t * find_client_id(client_list_t * list, int id);

/*******************************************************************************
 * Iterates through a linked list of clients and returns a pointer to the first
 * client found with a file descriptor matching the passed fd argument, else a
 * NULL pointer.
 *
 * @param list - The linked list to search through
 * @param fd - The file descriptor to look for
 * @return tmp - The pointer to the node with the desired file descriptor
 ******************************************************************************/
client_node_t * find_client_fd(client_list_t * list, int fd);

/*******************************************************************************
 * Dynamically allocates memory for a new client_node_t, and initializes its
 * data to be equal to that of the passed client_t. Returns a pointer to the
 * new node.
 *
 * @param data - The data to use to initialize the node
 * @return node - The pointer to the new node
 ******************************************************************************/
client_node_t * init_node(client_t * data);


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
void send_to_target(client_list_t * list, client_t * sender,
                    int target, const char * line);

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
int disconnect_client(client_list_t * list, int id);

/*******************************************************************************
 * Prints the IDs of all the connected clients to a formatted string using
 * sprintf().
 *
 * @param list - The list of clients
 * @param list_str - The location to store the formatted output string
 * @param size - The size of the passed character array
 ******************************************************************************/
void to_string(client_list_t * list, char * list_str, size_t size);

#endif //CIS457_ENCRYPTED_CHAT_CLIENT_LIST_H
