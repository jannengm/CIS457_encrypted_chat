//
// Created by jannengm on 11/4/16.
//

#ifndef CIS457_ENCRYPTED_CHAT_TCP_CHAT_H
#define CIS457_ENCRYPTED_CHAT_TCP_CHAT_H

#define MAX_ID_LEN 10
#define MAX_CLIENTS 100
#define BUFF_SIZE 2048
#define LINE_SIZE 2048

/*Command codes*/
#define NO_CODE 0
#define EXIT 1
#define LIST 2
#define KILL 3
#define TARGET 4
#define SHUTDOWN 5

/*Command error codes*/
#define INVALID_TARGET -10
#define INVALID_COMMAND -11

/*Other constants*/
#define BROADCAST -1

/*General errors*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>

int check_command(char * msg, int * target);

#endif //CIS457_ENCRYPTED_CHAT_TCP_CHAT_H
