// ECE361 Text Conferencing Lab 2
// Submitted on Mar 30, 2022
// Group 42
// Adel Aswad 1005362466
// Nick Woo 1002557271

#include "utils/parser.h"
#include "utils/user.h"

#define BACKLOG 10

struct user users[USRNUM];

void* receive(void* void_sockfd) {
    // handle various ACKs and messages from server

    int sockfd = *(int*) void_sockfd;
    int numbytes;
    struct message packet;

    char buf[BUF_SIZE];
    int i, j;
    char sess[MAX_NAME];
    while (true) {
        memset(buf, 0, BUF_SIZE);
        if ((numbytes = recv(sockfd, buf, BUF_SIZE-1, 0)) == -1) {
            perror("recv");
            return NULL;
        }
        
        // printf("%lu %d\n", pthread_self(), sockfd);

        if (numbytes == 0) {continue;}
        buf[numbytes] = '\0';
        memset(packet.source, 0, MAX_NAME);
        memset(packet.data, 0, MAX_DATA);
        string2packet(buf, &packet);
        // printf("\ttype: %d\n\tsize: %d\n\tsource: %s\n\tdata: %s\n", packet.type, packet.size, packet.source, packet.data);
        switch(packet.type) {
            case LOGIN:
                printf("LOGIN: %s\n", buf);
                for (i = 0; i < USRNUM; i++) {
                    if (strcmp(users[i].id, packet.source) == 0) {
                        if (strcmp(users[i].pwd, packet.data) == 0) {
                            if (users[i].sockfd == -1){
                                memset(packet.source, 0, MAX_NAME);
                                memset(packet.data, 0, MAX_DATA);
                                packet.type = LO_ACK;
                                strcpy(packet.source, "SERVER");
                                strcpy(packet.data, "login okay");
                                packet.size = strlen(packet.data);

                                users[i].sockfd = sockfd;
                            } else {
                                memset(packet.source, 0, MAX_NAME);
                                memset(packet.data, 0, MAX_DATA);
                                packet.type = LO_NAK;
                                strcpy(packet.source, "SERVER");
                                strcpy(packet.data, "already logged in from another device");
                                packet.size = strlen(packet.data);
                            }
                        } else {
                            memset(packet.source, 0, MAX_NAME);
                            memset(packet.data, 0, MAX_DATA);
                            packet.type = LO_NAK;
                            strcpy(packet.source, "SERVER");
                            strcpy(packet.data, "wrong password");
                            packet.size = strlen(packet.data);
                        }
                        break;
                    }
                }
                if (i == USRNUM) {
                    memset(packet.source, 0, MAX_NAME);
                    memset(packet.data, 0, MAX_DATA);
                    packet.type = LO_NAK;
                    strcpy(packet.source, "SERVER");
                    strcpy(packet.data, "id doesn't exist");
                    packet.size = strlen(packet.data);
                }

                memset(buf, 0, BUF_SIZE);
                packet2string(&packet, buf);
                if ((numbytes = send(sockfd, buf, BUF_SIZE-1, 0)) == -1) {
                    perror("send");
                    users[i].sockfd = -1;
		    memset(users[i].sess, 0, MAX_NAME);
                    // users[i].sess = "";
                }
                break;
            
            case EXIT:
                printf("LOGOUT: %s\n", buf);
                for (i = 0; i < USRNUM; i++) {
                    if (users[i].sockfd == sockfd) {
                        users[i].sockfd = -1;
		        memset(users[i].sess, 0, MAX_NAME);
                        // users[i].sess = "";
                    }
                }
                break;
            
            case JOIN:
                printf("JOIN: %s\n", buf);
		memset(sess, 0, MAX_NAME);
                strcpy(sess, packet.data);
                for (i = 0; i < USRNUM; i++) {
                    if (users[i].sockfd == sockfd) {
                        for (j = 0; j < USRNUM; j++) {
                            if (strcmp(users[j].sess, sess) == 0) {
                                memset(packet.source, 0, MAX_NAME);
                                memset(packet.data, 0, MAX_DATA);
                                packet.type = JN_ACK;
                                strcpy(packet.source, "SERVER");
                                sprintf(packet.data, "session %s join okay", sess);
                                packet.size = strlen(packet.data);
				
                                memset(users[i].sess, 0, MAX_NAME);
				strcpy(users[i].sess, sess);
                                break;
                            }
                        }
                        if (j == USRNUM) {
                            memset(packet.source, 0, MAX_NAME);
                            memset(packet.data, 0, MAX_DATA);
                            packet.type = JN_NAK;
                            strcpy(packet.source, "SERVER");
                            strcpy(packet.data, "session doesn't exist");
                            packet.size = strlen(packet.data);
                        }
                        break;
                    }
                }
                memset(buf, 0, BUF_SIZE);
                packet2string(&packet, buf);
                if ((numbytes = send(sockfd, buf, BUF_SIZE-1, 0)) == -1) {
                    perror("send");
                    users[i].sockfd = -1;
		    memset(users[i].sess, 0, MAX_NAME);
                    // users[i].sess = "";
                }
                break;
            
            case LEAVE_SESS:
                printf("LEAVE_SESS: %s\n", buf);
                for (i = 0; i < USRNUM; i++) {
                    if (users[i].sockfd == sockfd) {
			memset(users[i].sess, 0, MAX_NAME);
                        // users[i].sess = "";
                    }
                }
                break;
            
            case NEW_SESS:
                printf("CREATE_SESS: %s\n", buf);
		memset(sess, 0, MAX_NAME);
                strcpy(sess, packet.data);
		printf("sess = %s\n", sess);
                for (i = 0; i < USRNUM; i++) {
                    if (users[i].sockfd == sockfd) {
                        for (j = 0; j < USRNUM; j++) {
                            if (strcmp(users[j].sess, sess) == 0) {
                                memset(packet.source, 0, MAX_NAME);
                                memset(packet.data, 0, MAX_DATA);
                                packet.type = JN_NAK;
                                strcpy(packet.source, "SERVER");
                                strcpy(packet.data, "session already exists");
                                packet.size = strlen(packet.data);
                                break;
                            }
                        }
                        if (j == USRNUM) {
                            memset(packet.source, 0, MAX_NAME);
                            memset(packet.data, 0, MAX_DATA);
                            packet.type = NS_ACK;
                            strcpy(packet.source, "SERVER");
                            sprintf(packet.data, "session %s join okay", sess);
                            packet.size = strlen(packet.data);

                            memset(users[i].sess, 0, MAX_NAME);
			    strcpy(users[i].sess, sess);
                        }
                        break;
                    }
                }
                
		memset(buf, 0, BUF_SIZE);
                packet2string(&packet, buf);
                if ((numbytes = send(sockfd, buf, BUF_SIZE-1, 0)) == -1) {
                    perror("send");
                    users[i].sockfd = -1;
		    memset(users[i].sess, 0, MAX_NAME);
                    // users[i].sess = "";
                }
                break;
            
            case MESSAGE:
                printf("MESSAGE: %s\n", buf);
                for (i = 0; i < USRNUM; i++) {
                    if (users[i].sockfd == sockfd) {
                        break;
                    }
                }
                for (j = 0; j < USRNUM; j++) {
                    if (strcmp(users[j].sess, users[i].sess) == 0) {
                        memset(packet.source, 0, MAX_NAME);
                        strcpy(packet.source, users[i].id);

                        memset(buf, 0, BUF_SIZE);
                        packet2string(&packet, buf);
                        if ((numbytes = send(users[j].sockfd, buf, BUF_SIZE-1, 0)) == -1) {
                            perror("send");
                            users[j].sockfd = -1;
                            memset(users[j].sess, 0, MAX_NAME);
			    // users[j].sess = "";
                        }
                    }
                }                
                break;
            
            case QUERY:
                printf("LIST: %s\n", buf);

                memset(packet.source, 0, MAX_NAME);
                memset(packet.data, 0, MAX_DATA);
                packet.type = QU_ACK;
                strcpy(packet.source, "SERVER");
                for (int i = 0; i < USRNUM; i++) {
                    if (users[i].sockfd != -1) {
                        sprintf(packet.data + strlen(packet.data), 
                                "%-12s%s\n", users[i].id, users[i].sess);
                    }
                }
                packet.size = strlen(packet.data);

                memset(buf, 0, BUF_SIZE);
                packet2string(&packet, buf);
                if ((numbytes = send(sockfd, buf, BUF_SIZE-1, 0)) == -1) {
                    perror("send");
                    users[i].sockfd = -1;
		    memset(users[i].sess, 0, MAX_NAME);
                    // users[i].sess = "";
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

    // initialize users (hardcoded in user.h)
    for (int i = 0; i < USRNUM; i++) {
        strcpy(users[i].id, IDS[i]);
        strcpy(users[i].pwd, PWDS[i]);
        users[i].sockfd = -1;
        memset(users[i].sess, 0, MAX_NAME);
	// users[i].sess = "";
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
            fprintf(stdout, "Client connected at pthread %lu\n", recvthread);
        }

    }

}
