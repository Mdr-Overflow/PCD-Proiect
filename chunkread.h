#ifndef FILE_READER_H
#define FILE_READER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>

typedef struct threadArgs {
   FILE *fp;
   int id;
} threadArgs_t;

// Thread function to read a portion of the file into the buffer
void* read_file(void* arg);
int call_readthread(char * filename, int * socketfd  );
int call_serverthread(char *filename, int *socketfd , int filesize );

#endif
