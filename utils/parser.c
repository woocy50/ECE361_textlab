#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

char* parsePort(char* portString) {
	int port = 0;
	int i = 0;
	char ch = portString[i];
	while (ch != '\0') {
		if(ch < '0' || ch > '9')
			return 0;

		port *= 10;
		port += ch-'0';
		ch = portString[++i];
	}
	return portString;
}

void packet2string(struct message *packet, char* msg) {
	memset(msg, 0, BUF_SIZE);
	sprintf(msg, "%d:%d:%s:%s", packet->type, packet->size, packet->source, packet->data);
}

void string2packet(char* msg, struct message *packet) {
	char temp[MAX_DATA];
	memset(temp, 0, MAX_DATA);

	int idx, idx2;
    
    idx2 = (int) (strchr(msg, ':') - msg);
    strncpy(temp, msg, idx2);
    packet->type = atoi(temp);
    idx = idx2 + 1;
	
	idx2 = (int) (strchr(msg + idx, ':') - msg);
    strncpy(temp, msg + idx, idx2 - idx);
    packet->size = atoi(temp);
    idx = idx2 + 1;

	idx2 = (int) (strchr(msg + idx, ':') - msg);
    // strncpy(temp, msg + idx, idx2 - idx);
	// pack->source = temp;
    memcpy(&(packet->source), msg + idx, idx2 - idx);
    idx = idx2 + 1;

	// idx2 = (int) (strchr(msg + idx, ':') - msg);
    // strncpy(temp, msg + idx, idx2 - idx);
	// pack->data = temp;
	memcpy(&(packet->data), msg + idx, packet->size);
}