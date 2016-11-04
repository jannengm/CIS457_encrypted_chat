//
// Created by jannengm on 11/3/16.
//

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFF_SIZE 4096
#define LINE_SIZE 4096

void * get_input (void * arg);

int main( int argc, char * argv[] ) {
    char buffer[BUFF_SIZE], input[LINE_SIZE], msg[LINE_SIZE];
    int err, sockfd, flag, buff_len, file_size, i;
    struct sockaddr_in serveraddr;
    pthread_t child;

    /*Create socket*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
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
    err = connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
    if (err < 0) {
        printf("There was an error with connecting\n");
        exit(1);
    }

    if( pthread_create(&child, NULL, get_input, &sockfd) != 0) {
        printf("Failed to create thread\n");
        return 1;
    }
    pthread_detach(child);

    while(1){
        memset(msg, 0, LINE_SIZE);

        /*Get message from server*/
        recv(sockfd, msg, LINE_SIZE, 0);

        printf("Got from client:\n,%s\n", msg);

        /*Get user input*/
//        printf("Enter message: ");
//        fgets(input, LINE_SIZE, stdin);
//        input[strlen(input) - 1] = 0;
//
//        /*If no error, send requested file name to server*/
//        send(sockfd, input, strlen( input ), 0);

    }
}

void * get_input (void * arg){
    int sockfd = *(int *)arg;
    char to_send[LINE_SIZE];

    memset(to_send, 0, LINE_SIZE);

    /*Get input, send to server*/
    while(strcmp(to_send, "!exit") != 0){
        fgets(to_send, LINE_SIZE, stdin);
        to_send[strlen(to_send) - 1] = 0;
        send(sockfd, to_send, strlen(to_send), 0);

        /*If command is !exit, disconnect*/
        if( strcmp(to_send, "!exit") == 0 ){
            printf("quitting server\n");
            break;
        }

        memset(to_send, 0, LINE_SIZE);
    }
    close(sockfd);
    exit(0);
}