//
// Created by jannengm on 11/4/16.
//

#include "tcp_chat.h"

int check_command(const char * msg, int * target){

    /*Check target to prevent segfaults when not needed*/
    if (target == NULL){
        int tmp;
        target = &tmp;
    }

    /*If message starts with '!' check for valid commands*/
    if(msg[0] == '!'){
        if( strncmp(msg, "!exit", 5) == 0 ){
            return EXIT;
        }
        if( strncmp(msg, "!list", 5) == 0 ){
            return LIST;
        }
        if( strncmp(msg, "!shutdown", 9) == 0 ){
            return SHUTDOWN;
        }
        if( strncmp(msg, "!kill", 5) == 0 ){
            return KILL;
        }
        else{
            fprintf(stderr, "Invalid command. Valid commands are:\n");
            fprintf(stderr, "!exit - disconnect this client\n");
            fprintf(stderr, "!list - get a list of clients\n");
            fprintf(stderr, "!kill [target] - disconnect the target\n");
            fprintf(stderr, "!shutdown - shuts down the server\n");
            return INVALID_COMMAND;
        }
    }

        /*If message starts with '@' set target*/
    else if(msg[0] == '@'){
        if( strncmp(msg, "@all", 4) == 0 || strncmp(msg, "@-1", 3) == 0){
            *target = BROADCAST;
            return TARGET;
        }
        char target_id[MAX_ID_LEN];
        int i, j = 0;
        for(i = 1; i < MAX_ID_LEN; i++){
            if( isdigit(msg[i]) ){
                target_id[j++] = msg[i];
            }
        }
        if(j <= 0){
            //Invalid target
            fprintf(stderr, "Invalid target. Syntax is:\n");
            fprintf(stderr, "@[target id #] or @all\n");
            return INVALID_TARGET;
        }
        else{
            *target = atoi(target_id);
            return TARGET;
        }
    }

        /*Else there is no command, only text*/
    else{
        return NO_CODE;
    }
}