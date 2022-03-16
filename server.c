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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdbool.h>
#include <pthread.h>

#include "utils/parser.h"

#define BACKLOG 10

void* receive(void* void_sockfd) {
    // handle various ACKs and messages from server

    int* sockfd = (int*) void_sockfd;
    int numbytes;
    struct message packet;

    char buf[BUF_SIZE];
    while (true) {
        memset(buf, 0, BUF_SIZE);
        if ((numbytes = recv(*sockfd, buf, BUF_SIZE-1, 0)) == -1) {
            perror("recv");
            return NULL;
        }
        
        if (numbytes == 0) {continue;}
        buf[numbytes] = '\0';
        memset(packet.source, 0, MAX_NAME);
        memset(packet.data, 0, MAX_DATA);
        string2packet(buf, &packet);
        printf("\ttype: %d\n\tsize: %d\n\tsource: %s\n\tdata: %s\n", packet.type, packet.size, packet.source, packet.data);
        switch(packet.type) {
            case LOGIN:
                printf("LOGIN: %s\n", buf);
                packet.type = LO_ACK;
                strcpy(packet.source, "SERVER");
                strcpy(packet.data, "login okay");
                packet.size = strlen(packet.data);

                memset(buf, 0, BUF_SIZE);
                packet2string(&packet, buf);
                if ((numbytes = send(*sockfd, buf, BUF_SIZE-1, 0)) == -1) {
                    perror("send");
                    return NULL;
                }
                break;
            
            case EXIT:
                printf("LOGOUT: %s\n", buf);
                break;
            
            case JOIN:
                printf("JOIN: %s\n", buf);
                packet.type = JN_ACK;
                strcpy(packet.source, "SERVER");
                strcpy(packet.data, "1");
                packet.size = strlen(packet.data);

                memset(buf, 0, BUF_SIZE);
                packet2string(&packet, buf);
                if ((numbytes = send(*sockfd, buf, BUF_SIZE-1, 0)) == -1) {
                    perror("send");
                    return NULL;
                }
                break;
            
            case LEAVE_SESS:
                printf("LEAVE_SESS: %s\n", buf);
                break;
            
            case NEW_SESS:
                printf("NEW_SESS: %s\n", buf);
                packet.type = LO_ACK;
                strcpy(packet.source, "SERVER");
                strcpy(packet.data, "2");
                packet.size = strlen(packet.data);

                memset(buf, 0, BUF_SIZE);
                packet2string(&packet, buf);
                if ((numbytes = send(*sockfd, buf, BUF_SIZE-1, 0)) == -1) {
                    perror("send");
                    return NULL;
                }
                break;
            
            case MESSAGE:
                printf("MESSAGE: %s\n", buf);
                if ((numbytes = send(*sockfd, buf, BUF_SIZE-1, 0)) == -1) {
                    perror("send");
                    return NULL;
                }
                break;
            
            case QUERY:
                printf("LIST: %s\n", buf);
                packet.type = QU_ACK;
                strcpy(packet.source, "SERVER");
                strcpy(packet.data, "list results");
                packet.size = strlen(packet.data);

                memset(buf, 0, BUF_SIZE);
                packet2string(&packet, buf);
                if ((numbytes = send(*sockfd, buf, BUF_SIZE-1, 0)) == -1) {
                    perror("send");
                    return NULL;
                }
                break;
            
            default:
                fprintf(stderr, "Unexpected packet %s\n", buf);
                return NULL;
        }
    }
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Invalid use\n");
        fprintf(stdout, "Usage: server <TCP port number>\n");
        exit(1);
    }

    char* port = parsePort(argv[1]);
    if (port == 0){
		fprintf(stderr, "Invalid use\n");
        fprintf(stdout, "Usage: server <TCP port number>\n");
	}

    int sockfd, clientfd;
    char buf[MAX_DATA];
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rv));
		return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        close(sockfd);
        perror("listen");
        exit(1);
    }
    
    // ---------- server is now connected to server socket ----------
    while (true) {
        sin_size = sizeof their_addr;
        clientfd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (clientfd == -1) {
            perror("accept");
            continue;
        }

        pthread_t recvthread;
        if (pthread_create(&recvthread, NULL, receive, &clientfd) == 0) {
            fprintf(stdout, "Client connected.\n");
        }
        


    }

}