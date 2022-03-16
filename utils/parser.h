#ifndef PARSER_H
#define PARSER_H

#define MAX_NAME 100
#define MAX_DATA 1000

struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

char* parsePort(char *portString);




#endif