/*******************************************************************************
 *
 ******************************************************************************/

#include "tcp_chat.h"
#include "client_list.h"
#include "encrypt.h"

int handle_client( client_t *sender, client_list_t *clients, fd_set *active_set );
void recv_symmetric_key(int sockfd, unsigned char *key, unsigned char *iv);

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
                    new_sock = accept(sockfd, (struct sockaddr*) &clientaddr, &len);
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

//                    printf("Stored in client list:\n");
//                    printf("IV:\n");
//                    BIO_dump_fp(stdout, (const char *)client.iv, IV_LEN);
//                    printf("\nDECRYPTED KEY:\n");
//                    BIO_dump_fp(stdout, (const char *)client.key, KEY_LEN);
//                    printf("\n");
                }

                /*If client is connected but not on master socket,
                 *it is ready for use*/
                else{
//                    client = find_client_fd(&client_list, i)->data;
                    client_node_t * sender = find_client_fd(&client_list, i);

//                    printf("Retrieved in client list:\n");
//                    printf("IV:\n");
//                    BIO_dump_fp(stdout, (const char *)client.iv, IV_LEN);
//                    printf("\nDECRYPTED KEY:\n");
//                    BIO_dump_fp(stdout, (const char *)client.key, KEY_LEN);
//                    printf("\n");

                    int command = handle_client( &(sender->data), &client_list, &active_set);

                    /*If !exit received, disconnect the client*/
                    if(command == EXIT){
                        FD_CLR(i, &active_set);
                        int exit_code = disconnect_client(&client_list, client.id);
                        if(exit_code == REM_ERROR){
                            fprintf(stderr, "Error disconnecting client\n");
                        }
//                        FD_CLR(i, &active_set);
                    }

                    /*If !shutdown received, disconnect all clients and exit*/
                    if(command == SHUTDOWN){
                        client.id = BROADCAST;
                        send_to_target(&client_list, &client, BROADCAST, "!exit");
                        free_list(&client_list);
                        FD_ZERO (&active_set);
                        exit(0);
                    }
                }
            }
        }
    }
}

int handle_client( client_t *sender, client_list_t *clients, fd_set *active_set ){
//    static int runs;
//    printf("handle_client call %d\n", runs++);
//    printf("Handling packet from client #%d on socket #%d\n", sender->id, sender->fd);
//    sleep(1);

    char buffer[BUFF_SIZE], line[LINE_SIZE];
    unsigned char encrypt_text[LINE_SIZE];
    //int buff_len, file_size;
    int command, target, to_clear, encrypt_len = 0, code;

    /*SSL Initialization functions?*/
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    /*Clear the input buffer*/
    memset(line, 0, LINE_SIZE);
    memset(buffer, 0, BUFF_SIZE);
    memset(encrypt_text, 0, LINE_SIZE);

    /*Get encrypted message from client*/
    //recv(sender->fd, &encrypt_len, sizeof(int), 0);
    encrypt_len = (int)recv(sender->fd, encrypt_text, LINE_SIZE, 0);

//    printf("Received encrypted packet of length %d\n", encrypt_len);
//    printf("ENCRYPTED MESSAGE:\n");
//    BIO_dump_fp(stdout, (const char *)encrypt_text, encrypt_len);
//    printf("\n");
//
//    printf("Attempting to decrypt with key:\n");
//    BIO_dump_fp(stdout, (const char *)sender->key, KEY_LEN);
//    sleep(5);

    /*Decrypt message*/
    if(encrypt_len > 0)
        decrypt(encrypt_text, encrypt_len, sender->key, sender->iv,
                (unsigned char *)buffer);

//    printf("Got from client:\n%s\n", buffer);

    command = check_command(buffer, &target);

    if(command == EXIT){
        //remove this client from list
        code = EXIT;
    }
    else if(command == KILL){
        //send !exit to specified client
        sscanf(buffer, "%s %d", line, &target);
        send_to_target(clients, sender, target, "!exit");
        to_clear = find_client_id(clients, target)->data.fd;
        disconnect_client(clients, target);
        FD_CLR(to_clear, active_set);
        code = KILL;
    }
    else if(command == SHUTDOWN){
        //Disconnect all clients, shut down server
        code = SHUTDOWN;
    }
    else if(command == LIST){
        //send list to requesting client
        to_string(clients, line, LINE_SIZE);
        client_t server;
        memcpy(&server, sender, sizeof(client_t));
        server.id = BROADCAST;
        send_to_target(clients, &server, sender->id, line);
        code = LIST;
    }
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

    return code;
}
void recv_symmetric_key(int sockfd, unsigned char *key, unsigned char *iv){
    EVP_PKEY *privkey;
    FILE* privf;
    char *privfilename = "RSApriv.pem";
    int encryptedkey_len = 0;
    unsigned char encrypted_key[ENCRYPTEDKEY_LEN];

    /*SSL Initialization functions?*/
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    privf = fopen(privfilename,"rb");
    if(privf == NULL){
        fprintf(stderr, "Error opening RSApriv.pem\n");
        return;
    }
    privkey = PEM_read_PrivateKey(privf,NULL,NULL,NULL);

    /*Get plain text Initialization Vector from client*/
    recv(sockfd, iv, IV_LEN, 0);

    /*Get length of encrypted key*/
//    recv(sockfd, &encryptedkey_len, sizeof(int), 0);

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