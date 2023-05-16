#ifndef CHUNKSOCK_H
#define CHUNKSOCK_H

#include <pthread.h>
#include <dirent.h>

// We'll store the arguments for doChunking and sendChunks functions in these structures
typedef struct {
    char* filename;
    int chunkSize;
    int numThreads;
    pthread_mutex_t* mutex;
    pthread_cond_t* cond;
    int* doneChunking;
} DoChunkingArgs;

typedef struct {
    char* dirName;
    int numCores;
    int* socketfd;
    pthread_mutex_t* mutex;
    pthread_cond_t* cond;
    int* doneChunking;
} SendChunksArgs;

void* doChunkingSOCK(void* args);
void* sendChunks(void* args);

#endif /* CHUNKSOCK_H */
