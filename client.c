// ECE361 Text Conferencing Lab 1
// Submitted on Mar 15, 2022
// Group 42
// Adel Aswad 1005362466
// Nick Woo 1002557271

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "utils/parser.h"

int main(int argc, char *argv[]) {
	if (argc != 1) {
		fprintf(stderr, "Invalid use\n");
        fprintf(stdout, "Usage: client\n");
        exit(1);
    }

    int sockfd, numbytes;
    char buf[MAX_DATA];
    struct addrinfo hints, *servinfo, *p;
    int rf;
    char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
		break;
	}

    if (p == NULL) {
		fprintf(stderr, "client: failed to create a socket\n");
		return 2;
	}

    // ---------- client is now connected to client socket ----------





    // ---------- client is quitting ----------
    close(sockfd);
    return 0;
}

