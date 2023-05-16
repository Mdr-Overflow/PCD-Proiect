#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include <dirent.h>

#include "chunking.h"
#include "checkFiles.h"
#include "chunkSock.h"

#include "chunking.c"
#include "checkFiles.c"
 

 int doneChunking = 0;
 // We'll store the arguments for doChunking and sendChunks functions in these structures

typedef struct Node {
    char filename[256];
    struct Node* next;
} Node;




void* doChunkingSOCK(void* args) {
    DoChunkingArgs* doChunkingArgs = (DoChunkingArgs*)args;
    char* filename = doChunkingArgs->filename;
    int chunkSize = doChunkingArgs->chunkSize;
    int numThreads = doChunkingArgs->numThreads;
    
    printf("CHUNK SOCK THREAD ENTERED. \n");

    doChunking(filename,chunkSize,numThreads);

    pthread_mutex_lock(doChunkingArgs->mutex);
    *doChunkingArgs->doneChunking = 1;
    doneChunking = 1;   
    pthread_cond_signal(doChunkingArgs->cond);
    pthread_mutex_unlock(doChunkingArgs->mutex);
    

    
    printf("CHUNK SOCK THREAD DONE. \n");

    pthread_exit(NULL);
}

void* sendChunks(void* args) {
     SendChunksArgs* sendChunksArgs = (SendChunksArgs* )args;
    const char* dirName = sendChunksArgs->dirName;
    int numCores = sendChunksArgs->numCores;
    int* socketfd = sendChunksArgs->socketfd;
   

     DIR* dir;
    struct dirent* ent;
    Node* head = NULL;

    // Open the directory
    dir = opendir(dirName);
    if (dir == NULL) {
        perror("Failed to open directory");
        pthread_exit(NULL);
    }
    

    while (!doneChunking) {

        printf("Waiting...\n");
    
    }

    // Gather all filenames into a list
    while ((ent = readdir(dir)) != NULL) {
        Node* new_node = malloc(sizeof(Node));
        strcpy(new_node->filename, ent->d_name);
        new_node->next = head;
        head = new_node;
    }

    // closedir(dir);

    // Loop over the list of files
    Node* current = head;
    while (current != NULL) {
        // Check if there are at least numCores * 2 files and send them over the socket
        // If successful, delete the file
        printf("Processing file %s\n", current->filename);
        check_send_and_delete_files(dirName, numCores, socketfd);

        Node* to_free = current;
        current = current->next;
        free(to_free);
    }

    // After sending all files, delete the directory
    deleteDirectory(dirName);


        printf("I GET HERE SOMETIMES \n");
        // If done chunking and no more files in directory, exit thread
        pthread_mutex_lock(sendChunksArgs->mutex);
        doneChunking = *sendChunksArgs->doneChunking;
        pthread_mutex_unlock(sendChunksArgs->mutex);

        if (doneChunking && (ent == NULL)) {
            closedir(dir);
       
        }

    

    pthread_exit(NULL);
}
