
#ifdef __cplusplus
extern "C" {
#endif

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
   int *socketd;
   int id;
} threadArgs_t;

// Thread function to read a portion of the file into the buffer
extern void* read_file(void* arg);
extern int call_readthread(char * filename, int * socketfd  );
extern int call_serverthread(char *filename, int socketfd , int filesize );

#endif

#ifdef __cplusplus
}
#endif

