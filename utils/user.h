#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 100
#define USRNUM 3

struct user {
    char id[MAX_NAME];
    char pwd[MAX_NAME];
    int sockfd;
    char sess[MAX_NAME];
};

char* IDS[USRNUM] = {"aswadamoh", "woochang", "admin"};
char* PWDS[USRNUM] = {"abc", "123", "password"};

#endif
