#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>

#include "chunkread.h"
#include "chunkSock.h"
#include "chunkSock.c"



#define BUF_SIZE 8000
#define BUF_NR_DEF 1

char **buf;
int buf_len;
int buf_to_write;
pthread_mutex_t buf_lock;


// Thread function to read a portion of the file into the buffer
void* read_file(void* arg) {
 
  threadArgs_t *args = (threadArgs_t *)arg;
    
   
    FILE* fp  = (args->fp);
    int id = args->id;

    int start = id * BUF_SIZE;
    // int end = (id+1) * BUF_SIZE;
    char* thread_buf = malloc(sizeof(char) * BUFSIZ);
    
    
    fseek(fp, start, SEEK_SET);
    
    int bytes_read = fread(thread_buf, 1, BUF_SIZE, fp);
   // printf("%s", thread_buf);
    
    strcpy(buf[id],thread_buf);
    //printf("%s", thread_buf);

    
    pthread_mutex_lock(&buf_lock);
    buf_len += bytes_read;
    pthread_mutex_unlock(&buf_lock);

   
    free(thread_buf);
    pthread_exit(NULL);
}


// Thread function to read a portion of the file into the buffer
void* write_file(void* arg) {
    threadArgs_t* args = (threadArgs_t*)arg;
    FILE* fp = args->fp;
    int id = args->id;
    int *socket = args->socketd;

    int start = id * BUF_SIZE;

    pthread_mutex_lock(&buf_lock);
    int bytes_remaining = buf_len;
    pthread_mutex_unlock(&buf_lock);

    while (bytes_remaining > 0) {
        int bytes_to_write = bytes_remaining > BUF_SIZE ? BUF_SIZE : bytes_remaining;

        pthread_mutex_lock(&buf_lock);
        fwrite(buf[id], 1, bytes_to_write, fp);

        memmove(buf[id], buf[id] + bytes_to_write, buf_len - bytes_to_write);
        buf_to_write += bytes_to_write;
        pthread_mutex_unlock(&buf_lock);

        bytes_remaining -= bytes_to_write;
    }

    pthread_exit(NULL);
}

// THIS READS AND SENDS 
int call_readthread(char * filename, int * socketfd ) {

    int i;

    

    struct stat st;
    if (stat(filename, &st) != 0) {
        perror("stat");
        exit(1);
    }

    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "%s is not a regular file\n", filename);
        exit(1);
    }

    printf("%s: %lu bytes\n", filename, st.st_size);


    if (st.st_size >= 8589934592 ) { // > 1 GB 

        perror("Error: size too big , size must be smaller or equal to  1GB");
    }
    else {

        if (st.st_size <= 8589)
            {
                // small file time :>

            }
        else {

            // do this  -> refeed 

        }
    }

    
    // get cores
    
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("Failed to open file\n");
        pthread_exit(NULL);
    }


    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of virtual cores: %ld\n", num_cores);

    // Only use as many as needed   
    
         if (st.st_size/BUF_SIZE < num_cores && st.st_size % BUF_SIZE != 0 )
            num_cores = (int) (st.st_size/BUF_SIZE) + 1;
         else if (st.st_size % BUF_SIZE == 0)
         num_cores = (int) (st.st_size/BUF_SIZE);

    printf("Number of usefull threads: %ld\n", num_cores);
    
printf("Number of useful threads: %ld\n", num_cores);





    // Extract filename from path
  char *shortfile = strrchr(filename, '/');
  if (shortfile) {
    shortfile++;  // Skip the '/'
  } else {
    shortfile = filename;
  }

  // Copy filename to another string for modification
  char modifiedFilename[128];
  strncpy(modifiedFilename, filename, sizeof(modifiedFilename) - 1);
  modifiedFilename[sizeof(modifiedFilename) - 1] = '\0';  // Null-terminate the string

  // Replace '.' with 'D'
  for (int i = 0; modifiedFilename[i]; i++) {
    if (modifiedFilename[i] == '.') {
      modifiedFilename[i] = '@';
    }
  }
    


    char dirName[256];
    snprintf(dirName, sizeof(dirName), "ChunksTEMP_%s", modifiedFilename);
    


    // Initialize the mutex and the condition variable
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // Initialize the doneChunking flag to 0
    int doneChunking = 0;

    DoChunkingArgs doChunkingArgs = {filename, BUF_SIZE,
     num_cores, &mutex, &cond, &doneChunking};
    SendChunksArgs sendChunksArgs = {dirName, num_cores , 
    socketfd, &mutex, &cond, &doneChunking};

    pthread_t doChunkingThread, sendChunksThread;
    
    // Create the threads
    pthread_create(&doChunkingThread, NULL, doChunkingSOCK, &doChunkingArgs);
    pthread_create(&sendChunksThread, NULL, sendChunks, &sendChunksArgs);
        
        // IMIDIATE PROBLEM THREADS DON'T FINISH. WHY ?

    // Wait for the threads to finish
    pthread_join(doChunkingThread, NULL);
    pthread_join(sendChunksThread, NULL);

    // Destroy the mutex and the condition variable
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    printf("Done.\n");

    return 0;
}

int call_serverthread(char * filename, int socketfd , int filesize ) {

    int i;
    
    // get cores
    
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("Failed to open file\n");
        pthread_exit(NULL);
    }


    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of virtual cores: %ld\n", num_cores);

    // Only use as many as needed   
    
         if (filesize/BUF_SIZE < num_cores )
            num_cores = (int) (filesize/BUF_SIZE) + 1;

    // printf("Number of usefull threads: %ld\n", num_cores);
    // // allocate mem for buff

    // pthread_t threads[num_cores];
    // int thread_ids[(int)num_cores]; // cringe
    
    // // make args  
    // threadArgs_t ARGS[num_cores];
    // ///


    // for (i= 0 ; i<num_cores ; i++){
    //     ARGS[i].fp = fp;  // when copy char * to char * do not use strcpy , use it only if you want to copy to char **
    //     ARGS[i].id = i;
    // }


    // 

    
    // re - init matrix

    buf = (char **)malloc(num_cores * sizeof(int *));

      for (i = 0; i < num_cores; i++) 
        buf[i] = (char *)malloc(BUF_SIZE * sizeof(char));
     
    

    // Initialize the mutex
    pthread_mutex_init(&buf_lock, NULL);

   //printf("AM AJUNS LA MUTEX pe server \n");
    char * BUF = (char*)malloc(BUF_SIZE);

    while ( buf_to_write < filesize){
        
        
        
        
        // Print the contents of th fclose(fp);e buffer

        int bytes_received = 0;
           bytes_received = recv(socketfd, BUF, BUF_SIZE, 0);
           fwrite(BUF, 1, strlen(BUF) ,fp); // Write the received data to the file
           buf_to_write+=strlen(BUF);   
           printf("RECV ::: = %d",bytes_received);
        // memset(BUF, 0, BUF_SIZE);  
    

    printf("WRITITNG ... buf_remain =%d\n",filesize - buf_to_write);

    if (bytes_received == 0) {
        printf("Client disconnected\n");
    } else if (bytes_received == -1) {
        perror("recv failed");
        break;
    }

        
    }

    // Destroy the mutex
    pthread_mutex_destroy(&buf_lock);
    printf("Done.\n");
    free(buf);
    fclose(fp);
    return 0;
}
