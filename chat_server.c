/*******************************************************************************
 * CIS 457 - Project 3: TCP Encrypted Chat Program
 * TCP Chat Server
 * @author Mark Jannenga
 *
 * This program implements an encrypted TCP chat server using the OpenSSL
 * libcrypto library. The server maintains a linked list of connected clients
 * with an ID number, file descriptor, an initialization vector, and a
 * symmetric encryption key for each client. The server decrypts each message
 * it receives using the appropriate symmetric key, re-encrypts it with the
 * symmetric key of the intended recipient, designated at the beginning of each
 * message with the "@[target ID]" tag, and forwards the message. Symmetric
 * keys are created by the clients upon connection and are communicated to the
 * server using the server's public RSA key.
 *
 * The server supports several administrative commands sent from clients:
 *   !exit          -   Removes the client from the list of clients and performs
 *                      an orderly shutdown of that client's connection.
 *   !kill [target] -   Performs an orderly shutdown of the client designated as
 *                      the target.
 *   !list          -   Sends a list of all connected clients to the client that
 *                      sent the command.
 *   !shutdown      -   Causes the server to perform an orderly shutdown of all
 *                      client conections and then exit.
 ******************************************************************************/

#include "tcp_chat.h"
#include "client_list.h"
#include "encrypt.h"

/*Function Prototypes*/
int handle_client(client_t *sender, client_list_t *clients, fd_set *active_set);
void recv_symmetric_key(int sockfd, unsigned char *key, unsigned char *iv);

/*******************************************************************************
 * Server main method. Expects a port number as a command line argument.
 *
 * @param argc - The number of command line arguments
 * @param argv - The list of command line arguments
 * @return
 ******************************************************************************/
int main(int argc, char **argv){
    int sockfd, new_sock, i, num_clients = 0;
    fd_set active_set, read_set;
    socklen_t len;
    struct sockaddr_in serveraddr, clientaddr;
    client_list_t client_list;
    client_t client;
    client_node_t * node;

    init_list(&client_list);

    /*Check for command line arguments*/
    if( argc < 2 ){
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(1);
    }

    /*Create a new master socket to listen for new connections*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ( sockfd < 0 ) {
        fprintf(stderr, "There was an error creating the socket\n");
        exit(1);
    }

    /*Set up server port and address*/
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons( (uint16_t) atoi(argv[1]) );
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    /*bind socket to the address of the server*/
    bind(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr));

    /*Listen on socket for up to 10 connections in queue*/
    listen(sockfd, 10);

    /*Initialize active socket set*/
    FD_ZERO (&active_set);
    FD_SET (sockfd, &active_set);

    /*Loop continuously until killed with ctrl+C */
    while(1){
        read_set = active_set;
        if( select(FD_SETSIZE, &read_set, NULL, NULL, NULL) < 0){
            fprintf(stderr, "Select failure\n");
            exit(1);
        }

        /*Cycle through all sockets in read_set*/
        for(i = 0; i < FD_SETSIZE; i++){

            /*Check if socket has pending input*/
            if( FD_ISSET(i, &read_set) ){

                /*If connection request on master socket (sockfd), create
                 * new socket and add to the active_set*/
                if(i == sockfd){
                    len = sizeof(clientaddr);
                    new_sock = accept(sockfd, (struct sockaddr*) &clientaddr,
                                      &len);
                    if(new_sock < 0){
                        fprintf(stderr, "Error connecting to client\n");
                        exit(1);
                    }
                    printf("Successfully connected to client #%d on socket"
                                   " %d\n", num_clients, new_sock);
                    FD_SET (new_sock, &active_set);

                    /*Send assigned Client ID# to new client*/
                    send(new_sock, &num_clients, sizeof(int), 0);

                    /*Receive encrypted symmetric key*/
                    recv_symmetric_key(new_sock, client.key, client.iv);

                    /*Add the new client to the client list*/
                    client.id = num_clients++;
                    client.fd = new_sock;
                    node = init_node(&client);
                    push_back(&client_list, node);
                }

                /*If client is connected but not on master socket,
                 *it is ready for use*/
                else{
                    client_node_t * sender = find_client_fd(&client_list, i);

                    int command = handle_client( &(sender->data), &client_list,
                                                 &active_set);

                    /*If !exit received, disconnect the client*/
                    if(command == EXIT){
                        FD_CLR(i, &active_set);
                        int exit_code = disconnect_client(&client_list,
                                                          client.id);
                        if(exit_code == REM_ERROR){
                            fprintf(stderr, "Error disconnecting client\n");
                        }
                    }

                    /*If !shutdown received, disconnect all clients and exit*/
                    if(command == SHUTDOWN){
                        client.id = BROADCAST;
                        send_to_target(&client_list, &client, BROADCAST,
                                       "!exit");
                        free_list(&client_list);
                        FD_ZERO (&active_set);
                        exit(0);
                    }
                }
            }
        }
    }
}

/*******************************************************************************
 * This function receives an encrypted message from a connected client, decrypts
 * it and determines what type of message it was, then takes the appropriate
 * action.
 *
 * @param sender - The client that sent the message
 * @param clients - A linked list of all connected clients
 * @param active_set - The fdset of all active file descriptors
 * @return The return value of check_command, designating what type of message
 *         was received. Values defined in tcp_chat.h.
 ******************************************************************************/
int handle_client(client_t *sender, client_list_t *clients, fd_set *active_set){
    char buffer[LINE_SIZE], line[LINE_SIZE];
    unsigned char encrypt_text[LINE_SIZE];
    int command, target, to_clear, encrypt_len = 0, code;

    /*SSL Initialization functions*/
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    /*Clear the input buffer*/
    memset(line, 0, LINE_SIZE);
    memset(buffer, 0, LINE_SIZE);
    memset(encrypt_text, 0, LINE_SIZE);

    /*Get encrypted message from client*/
    encrypt_len = (int)recv(sender->fd, encrypt_text, LINE_SIZE, 0);

    /*Decrypt message*/
    if(encrypt_len > 0)
        decrypt(encrypt_text, encrypt_len, sender->key, sender->iv,
                (unsigned char *)buffer);

    /*Determine what type of message was received (command or message)*/
    command = check_command(buffer, &target);

    /*!exit was received. Handled in main, return EXIT*/
    if(command == EXIT){
        code = EXIT;
    }

    /*!kill [target] was received. Send an !exit command to the specified
     * target, then perform an orderly disconnect. Return KILL.*/
    else if(command == KILL){
        sscanf(buffer, "%s %d", line, &target);
        send_to_target(clients, sender, target, "!exit");
        to_clear = find_client_id(clients, target)->data.fd;
        disconnect_client(clients, target);
        FD_CLR(to_clear, active_set);
        code = KILL;
    }

    /*!shutdown was received. Handled in main, return SHUTDOWN.*/
    else if(command == SHUTDOWN){
        //Disconnect all clients, shut down server
        code = SHUTDOWN;
    }

    /*!list was received. Send a list of all connected clients to the
     * sender. Return LIST*/
    else if(command == LIST){
        //send list to requesting client
        to_string(clients, line, LINE_SIZE);
        client_t server;
        memcpy(&server, sender, sizeof(client_t));
        server.id = BROADCAST;
        send_to_target(clients, &server, sender->id, line);
        code = LIST;
    }

    /*Message with an "@[target ID]" prefix was received. Identify the target
     * and then forward the message on. Return TARGET.*/
    else if(command == TARGET) {
        /*Remove target tag*/
        int i;
        char *msg_ptr = NULL;
        for(i = 0; i < strlen(buffer); i++){
            if( isalpha(buffer[i]) ){
                msg_ptr = buffer + i;
                break;
            }
        }

        if(msg_ptr != NULL){
            strncpy(line, msg_ptr, LINE_SIZE);
            send_to_target(clients, sender, target, line);
        }

        code = TARGET;
    }
    else{
        code = NO_CODE;
    }

    /*SSL Cleanup functions?*/
    EVP_cleanup();
    ERR_free_strings();

    /*Return command code value*/
    return code;
}

/*******************************************************************************
 * Receives an initialization vector (as plain text) and a symmetric key
 * encypted with the server's public RSA key from the client. Decrypts the key
 * and stores the initialization vector and symmetric key in the supplied
 * parameters.
 *
 * @param sockfd - The socket to read the messages in on
 * @param key - The location to store the decrypted symmetric key
 * @param iv - The location to store the initialization vector
 ******************************************************************************/
void recv_symmetric_key(int sockfd, unsigned char *key, unsigned char *iv){
    EVP_PKEY *privkey;
    FILE* privf;
    char *privfilename = "RSApriv.pem";
    int encryptedkey_len = 0;
    unsigned char encrypted_key[ENCRYPTEDKEY_LEN];

    /*SSL Initialization functions*/
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    /*Read in the private RSA key from file*/
    privf = fopen(privfilename,"rb");
    if(privf == NULL){
        fprintf(stderr, "Error opening RSApriv.pem\n");
        return;
    }
    privkey = PEM_read_PrivateKey(privf,NULL,NULL,NULL);

    /*Get plain text Initialization Vector from client*/
    recv(sockfd, iv, IV_LEN, 0);

    /*Receive encrypted key*/
    encryptedkey_len = (int)recv(sockfd, encrypted_key, ENCRYPTEDKEY_LEN, 0);

    /*Decrypt key*/
    rsa_decrypt(encrypted_key, (size_t)encryptedkey_len, privkey, key);

    /*Print received key to stdout*/
    printf("Received key from client:\n");
    printf("IV:\n");
    BIO_dump_fp(stdout, (const char *)iv, IV_LEN);
    printf("\nENCRYPTED KEY:\n");
    BIO_dump_fp(stdout, (const char *)encrypted_key, encryptedkey_len);
    printf("\nDECRYPTED KEY:\n");
    BIO_dump_fp(stdout, (const char *)key, KEY_LEN);
    printf("\n");

    /*SSL Cleanup functions?*/
    EVP_cleanup();
    ERR_free_strings();
    fclose(privf);
}