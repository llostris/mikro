#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include "pagehtml.h"


const char* get_msg = "GET / HTTP/1.1 \nHost: [fe80::a00:27ff:fe5c:2c17]";

int is_httpget(unsigned char* msg) {
	unsigned char* to_cmp;
	to_cmp = malloc(strlen(get_msg));
	strncpy(to_cmp, msg, strlen(get_msg)+1);
//	printf("%s\n", (char*) msg); 
	printf("%x %x %x %x\n", msg[0], msg[1], msg[2], msg[3]);
	if ( strstr(to_cmp, "Host: [fe80::a00:27ff:fe5c:2c17]") == NULL )
		return -1;
	return 0;
}

int parse_file(unsigned char* file) {
	printf("HERE... ");
	FILE* filePointer;
	filePointer = fopen(FILE_NAME, MODE);
	unsigned char buffer;
//	char file[MAX_SIZE];
	
	if(filePointer == NULL)
	{
		fprintf(stderr, "Can't open input file\n");
	} else {
		while(fread(&buffer, 1, 1, filePointer) != 0);
	}
	fclose(filePointer);
	return strlen(file);
}
