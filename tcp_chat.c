/*******************************************************************************
 * CIS 457 - Project 3: TCP Encrypted Chat Program
 * tcp_chat.c source code
 * @author Mark Jannenga
 *
 * Implements functions declared in tcp_chat.h
 ******************************************************************************/

#include "tcp_chat.h"

/*******************************************************************************
 * Checks for command or target tags at the beginning of a message. If the
 * message begins with '!', checks to see which command it is, and returns the
 * appropriate command code. If no valid code is found, returns INVALID_CODE.
 * If the message begins with '@', checks for a target. If a valid target is
 * found, sets the value of the target parameter appropriately, else returns
 * INVALID_TARGET.
 *
 * @param msg - The message string to check
 * @param target - The location to store the target if there is one
 * @return The appropriate command code or error code.
 ******************************************************************************/
int check_command(char * msg, int * target){

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
        if( strncmp(msg, "@all", 4) == 0 ){
            msg[1] = '-';
            msg[2] = '1';
            msg[3] = ' ';
            *target = BROADCAST;
            return TARGET;
        }
        if( strncmp(msg, "@-1", 3) == 0 ){
            *target = BROADCAST;
            return TARGET;
        }
        int tmp;
        if( sscanf(msg + 1, "%d", &tmp) <= 0){
            //Invalid target
            fprintf(stderr, "Invalid target. Syntax is:\n");
            fprintf(stderr, "@[target id #] or @all\n");
            return INVALID_TARGET;
        }
        else{
            *target = tmp;
            return TARGET;
        }
    }

    /*Else there is no command, only text*/
    else{
        return NO_CODE;
    }
}