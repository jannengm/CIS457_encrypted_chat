/*******************************************************************************
 * CIS 457 - Project 3: TCP Encrypted Chat Program
 * TCP Chat Server
 * author: Mark Jannenga
 *
 * This program implements an encrypted TCP chat client using the OpenSSL
 * libcrypto library. Upon connection to the server, the client randomly
 * generates a symmetric key, encrypts it with the server's RSA public key, and
 * sends it to the server. The client creates seperate threads to handle user
 * input and to receive messages from the client. The client takes user input
 * from stdin, encrypts it with the symmetric key, then sends it to the server.
 * The client receives encrypted messages from the server, decrypts them with
 * the symmetric key, and prints them to stdout.
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
 *
 * The client sends messages to all other connected clients by default. To
 * specify a particular client to message, @[target ID] is entered as a prefix
 * to the message. The client remembers what the last target was and continues
 * sending messages to that target until a new target is given. To go back to
 * broadcasting to all clients, @all or @-1 is entered as a prefix.
 ******************************************************************************/

#include "tcp_chat.h"
#include "encrypt.h"
#include "client_list.h"

/*Function prototypes*/
void * get_input (void * arg);
void create_symmetric_key(unsigned char *key, unsigned char *iv);
void send_symmetric_key(int sockfd, unsigned char *key, unsigned char *iv);

/*******************************************************************************
 * Client main method. Expects a port number and the IPv4 address of the server
 * as command line arguments.
 *
 * @param argc - The number of command line arguments
 * @param argv - The list of command line arguments
 * @return
 ******************************************************************************/
int main( int argc, char * argv[] ) {
    char msg[LINE_SIZE];
    unsigned char encrypt_text[LINE_SIZE];
    int err, encrypt_len, decrypt_len;
    struct sockaddr_in serveraddr;
    pthread_t child;
    client_t client;

    /*Create socket*/
    client.fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client.fd < 0) {
        printf("There was an error creating the socket\n");
        exit(1);
    }

    /*Check for command line arguments*/
    if (argc < 3) {
        printf("Invalid arguments. ");
        printf("Args should be of form: ");
        printf("[port] [IP address]\n");
        exit(1);
    }

    /*Get server port number*/
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons( (uint16_t)atoi(argv[1]) );
    serveraddr.sin_addr.s_addr = inet_addr(argv[2]);

    /*Connect to server*/
    err = connect(client.fd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
    if (err < 0) {
        printf("There was an error with connecting\n");
        exit(1);
    }

    /*Receive assigned Client ID number from server*/
    recv(client.fd, &(client.id), sizeof(int), 0);

    /*Create new symmetric key*/
    create_symmetric_key(client.key, client.iv);

    /*Print key debug information to stdout*/
    printf("Created new symmetric key:\nKEY:\n");
    BIO_dump_fp(stdout, (const char *)client.key, KEY_LEN);
    printf("\nIV:\n");
    BIO_dump_fp(stdout, (const char *)client.iv, IV_LEN);
    printf("\n");

    /*Send the symmetric key to the server*/
    send_symmetric_key(client.fd, client.key, client.iv);

    /*Create child thread to handle user input*/
    if( pthread_create(&child, NULL, get_input, &client) != 0) {
        printf("Failed to create thread\n");
        return 1;
    }
    pthread_detach(child);

    while(1){
        /*Clear buffers*/
        memset(msg, 0, LINE_SIZE);
        memset(encrypt_text, 0, LINE_SIZE);

        /*Get message from server*/
        encrypt_len = (int)recv(client.fd, encrypt_text, LINE_SIZE, 0);

        /*Decrypt message*/
        if(encrypt_len > 0) {
            decrypt_len = decrypt(encrypt_text, encrypt_len, client.key, client.iv,
                                  (unsigned char *) msg);

            /*Remove garbage characters at the end of the decrypted message*/
            int i;
            for(i = 0; i < strlen(msg); i++){
                if( !isprint( msg[i] ) && msg[i] != '\n' ){
                    msg[i] = '\0';
                    break;
                }
            }
        }
        else{
            printf("\nReceived zero length packet\n");
            sleep(1);
        }

        /*Check for commands*/
        int command = check_command(msg, NULL);

        /*If !exit command was received, shut down gracefully*/
        if(command == EXIT){
            close(client.fd);
            printf("\nReceived exit command. Disconnecting...\n");
            exit(0);
        }

        printf("\n%s\nClient #%d>", msg, client.id);
        fflush(stdout);
    }
}

/*******************************************************************************
 * Gets user input from stdin, encrypts it using the symmetric key, then sends
 * the encrypted message to the server.
 *
 * @param arg - A client_t struct containing the key, initialization vector,
 *              client ID, and file descriptor
 * @return
 ******************************************************************************/
void * get_input (void * arg){
    client_t client = *(client_t *)arg;
    int target = -1, encrypt_len;
    char buffer[LINE_SIZE], to_send[LINE_SIZE];
    unsigned char encrypt_text[LINE_SIZE];

    memset(to_send, 0, LINE_SIZE);
    memset(buffer, 0, LINE_SIZE);

    /*Get input, send to server*/
    while(strcmp(to_send, "!exit") != 0){
        printf("Client #%d>", client.id);
        fgets(buffer, LINE_SIZE, stdin);
        buffer[strlen(buffer) - 1] = 0;

        /*Check for commands*/
        int command = check_command(buffer, &target);

        /*If invalid command entered, do not send*/
        if(command < 0){
            memset(buffer, 0, LINE_SIZE);
            continue;
        }

        /*If no target specified, add current target*/
        if(command == NO_CODE){
            snprintf(to_send, LINE_SIZE, "@%d %s", target, buffer);
        }
        else{
            strncpy(to_send, buffer, LINE_SIZE);
        }

        /*Encrypt message*/
        encrypt_len = encrypt( (unsigned char *)to_send, strlen(to_send),
                               client.key, client.iv, encrypt_text);

        /*Send encrypted message*/
        send(client.fd, encrypt_text, encrypt_len, 0);

        /*If command is !exit, disconnect*/
        if(command == EXIT){
            close(client.fd);
            printf("Received exit command. Disconnecting...\n");
            break;
        }

        /*Clear line and buffer for next message*/
        memset(buffer, 0, LINE_SIZE);
        memset(to_send, 0, LINE_SIZE);
    }

    /*Once !exit is received or entered, break close the file descriptor
     * and exit*/
    close(client.fd);
    exit(0);
}

/*******************************************************************************
 * Randomly generates an initialization vector and a symmetric key, then stores
 * them in the passed parameters
 *
 * @param key - The location to store the symmetric key
 * @param iv - The location to store the initialization vector
 ******************************************************************************/
void create_symmetric_key(unsigned char *key, unsigned char *iv){
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    /*Create key and Initialization Vector*/
    RAND_bytes(key,KEY_LEN);
    RAND_pseudo_bytes(iv,IV_LEN);

    /*SSL Cleanup functions?*/
    EVP_cleanup();
    ERR_free_strings();
}

/*******************************************************************************
 * Takes a symmetric key and initialization vector as arguments, sends the
 * initialization vector as plain text to the server using the sockfd file
 * descriptor, encrypts the key using the server's RSA public key, then sends
 * the encrypted key.
 *
 * @param sockfd - The server socket to send messages to
 * @param key - The symmetric key to encrypt and send
 * @param iv - The initialization vector to send as plain text
 ******************************************************************************/
void send_symmetric_key(int sockfd, unsigned char *key, unsigned char *iv){
    int encryptedkey_len = 0;
    EVP_PKEY *pubkey;
    FILE* pubf;
    unsigned char encrypted_key[ENCRYPTEDKEY_LEN];
    char *pubfilename = "RSApub.pem";

    /*SSL Initialization functions?*/
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    /*Open server public key from file*/
    pubf = fopen(pubfilename,"rb");
    if(pubf == NULL){
        fprintf(stderr, "Error opening RSApub.pem\n");
        return;
    }
    pubkey = PEM_read_PUBKEY(pubf,NULL,NULL,NULL);

    /*Encrypt symmetric key*/
    encryptedkey_len = rsa_encrypt(key, KEY_LEN, pubkey, encrypted_key);

    /*Print key debug information to stdout*/
    printf("ENCRYPTED KEY:\n");
    BIO_dump_fp(stdout, (const char *)encrypted_key, encryptedkey_len);
    printf("\n");

    /*Send plain text Initialization Vector*/
    send(sockfd, iv, IV_LEN, 0);

    /*Send encrypted symmetric key*/
    send(sockfd, encrypted_key, ENCRYPTEDKEY_LEN, 0);

    /*SSL Cleanup functions?*/
    EVP_cleanup();
    ERR_free_strings();
    fclose(pubf);
}