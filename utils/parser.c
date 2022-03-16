#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "parser.h"

char* parsePort(char *portString){
	int port = 0;
	int i = 0;
	char ch = portString[i];
	while(ch != '\0'){
		if(ch < '0' || ch > '9')
			return 0;

		port *= 10;
		port += ch-'0';
		ch = portString[++i];
	}
	return portString;
}