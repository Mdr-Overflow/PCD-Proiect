#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>


#define BUF_SIZE 8000
#define BITS_BUF_SIZE 64000
#define BUF_NR_DEF 1

unsigned char **buf = NULL; // Shared buffer
int buf_len = 0; // Current length of buffer
pthread_mutex_t buf_lock; // Mutex to lock the buffer

typedef struct threadArgs{
   FILE * fp;
   int id;
}threadArgs_t;

char *getBufferAsBinaryString(void *in, size_t bufferSize) {
    int pos = 0;
    char result;
    char *bitstring = (char *)malloc(bufferSize * 8 + bufferSize); // Allocate enough space for the bitstring
    
    if (bitstring == NULL) {
        return NULL; // Failed to allocate memory
    }
    
    memset(bitstring, 0, bufferSize * 8 + bufferSize);
    unsigned char *input = (unsigned char *)in;
    
    for (int i = bufferSize - 1; i >= 0; i--) {
        for (int j = 7; j >= 0; j--) {
            if ((input[i] >> j) & 1)
                result = '1';
            else
                result = '0';
            
            bitstring[pos++] = result;
            
            if ((j > 0) && ((j) % 4) == 0) {
                bitstring[pos++] = ' ';
            }
        }
    }
    
    return bitstring;
}

// Thread function to read a portion of the file into the buffer
void* read_file(void* arg) {
 
  threadArgs_t *args = (threadArgs_t *)arg;
    
   
    FILE* fp  = (args->fp);
    int id = args->id;

    int start = id * BUF_SIZE;
 //   int end = (id+1) * BUF_SIZE;
    unsigned char* thread_buf = (unsigned char*)malloc(BUFSIZ);
    
    fseek(fp, start, SEEK_SET);
    
    int bytes_read = fread_unlocked(thread_buf, 1, BUF_SIZE, fp);
   // printf("%s", thread_buf);
    
    // strcpy(buf[id],thread_buf);
    memcpy(buf[id], getBufferAsBinaryString(thread_buf, BITS_BUF_SIZE), strlen((const char*)thread_buf) + 1);
    //printf("%s", thread_buf);

    printf("\nTHREAD %i HAS =  %s\n",id,getBufferAsBinaryString(thread_buf,BUF_SIZE));
    
    pthread_mutex_lock(&buf_lock);
    buf_len += bytes_read;
    pthread_mutex_unlock(&buf_lock);

   
    free(thread_buf);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

    int i;

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


    if (st.st_size >= 8589934592 ) { // > 1 GB 

        perror("Error: size too big , size must be smaller then 1GB");
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
    
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("Failed to open file\n");
        pthread_exit(NULL);
    }



    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of virtual cores: %ld\n", num_cores);

    // Only use as many as needed   
    
         if (st.st_size/BUF_SIZE < num_cores )
            num_cores = (int) (st.st_size/BUF_SIZE) + 1;

    printf("Number of usefull threads: %ld\n", num_cores);
    // allocate mem for buff


    // num_cores = 3;

    pthread_t threads[num_cores];
    int thread_ids[(int)num_cores]; // cringe
    
    // make args  
    threadArgs_t ARGS[num_cores];
    ///


    for (i= 0 ; i<num_cores ; i++){
        ARGS[i].fp = fp;  // when copy char * to char * do not use strcpy , use it only if you want to copy to char **
        ARGS[i].id = i;
    }


    // 

    

    


    // re - init matrix

    buf = (unsigned char **)malloc(num_cores * sizeof(int *));

      for (i = 0; i < num_cores; i++) 
        buf[i] = (unsigned char *)malloc(BITS_BUF_SIZE * sizeof(unsigned char));
     
    

    // Initialize the mutex
    pthread_mutex_init(&buf_lock, NULL);

    int round = 0;
    while (buf_len < st.st_size){
        
        

        // Create the threads
        for (i = 0; i < num_cores; i++) {
            ARGS[i].id = i + round;
            pthread_create(&threads[i], NULL, read_file, (void *) &ARGS[i]);
        }



        // Wait for the threads to finish
        for (i = 0; i < num_cores; i++) {
            pthread_join(threads[i], NULL);
        }

        round +=num_cores;

    }

         FILE *output_file = fopen("output3.png", "wb");
            if (output_file == NULL) {
                perror("Failed to open output file");
                exit(1);
                }
    
        // Print the contents of th fclose(fp);e buffer
        for (int i = 0 ; i< num_cores ; i++) { // should handle case were more then num_cores
         size_t bytes_written = fwrite(buf[i], 1, strlen((const char*)buf[i]), output_file);
         printf("chunk is = %s\n",buf[i]);
        // if (bytes_written < buf_len) {
        //     perror("Failed to write buffer to output file");
        //     exit(1);
        // }
       // fputc('\n', output_file); // Add a newline after each buffer
    }
    
    fclose(output_file);

        
    

    // Destroy the mutex
    pthread_mutex_destroy(&buf_lock);
    printf("Done.\n");
    free(buf);

    fclose(fp);
   
    return 0;
}