#ifndef PARSER_H
#define PARSER_H

#define MAX_NAME 100
#define MAX_DATA 1000
#define USERS "userlist.txt"

struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

enum mtype {LOGIN, LO_ACK, LO_NAK, EXIT, JOIN, JN_ACK, JN_NAK, 
            LEAVE_SESS, NEW_SESS, NS_ACK, MESSAGE, QUERY, QU_ACK};

char* parsePort(char *portString);




#endif