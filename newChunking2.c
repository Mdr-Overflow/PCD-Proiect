#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define CHUNK_SIZE 8000 // Define your desired chunk size here
#define NUM_THREADS 4   // Define the number of threads to use

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
  unsigned char *buffer = threadData->buffer;

  fseek(file, chunkIndex * CHUNK_SIZE, SEEK_SET);

  size_t bytesRead = fread(buffer, 1, CHUNK_SIZE, file);
  if (bytesRead < CHUNK_SIZE) {
    printf("Error reading image chunk\n");
    pthread_exit(NULL);
  }

  printf("Chunk %d read\n", chunkIndex);

  fseek(file, -(long)bytesRead,
        SEEK_CUR); // Set file position back to the original position

  pthread_exit(NULL);
}

int main(int argc, char **argv) {
  FILE *file =
      fopen(argv[1], "rb"); // Change "input.jpg" to your image file name
  if (file == NULL) {
    printf("Failed to open image file\n");
    return 1;
  }

  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  int numChunks = (fileSize + CHUNK_SIZE - 1) / CHUNK_SIZE;

  unsigned char *buffer = (unsigned char *)malloc(numChunks * CHUNK_SIZE);
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

  for (int i = 0; i < NUM_THREADS; i++) {
    int chunkCount = threadChunkCount;
    if (remainingChunks > 0) {
      chunkCount++;
      remainingChunks--;
    }

    threadData[i].file = file;
    threadData[i].chunkIndex = currentChunkIndex;
    threadData[i].buffer = buffer + (currentChunkIndex * CHUNK_SIZE);

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

  int lastChunkSize = fileSize % CHUNK_SIZE;
if (lastChunkSize > 0) {
fseek(file, -(long)lastChunkSize, SEEK_END);
size_t bytesRead = fread(buffer + (numChunks - 1) * CHUNK_SIZE, 1, lastChunkSize, file);
if (bytesRead < lastChunkSize) {
printf("Error reading last image chunk\n");
free(buffer);
fclose(file);
return 1;
}
printf("Last chunk read\n");
}

//   Print the content of the buffer
  printf("Buffer content:\n");
  for (int i = 0; i < numChunks; i++) {
    printf("Chunk %d:\n", i);
    for (int j = 0; j < CHUNK_SIZE; j++) {
      printf("%02X ", buffer[i * CHUNK_SIZE + j]);
    }
    printf("\n\n");
  }
    writeChunkToFile("output11.png", buffer, fileSize );
  free(buffer);
  fclose(file);
  return 0;
}