#ifndef PAGEHTML_H
#define PAGEHTML_H

#define MODE "r"
#define FILE_NAME "page.html"
#define MAX_SIZE 1000

int is_httpget(unsigned char* msg);
int parse_file(unsigned char* file);
int choose_file(unsigned char* name, unsigned char* buff);	/* To be implemented by user */

#endif
