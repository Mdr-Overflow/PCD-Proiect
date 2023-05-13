#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


#define BUF_SIZE 62480
#define BUF_NR_DEF 1

char **buf; // Shared buffer
int buf_len = 0; // Current length of buffer
pthread_mutex_t buf_lock; // Mutex to lock the buffer

typedef struct threadArgs{
   char * filename;
   int id;
}threadArgs_t;

// Thread function to read a portion of the file into the buffer
void* read_file(void* arg) {
 
  threadArgs_t *args = (threadArgs_t *)arg;
    
   
    char * filename = (args->filename);
    int id = args->id;

    int start = id * BUF_SIZE;
    // int end = (id+1) * BUF_SIZE;
    char* thread_buf = malloc(sizeof(char) * BUFSIZ);

    
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("Failed to open file\n");
        pthread_exit(NULL);
    }
    fseek(fp, start, SEEK_SET);
    int bytes_read = fread(thread_buf, 1, BUF_SIZE, fp);
    strcpy(buf[id],thread_buf);
    printf("%s", thread_buf);

    pthread_mutex_lock(&buf_lock);
    buf_len += bytes_read;
    pthread_mutex_unlock(&buf_lock);
    fclose(fp);
    free(thread_buf);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

    int i;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

     char *filename = argv[1];

    struct stat st;
    if (stat(filename, &st) != 0) {
        perror("stat");
        return 1;
    }

    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "%s is not a regular file\n", filename);
        return 1;
    }

    printf("%s: %lu bytes\n", filename, st.st_size);


   


    // get cores

    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of virtual cores: %ld\n", num_cores);

    // allocate mem for buff

    pthread_t threads[num_cores];
    int thread_ids[(int)num_cores]; // cringe
    
    // make args  
    threadArgs_t ARGS[num_cores];
    ///
    for (i= 0 ; i<num_cores ; i++){
        ARGS[i].filename = filename;  // when copy char * to char * do not use strcpy , use it only if you want to copy to char **
        ARGS[i].id = i;
    }

    
    


    // re - init matrix

    buf = (char **)malloc(num_cores * sizeof(int *));

      for (i = 0; i < num_cores; i++) 
        buf[i] = (char *)malloc(BUF_SIZE * sizeof(char));
     
    

    // Initialize the mutex
    pthread_mutex_init(&buf_lock, NULL);

    
    // Create the threads
    for (i = 0; i < num_cores; i++) {
        pthread_create(&threads[i], NULL, read_file, (void *) &ARGS[i]);
    }

    // Wait for the threads to finish
    for (i = 0; i < num_cores; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print the contents of the buffer
    for (int i = 0 ; i< num_cores ; i++)
     printf("%.*s", buf_len, buf[i]);

    // Destroy the mutex
    pthread_mutex_destroy(&buf_lock);
    free(buf);
    return 0;
}