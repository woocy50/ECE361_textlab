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

const char* LOGIN_CMD = "/login";
const char* LOGOUT_CMD = "/logout";
const char* JOINSESSION_CMD = "/joinsession";
const char* LEAVESESSION_CMD = "/leavesession";
const char* CREATESESSION_CMD = "/createsession";
const char* LIST_CMD = "/list";
const char* QUIT_CMD = "/quit";
bool insession = false;

void* get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
	}
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

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
        string2packet(buf, &packet);
        switch(packet.type) {
            case JN_ACK:
                fprintf(stdout, "Join session successful\n");
                insession = true;
                break;
            case JN_NAK:
                fprintf(stdout, "Join session failed\n");
                insession = false;
                break;
            case NS_ACK:
                fprintf(stdout, "Create session successful\n");
                insession = true;
                break;
            case QU_ACK:
                fprintf(stdout, "User id\t\tSession ids\n%s", packet.data);
                break;
            case MESSAGE:
                fprintf(stdout, "%s:\t%s\n", packet.source, packet.data);
                break;
            default:
                fprintf(stderr, "Unexpected packet %s\n", buf);
                close(*sockfd);
                *sockfd = -1;
                return NULL;
        }
    }
}

void login(int* sockfd, pthread_t* recvthread) {
    char *id, *pwd, *ip, *port;
    id = strtok(NULL, " ");
    pwd = strtok(NULL, " ");
    ip = strtok(NULL, " ");
    port = strtok(NULL, " \n");
    if (id == NULL || pwd == NULL || ip == NULL || port == NULL) {
        fprintf(stderr, "Invalid use\n");
        fprintf(stdout, "Usage: /login <id> <pwd> <ip> <port>\n");
        return;
    } else if (*sockfd != -1) {
        fprintf(stderr, "Already logged in\n");
        return;
    }
    
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}
    
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

        if (connect(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(*sockfd);
            perror("client: connect");
            continue;
        }
		break;
	}
    
    if (p == NULL) {
		fprintf(stderr, "client: failed to create a socket\n");
        *sockfd = -1;
		return;
	}

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), s, sizeof s);
    freeaddrinfo(servinfo);

    // ---------- client is connected to server, now send LOGIN ----------
    int numbytes;
    struct message packet;
    packet.type = LOGIN;
    strncpy(packet.source, id, MAX_NAME);
    strncpy(packet.data, pwd, MAX_DATA);
    packet.size = strlen(packet.data);
    
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    packet2string(&packet, buf);
    if ((numbytes = send(*sockfd, buf, BUF_SIZE-1, 0)) == -1) {
        perror("send");
        close(*sockfd);
        *sockfd = -1;
        return;
    }

    memset(buf, 0, BUF_SIZE);
    if ((numbytes = recv(*sockfd, buf, BUF_SIZE-1, 0)) == -1) {
        perror("recv");
        close(*sockfd);
        *sockfd = -1;
        return;
    }

    buf[numbytes] = '\0';
    string2packet(buf, &packet);

    if (packet.type == LO_ACK) {
        if (pthread_create(recvthread, NULL, receive, sockfd) == 0) {
            fprintf(stdout, "Login successful.\n");
        }
    } else if (packet.type == LO_NAK) {
        fprintf(stdout, "Login failed: %s\n", packet.data);
        close(*sockfd);
        *sockfd = -1;
        return;
    } else {
        fprintf(stderr, "Unexpected packet %s\n", buf);
        close(*sockfd);
        *sockfd = -1;
        return;
    }
}

void logout(int* sockfd, pthread_t* recvthread) {
    if (*sockfd == -1) {
        fprintf(stderr, "Already logged out\n");
        return;
    }

    int numbytes;
    struct message packet;
    packet.type = EXIT;
    packet.size = 0;
    
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    packet2string(&packet, buf);
    if ((numbytes = send(*sockfd, buf, BUF_SIZE-1, 0)) == -1) {
        perror("send");
        return;
    }

    if (pthread_cancel(*recvthread)) {
        fprintf(stderr, "Logout failed\n");
        return;
    }

    insession = false;
    close(*sockfd);
    *sockfd = -1;
    fprintf(stdout, "Logout successful\n");
}

void joinsession(int* sockfd) {
    if (*sockfd == -1) {
        fprintf(stderr, "Currently logged out\n");
        return;
    }
    
    if (insession) {
        fprintf(stderr, "Currently in a session\n");
        return;
    }

    char* session_id;
    session_id = strtok(NULL, " ");
    if (session_id == NULL) {
        fprintf(stderr, "Invalid use\n");
        fprintf(stdout, "Usage: /joinsession <session_id>\n");
        return;
    }

    int numbytes;
    struct message packet;
    packet.type = JOIN;
    strncpy(packet.data, session_id, MAX_DATA);
    packet.size = strlen(packet.data);
    
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    packet2string(&packet, buf);
    if ((numbytes = send(*sockfd, buf, BUF_SIZE-1, 0)) == -1) {
        perror("send");
        return;
    }
}

void leavesession(int* sockfd) {
    if (*sockfd == -1) {
        fprintf(stderr, "Currently logged out\n");
        return;
    }
    
    if (!insession) {
        fprintf(stderr, "Currently not in a session\n");
        return;
    }

    int numbytes;
    struct message packet;
    packet.type = LEAVE_SESS;
    packet.size = 0;
    
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    packet2string(&packet, buf);
    if ((numbytes = send(*sockfd, buf, BUF_SIZE-1, 0)) == -1) {
        perror("send");
        return;
    }
    insession = false;
}

void createsession(int* sockfd) {
    if (*sockfd == -1) {
        fprintf(stderr, "Currently logged out\n");
        return;
    }
    
    if (insession) {
        fprintf(stderr, "Already in a session\n");
        return;
    }

    int numbytes;
    struct message packet;
    packet.type = NEW_SESS;
    packet.size = 0;
    
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    packet2string(&packet, buf);
    if ((numbytes = send(*sockfd, buf, BUF_SIZE-1, 0)) == -1) {
        perror("send");
        return;
    }
}

void list(int* sockfd) {
    if (*sockfd == -1) {
        fprintf(stderr, "Currently logged out\n");
        return;
    }

    int numbytes;
    struct message packet;
    packet.type = QUERY;
    packet.size = 0;
    
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    packet2string(&packet, buf);
    if ((numbytes = send(*sockfd, buf, BUF_SIZE-1, 0)) == -1) {
        perror("send");
        return;
    }
}

void send_text(int* sockfd, char* inputbuf) {
    if (*sockfd == -1) {
        fprintf(stderr, "Currently logged out\n");
        return;
    }
    
    if (!insession) {
        fprintf(stderr, "Currently not in a session\n");
        return;
    }

    int numbytes;
    struct message packet;
    packet.type = MESSAGE;
    strncpy(packet.data, inputbuf, MAX_DATA);
    packet.size = strlen(packet.data);
    
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    packet2string(&packet, buf);
    if ((numbytes = send(*sockfd, buf, BUF_SIZE-1, 0)) == -1) {
        perror("send");
        return;
    }
}

int main(int argc, char *argv[]) {
	if (argc != 1) {
		fprintf(stderr, "Invalid use\n");
        fprintf(stdout, "Usage: client\n");
        exit(1);
    }

    int sockfd = -1;
    pthread_t recvthread;
    char inputbuf[BUF_SIZE];
    char* input;
    while (true) {
        memset(inputbuf, 0, BUF_SIZE);
        fgets(inputbuf, BUF_SIZE-1, stdin);
        inputbuf[strcspn(inputbuf, "\n")] = '\0';
        if (*inputbuf == '/') {
            input = strtok(inputbuf, " ");
            if (strcmp(input, LOGIN_CMD) == 0) {
                login(&sockfd, &recvthread);
            } else if (strcmp(input, LOGOUT_CMD) == 0) {
                logout(&sockfd, &recvthread);
            } else if (strcmp(input, JOINSESSION_CMD) == 0) {
                joinsession(&sockfd);
            } else if (strcmp(input, LEAVESESSION_CMD) == 0) {
                leavesession(&sockfd);
            } else if (strcmp(input, CREATESESSION_CMD) == 0) {
                createsession(&sockfd);
            } else if (strcmp(input, LIST_CMD) == 0) {
                list(&sockfd);
            } else if (strcmp(input, QUIT_CMD) == 0) {
                logout(&sockfd, &recvthread);
                break;
            }
        } else {
            send_text(&sockfd, inputbuf);
        }
    }
    return 0;
}