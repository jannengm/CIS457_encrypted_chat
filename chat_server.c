/*******************************************************************************
 *
 ******************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CLIENTS 100

struct client_t{
    int id;
    int fd;
};

int handle_client( int fd );

int main(int argc, char ** argv){
    int sockfd, new_sock, i, num_clients = 0;
    fd_set active_set, read_set;
    socklen_t len;
    struct sockaddr_in serveraddr, clientaddr;
    struct client_t client_list[MAX_CLIENTS];

    for(i = 0; i < MAX_CLIENTS; i++){
        client_list[i].id = -1;
        client_list[i].fd = -1;
    }

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
                    printf("Successfully connected to client on socket %d\n", new_sock);
                    FD_SET (new_sock, &active_set);

                    /*Add the new client to the client list*/
                    client_list[num_clients].id = num_clients;
                    client_list[num_clients].fd = new_sock;
                    num_clients++;

                    for(int j = 0; j < num_clients; j++){
                        printf("Client #%d is on socket %d\n", client_list[j].id,
                               client_list[j].fd);
                    }
                }

                /*If client is connected but not on master socket,
                 *it is ready for use*/
                else{

                    /*If exit code of handle_client is CLIENT_DISCONNECT,
                    *the client has disconnected. Close client socket and
                    *remove it from the active_fd_set*/
//                    if( handle_client(i) == 0 ){
//                        printf("Closing client socket\n");
//                        close(i);
//                        FD_CLR(i, &active_set);
//                    }
                }
            }
        }
    }
    return 0;
}

int handle_client( int fd ){
    return 0;
}