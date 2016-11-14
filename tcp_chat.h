/*******************************************************************************
 * CIS 457 - Project 3: TCP Encrypted Chat Program
 * tcp_chat.h header file
 * @author Mark Jannenga
 *
 * Defines general constants, command codes, and exit codes used by TCP Chat
 * Server and TCP Chat Client. Declares function prototypes for general use
 * helper functions.
 ******************************************************************************/

#ifndef CIS457_ENCRYPTED_CHAT_TCP_CHAT_H
#define CIS457_ENCRYPTED_CHAT_TCP_CHAT_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>

#define LINE_SIZE 2048  /*Maximum message and input buffer size*/

/*Command codes*/
#define NO_CODE 0       /*No valid code detected*/
#define EXIT 1          /*!exit was received*/
#define LIST 2          /*!list was received*/
#define KILL 3          /*!killk [target] was received*/
#define TARGET 4        /*@[target] message was received*/
#define SHUTDOWN 5      /*!shutdown was received*/

/*Command error codes*/
#define INVALID_TARGET -10  /*Target specified by @[target] does not exist*/
#define INVALID_COMMAND -11 /*Command following '!' does not exist*/

/*Other constants*/
#define BROADCAST -1    /*Target is all clients*/

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
int check_command(char * msg, int * target);

#endif //CIS457_ENCRYPTED_CHAT_TCP_CHAT_H
