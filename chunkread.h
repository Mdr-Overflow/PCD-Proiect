#ifndef FILE_READER_H
#define FILE_READER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>

#define BUF_SIZE 8000
#define BUF_NR_DEF 1

extern char **buf;
extern int buf_len;
extern pthread_mutex_t buf_lock;

typedef struct threadArgs {
   FILE *fp;
   int id;
} threadArgs_t;

// Thread function to read a portion of the file into the buffer
void* read_file(void* arg);
int call_readthread(char * filename, int * socketfd  );

#endif
