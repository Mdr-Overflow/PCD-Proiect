#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>

#define MAX_CHUNK_SIZE 8000// Define your desired chunk size here
#define NUM_THREADS 8   // Define the number of threads to use

int rem_size = 0;

struct ThreadData {
  FILE *file;
  int chunkIndex;
  unsigned char *buffer;
};

void writeChunkToFile(const char* filename, unsigned char* chunkBuffer, size_t chunkSize) {
    FILE* outputFile = fopen(filename, "wb");
    if (outputFile == NULL) {
        printf("Failed to open output file for writing.\n");
        return;
    }
    
    size_t bytesWritten = fwrite(chunkBuffer, sizeof(unsigned char), chunkSize, outputFile);
    if (bytesWritten != chunkSize) {
        printf("Failed to write chunk data to file.\n");
    }
    
    fclose(outputFile);
}


void *splitImage(void *arg) {
  struct ThreadData *threadData = (struct ThreadData *)arg;
  FILE *file = threadData->file;
  int chunkIndex = threadData->chunkIndex;

  // int actual_chunk_size = threadData ->actualSize;
  unsigned char *buffer = threadData->buffer;

  if (rem_size >= MAX_CHUNK_SIZE){

  fseek(file, chunkIndex * MAX_CHUNK_SIZE, SEEK_SET);

  size_t bytesRead = fread(buffer, 1, MAX_CHUNK_SIZE, file);
  if (bytesRead < MAX_CHUNK_SIZE) {
    printf("Error reading image chunk\n");
    pthread_exit(NULL);
  }
  

  printf("Chunk %d read\n", chunkIndex);

  fseek(file, (long)bytesRead,
        SEEK_CUR); // Set file position back to the original position
  }

  else {
  
  size_t bytesRead = fread(buffer, 1, rem_size, file);



  }

  rem_size -= MAX_CHUNK_SIZE;
  pthread_exit(NULL);
}

int main(int argc, char **argv) {

   if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

     char *filename = argv[1];

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



  FILE *file =
      fopen(argv[1], "rb"); // Change "input.jpg" to your image file name
  if (file == NULL) {
    printf("Failed to open image file\n");
    return 1;
  }

  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  int numChunks = (fileSize + MAX_CHUNK_SIZE - 1) / MAX_CHUNK_SIZE;

  unsigned char *buffer = (unsigned char *)malloc(numChunks * MAX_CHUNK_SIZE);
  if (buffer == NULL) {
    printf("Failed to allocate memory for chunks\n");
    fclose(file);
    return 1;
  }

  pthread_t threads[NUM_THREADS];
  struct ThreadData threadData[NUM_THREADS];

  int threadChunkCount = numChunks / NUM_THREADS;
  int remainingChunks = numChunks % NUM_THREADS;

  int currentChunkIndex = 0;

  rem_size = fileSize;

  for (int i = 0; i < NUM_THREADS; i++) {
    int chunkCount = threadChunkCount;

    if ( rem_size == 0) break;

    if (remainingChunks > 0) {
      chunkCount++;
      remainingChunks--;
    }

    threadData[i].file = file;
    threadData[i].chunkIndex = currentChunkIndex;

    if (rem_size > MAX_CHUNK_SIZE){

        threadData[i].buffer = buffer + (currentChunkIndex * MAX_CHUNK_SIZE);
    }

    else {
    threadData[i].buffer = buffer + (currentChunkIndex * rem_size);
      
    }

    
   
    rem_size -= MAX_CHUNK_SIZE;
    currentChunkIndex += chunkCount;

    if (pthread_create(&threads[i], NULL, splitImage, (void *)&threadData[i]) !=
        0) {
      printf("Failed to create thread\n");
      free(buffer);
      fclose(file);
      return 1;
    }
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      printf("Failed to join thread\n");
      free(buffer);
      fclose(file);
      return 1;
    }
  }

  // Print the content of the buffer
  printf("Buffer content:\n");
  for (int i = 0; i < numChunks; i++) {
    printf("Chunk %d:\n", i);
    for (int j = 0; j < MAX_CHUNK_SIZE; j++) {
      printf("%02X ", buffer[i * MAX_CHUNK_SIZE + j]);
    }
    printf("\n\n");
  }
    // while (fileSize != 0) {

    // if (fileSize >= MAX_CHUNK_SIZE){  
    writeChunkToFile("output11.png", buffer, fileSize);
    // fileSize -= MAX_CHUNK_SIZE;
    // }
    // else {
    // writeChunkToFile("output11.png", buffer, fileSize);
    // }

    // }
    
  free(buffer);
  fclose(file);
  return 0;
}