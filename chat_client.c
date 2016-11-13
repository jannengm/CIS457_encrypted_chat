//
// Created by jannengm on 11/3/16.
//

#include "tcp_chat.h"
#include "encrypt.h"
#include "client_list.h"

void * get_input (void * arg);
void create_symmetric_key(unsigned char *key, unsigned char *iv);
void send_symmetric_key(int sockfd, unsigned char *key, unsigned char *iv);

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

    /*Create new symmetric key and send to the server*/
    create_symmetric_key(client.key, client.iv);

    /*Print key debug information to stdout*/
    printf("Created new symmetric key:\nKEY:\n");
    BIO_dump_fp(stdout, (const char *)client.key, KEY_LEN);
    printf("\nIV:\n");
    BIO_dump_fp(stdout, (const char *)client.iv, IV_LEN);
    printf("\n");

    send_symmetric_key(client.fd, client.key, client.iv);

    if( pthread_create(&child, NULL, get_input, &client) != 0) {
        printf("Failed to create thread\n");
        return 1;
    }
    pthread_detach(child);

    while(1){
        memset(msg, 0, LINE_SIZE);
        memset(encrypt_text, 0, LINE_SIZE);


//        printf()
        /*Get message from server*/
//        recv(client.fd, &encrypt_len, sizeof(int), 0);
//        recv(client.fd, encrypt_text, encrypt_len, 0);
        encrypt_len = (int)recv(client.fd, encrypt_text, LINE_SIZE, 0);

//        printf("Received encrypted packet of length %d\n", encrypt_len);
//        printf("ENCRYPTED MESSAGE:\n");
//        BIO_dump_fp(stdout, (const char *)encrypt_text, encrypt_len);
//        printf("\n");

        /*Decrypt message*/
        if(encrypt_len > 0) {
            decrypt_len = decrypt(encrypt_text, encrypt_len, client.key, client.iv,
                                  (unsigned char *) msg);
//            printf("\n%s", msg);
            /*There are some garbage characters at the end of the decrypted message*/
            int i;
            for(i = 0; i < strlen(msg); i++){
                if( !isprint( msg[i] ) && msg[i] != '\n' ){
                    msg[i] = '\0';
                    break;
                }
            }
//            printf("Decrypted message length: %d\n", decrypt_len);
//            msg[decrypt_len] = '\0';
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
            //Could cause buffer overflow, fix later
            snprintf(to_send, LINE_SIZE, "@%d %s", target, buffer);
        }
        else{
            strncpy(to_send, buffer, LINE_SIZE);
        }

        /*Encrypt message*/
        encrypt_len = encrypt( (unsigned char *)to_send, strlen(to_send),
                               client.key, client.iv, encrypt_text);

        /*Display encrypted message*/
//        printf("SENDING ENCRYPTED MESSAGE:\n");
//        BIO_dump_fp(stdout, (const char *)encrypt_text, encrypt_len);
//        printf("\n");

        /*Send size of encrypted message*/
//        send(client.fd, &encrypt_len, sizeof(int), 0);

        /*Send encrypted message*/
        send(client.fd, encrypt_text, encrypt_len, 0);

        /*Decrypt message and output*/
//        memset(buffer, 0, LINE_SIZE);
//        decrypt(encrypt_text, encrypt_len, client.key, client.iv, (unsigned char *)buffer);
//        printf("Decrypted message:\n%s\n", buffer);

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
    close(client.fd);
    exit(0);
}

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

/*Create symmetric key, encrypt with public key, and send to server*/
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

    /*Tell server length of encrypted key*/
//    send(sockfd, &encryptedkey_len, sizeof(int), 0);

    /*Send encrypted symmetric key*/
    send(sockfd, encrypted_key, ENCRYPTEDKEY_LEN, 0);

    /*SSL Cleanup functions?*/
    EVP_cleanup();
    ERR_free_strings();
    fclose(pubf);
}