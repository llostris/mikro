#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include "pagehtml.h"

const char* get_msg = "GET / HTTP/1.1 \nHost: [fe80::a00:27ff:fe5c:2c17]";

int is_httpget(unsigned char* msg) {
	unsigned char* to_cmp;
	to_cmp = malloc(strlen(get_msg) + 20);
	strncpy(to_cmp, msg, strlen(get_msg) + 20);
	if (strstr( msg, "GET") == NULL )
		return -1;	

	if ( strstr(to_cmp, "Host: [fe80::a00:27ff:fe5c:2c17]") == NULL )
		return -1;
	return 0;
}

int parse_file(unsigned char* file) {
	FILE* filePointer;
	filePointer = fopen(FILE_NAME, MODE);
	unsigned char buffer;
	int count;
	count = 0;
	if(filePointer == NULL)
	{
		fprintf(stderr, "Can't open input file\n");
	} else {
		while(fread(&buffer, 1, 1, filePointer) != 0)
			file[count++] = buffer;
	}
	fclose(filePointer);
	return count;
}


int parse_file2(unsigned char* file, char* filename) {
	FILE* filePointer;
	filePointer = fopen(filename, MODE);
	unsigned char buffer;
	int count;
	count = 0;
	if(filePointer == NULL)
	{
		fprintf(stderr, "Can't open input file\n");
	} else {
		while(fread(&buffer, 1, 1, filePointer) != 0)
			file[count++] = buffer;
	}
	fclose(filePointer);
	return count;
}


int choose_file(unsigned char* req, unsigned char* buff) {
	unsigned char* to_cmp;
	to_cmp = malloc(300);
	strncpy(to_cmp, req, 300);
	//printf("STRSTR: %s\n", strstr(to_cmp, "link.html"));
	int result;
	if ( strstr(to_cmp, "link.html") != NULL ) {
		free(to_cmp);
		return parse_file2(buff, "link.html");
	} else {
		free(to_cmp);
		return parse_file2(buff, "page.html");	
	}
	return 0;
}
