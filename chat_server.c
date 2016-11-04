/*******************************************************************************
 *
 ******************************************************************************/

#include "tcp_chat.h"

struct client_t{
    int id;
    int fd;
};

typedef struct client_t client_t;

int handle_client( int fd, client_t * clients, int num_clients );
//int check_command(const char * msg, int * target);

int main(int argc, char ** argv){
    int sockfd, new_sock, i, num_clients = 0;
    fd_set active_set, read_set;
    socklen_t len;
    struct sockaddr_in serveraddr, clientaddr;
    client_t client_list[MAX_CLIENTS];

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

//                    int j;
//                    for(j = 0; j < num_clients; j++){
//                        printf("Client #%d is on socket %d\n", client_list[j].id,
//                               client_list[j].fd);
//                    }
                }

                /*If client is connected but not on master socket,
                 *it is ready for use*/
                else{
                    int command = handle_client(i, client_list, num_clients);

                    /*If !exit received, disconnect the client*/
                    if(command == EXIT){
                        close(i);
                        FD_CLR(i, &active_set);
                        int k;
                        for(k = 0; k < num_clients; k++){
                            if(client_list[k].fd == i){
                                client_list[k].fd = -1;
                                client_list[k].id = -1;
                            }
                        }
                    }

                    /*If !shutdown received, disconnect all clients and exit*/
                    if(command == SHUTDOWN){
                        int m;
                        char * tmp = "!exit";
                        for(m = 0; m < num_clients; m++){
                            if( client_list[m].fd > 0 ){
                                send(client_list[m].fd, tmp, 5, 0);
                                close(client_list[m].fd);
                                FD_CLR(client_list[m].fd, &active_set);
                            }
                        }
                        exit(0);
                    }
                }
            }
        }
    }
    return 0;
}

int handle_client( int fd, client_t * clients, int num_clients ){
    char buffer[BUFF_SIZE], line[LINE_SIZE];
    //int buff_len, file_size;
    int command, target;

    /*Clear the input buffer*/
    memset(line, 0, LINE_SIZE);
    memset(buffer, 0, BUFF_SIZE);

    /*Get file name from client*/
    recv(fd, buffer, BUFF_SIZE, 0);

    printf("Got from client:\n%s\n", buffer);

    command = check_command(buffer, &target);

    if(command == EXIT){
        //remove this client from list
        return EXIT;
    }
    if(command == KILL){
        //send !exit to specified client
        return KILL;
    }
    if(command == SHUTDOWN){
        //Disconnect all clients, shut down server
        return SHUTDOWN;
    }
    if(command == LIST){
        //send list to requesting client
        return LIST;
    }
    if(command == TARGET) {
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
        }
        else{
            return TARGET;
        }

        /*If target is BROADCAST, broadcast to all clients*/
        if (target == BROADCAST) {
            int j;
            for (j = 0; j < num_clients; j++) {
                if (clients[j].fd != fd && clients[j].fd > 0) {
                    send(clients[j].fd, line, strlen(line), 0);
                }
            }
        }
        /*Else send to the specified target*/
        else{
            int k;
            for (k = 0; k < num_clients; k++) {
                if (clients[k].id == target) {
                    send(clients[k].fd, line, strlen(line), 0);
                }
            }
        }

        return TARGET;
    }

    return 0;
}