#ifndef CHUNKING_H
#define CHUNKING_H
#include <stdio.h>


void doChunking( char *fullFilename, int chunkSize, int numThreads);
int runReconstruct( char *filename ,char *dirName, unsigned int numChunks, int chunk_size);
int deleteDirectory(const char *path);
void *splitImage(void *arg);
void reconstructFile(char *outputFilename, char *dirName, int numChunks , int CHUNK_SIZE);
void writeChunkToFile(const char* filename,  char* chunkBuffer, size_t chunkSize);

#endif // CHUNKING_H
