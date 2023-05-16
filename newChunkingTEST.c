#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>


#define CHUNK_SIZE 8000
#define NUM_THREADS 20

struct ThreadData {
  char *filename;
  int chunkIndex;
  char *dirName;
};


int deleteDirectory(const char *path) {
  DIR *d = opendir(path);
  size_t path_len = strlen(path);
  int ret = -1;

  if (d) {
    struct dirent *p;

    ret = 0;

    while (!ret && (p = readdir(d))) {
      int r2 = -1;
      char *buf;
      size_t len;

      // Skip the names "." and ".." as we don't want to recurse on them.
      if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
        continue;
      }

      len = path_len + strlen(p->d_name) + 2;
      buf = malloc(len);

      if (buf) {
        struct stat statbuf;

        snprintf(buf, len, "%s/%s", path, p->d_name);

        if (!stat(buf, &statbuf)) {
          if (S_ISDIR(statbuf.st_mode)) {
            r2 = deleteDirectory(buf);
          } else {
            r2 = unlink(buf);
          }
        }

        free(buf);
      }

      ret = r2;
    }

    closedir(d);
  }

  if (!ret) {
    ret = rmdir(path);
  }

  return ret;
}




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



void reconstructFile(char *outputFilename, char *dirName, int numChunks) {
  FILE *outputFile = fopen(outputFilename, "wb");
  if (outputFile == NULL) {
    printf("Failed to open output file for writing.\n");
    return;
  }

  unsigned char buffer[CHUNK_SIZE];
  for (int i = 0; i < numChunks; i++) {
    char chunkFilename[128];
    snprintf(chunkFilename, sizeof(chunkFilename), "%s/output_%d.jpg", dirName, i);

    FILE *chunkFile = fopen(chunkFilename, "rb");
    if (chunkFile == NULL) {
      printf("Failed to open chunk file for reading.\n");
      return;
    }

    while (!feof(chunkFile)) {
      size_t bytesRead = fread(buffer, 1, CHUNK_SIZE, chunkFile);
      fwrite(buffer, 1, bytesRead, outputFile);
    }

    fclose(chunkFile);
  }

  fclose(outputFile);
}

int runReconstruct(const char * dirName , unsigned int numChunks ){


reconstructFile("reconstructed.jpg", dirName, numChunks);

  //  if (deleteDirectory(dirName) != 0) {
  //   printf("Failed to delete directory\n");
  //   return 1;
  // }


  return 0;

}


void *splitImage(void *arg) {
  struct ThreadData *threadData = (struct ThreadData *)arg;
  char *filename = threadData->filename;
  int chunkIndex = threadData->chunkIndex;
  char *dirName = threadData->dirName;
  
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    printf("Failed to open image file\n");
    pthread_exit(NULL);
  }
  
  fseek(file, chunkIndex * CHUNK_SIZE, SEEK_SET);
  
  unsigned char buffer[CHUNK_SIZE];
  size_t bytesRead = fread(buffer, 1, CHUNK_SIZE, file);
  if (bytesRead <= 0) {
    printf("Error reading image chunk\n");
    fclose(file);
    pthread_exit(NULL);
  }
  
  char outputFilename[128];
  snprintf(outputFilename, sizeof(outputFilename), "%s/output_%d.jpg", dirName, chunkIndex);
  writeChunkToFile(outputFilename, buffer, bytesRead);
  
  printf("Chunk %d read and written to file\n", chunkIndex);
  fclose(file);
  pthread_exit(NULL);
}

int main(int argc, char **argv) {

  

  
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }



    char *fullFilename = argv[1];


  // Extract filename from path
  char *filename = strrchr(fullFilename, '/');
  if (filename) {
    filename++;  // Skip the '/'
  } else {
    filename = fullFilename;
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

  char dirName[128];
  snprintf(dirName, sizeof(dirName), "ChunksTEMP_%s", modifiedFilename);

  // Create directory
  struct stat st = {0};
  if (stat(dirName, &st) == -1) {
    mkdir(dirName, 0700);
  }

  FILE *file = fopen(argv[1], "rb");
  if (file == NULL) {
    printf("Failed to open image file\n");
    return 1;
  }

  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fclose(file);

  int numChunks = (fileSize + CHUNK_SIZE - 1) / CHUNK_SIZE;
  int numThreads = numChunks < NUM_THREADS ? numChunks : NUM_THREADS;

 

  // pthread_t threads[numThreads];
  // struct ThreadData threadData[numThreads];
  
  pthread_t threads[NUM_THREADS];
    struct ThreadData threadData[NUM_THREADS];

    for (int chunkIndex = 0; chunkIndex < numChunks;) {
    int i;
    for (i = 0; i < NUM_THREADS && chunkIndex < numChunks; i++, chunkIndex++) {
      threadData[i].filename = fullFilename;
      threadData[i].chunkIndex = chunkIndex;
      threadData[i].dirName = dirName;
      if (pthread_create(&threads[i], NULL, splitImage, (void *)&threadData[i]) != 0) {
        printf("Failed to create thread\n");
        return 1;
      }
    }

    for (int j = 0; j < i; j++) {
      if (pthread_join(threads[j], NULL) != 0) {
        printf("Failed to join thread\n");
        return 1;
      }
    }
  }


  

  return 0;
}