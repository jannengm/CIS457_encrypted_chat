//
// Created by jannengm on 11/3/16.
//

#include "tcp_chat.h"

void * get_input (void * arg);

int id;

int main( int argc, char * argv[] ) {
    char msg[LINE_SIZE];
    int err, sockfd;
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

    recv(sockfd, &id, sizeof(int), 0);

    if( pthread_create(&child, NULL, get_input, &sockfd) != 0) {
        printf("Failed to create thread\n");
        return 1;
    }
    pthread_detach(child);

    while(1){
        memset(msg, 0, LINE_SIZE);

        /*Get message from server*/
        recv(sockfd, msg, LINE_SIZE, 0);

        /*Check for commands*/
        int command = check_command(msg, NULL);

        /*If !exit command was received, shut down gracefully*/
        if(command == EXIT){
            close(sockfd);
            printf("Received exit command. Disconnecting...\n");
            exit(0);
        }

        printf("\n%s\nClient #%d>", msg, id);
        fflush(stdout);
    }
}

void * get_input (void * arg){
    int sockfd = *(int *)arg;
    int target = -1;
    char buffer[LINE_SIZE], to_send[LINE_SIZE];

    memset(to_send, 0, LINE_SIZE);
    memset(buffer, 0, LINE_SIZE);

    /*Get input, send to server*/
    while(strcmp(to_send, "!exit") != 0){
        printf("Client #%d>", id);
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

        send(sockfd, to_send, strlen(to_send), 0);

        /*If command is !exit, disconnect*/
        if(command == EXIT){
            close(sockfd);
            printf("Received exit command. Disconnecting...\n");
            break;
        }

        /*Clear line and buffer for next message*/
        memset(buffer, 0, LINE_SIZE);
        memset(to_send, 0, LINE_SIZE);
    }
    close(sockfd);
    exit(0);
}