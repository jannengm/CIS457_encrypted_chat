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

#define BUFF_SIZE 4096
#define LINE_SIZE 4096

int main( int argc, char * argv[] ) {
    char buffer[BUFF_SIZE], in_file[LINE_SIZE], out_file[LINE_SIZE];
    int err, sockfd, flag, buff_len, file_size, i;
    struct sockaddr_in serveraddr;
    FILE *fp;

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
    serveraddr.sin_port = htons(atoi(argv[1]));
    serveraddr.sin_addr.s_addr = inet_addr(argv[2]);

    /*Connect to server*/
    err = connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
    if (err < 0) {
        printf("There was an error with connecting\n");
        exit(1);
    }
}