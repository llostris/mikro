#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define MODE "r"
#define FILE_NAME "page.html"
#define MAX_SIZE 1000

int main(int argc, char* argv[])
{
	FILE* filePointer;
	filePointer = fopen(FILE_NAME, MODE);
	char* buffer;
	char file[MAX_SIZE];

	if(filePointer == NULL)
	{
		fprintf(stderr, "Can't open input file\n");
	} else {
		while(fread(buffer, 1, 1, filePointer) != 0) {
			if(strcmp(buffer, "\t") && strcmp(buffer, "\n")) 
				strcat(file, buffer);
		}
	}
	printf("%s", file);
	fclose(filePointer);
}
