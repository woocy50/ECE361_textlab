#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdbool.h>
#include <pthread.h>

#define MAX_NAME 100
#define MAX_DATA 1000
#define BUF_SIZE 2000

struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

enum mtype {LOGIN, LO_ACK, LO_NAK, EXIT, JOIN, JN_ACK, JN_NAK, 
            LEAVE_SESS, NEW_SESS, NS_ACK, MESSAGE, QUERY, QU_ACK,
            PRV_MSG};

char* parsePort(char*);

void packet2string(struct message*, char*);

void string2packet(char*, struct message*);


#endif