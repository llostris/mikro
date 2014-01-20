#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include "pagehtml.h"

int parse_file(char* file) {
	printf("HERE... ");
	FILE* filePointer;
	filePointer = fopen(FILE_NAME, MODE);
	char* buffer;
//	char file[MAX_SIZE];
	
	if(filePointer == NULL)
	{
		fprintf(stderr, "Can't open input file\n");
	} else {
		while(fread(buffer, 1, 1, filePointer) != 0) {
			if(strcmp(buffer, "\t") && strcmp(buffer, "\n")) 
				strcat(file, buffer);
		}
	}
	fclose(filePointer);
	return strlen(file);
}
