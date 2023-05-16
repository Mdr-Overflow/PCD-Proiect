#include <asm-generic/errno.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "chunking.h"

int check_send_and_delete_files(const char* dir, int num, int * socketfd) {
    DIR *directory;
    struct dirent *file;
    int count = 0;
    int ignore =0;

    directory = opendir(dir);



    if (directory == NULL) {
        printf("Failed to open directory\n");
    }

    // Count files in the directory
    // here seg faults - dir not found 
    while ((file = readdir(directory)) != NULL) {
    char *ext = strrchr(file->d_name, '.');

    if (ext != NULL) {
        if (strcmp(ext, ".jpg") == 0 ||
            strcmp(ext, ".jpeg") == 0 ||
            strcmp(ext, ".png") == 0 ||
            strcmp(ext, ".gif") == 0 ||
            strcmp(ext, ".bmp") == 0) {
            count++;
        }
    }
    }


    printf("dir = %s \n" , dir);    
    printf("count = %d \n" , count);
    printf("num = %d \n" , num);

    // // If count of files matches "num"
    // if(count == num){

        printf(" GOT TO SENDER ...\n");

                rewinddir(directory);  // Reset directory stream to beginning

                    while ((file = readdir(directory)) != NULL) {
                   
                  if(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0){
                    continue;            
                  }

                

             // AICI SE FUTE
              
                printf("FILE = %s",file->d_name);
                char filepath[1024];
                snprintf(filepath, sizeof(filepath), "%s/%s", dir, file->d_name);

                // Open the file
                int fd = open(filepath, O_RDONLY);
                if (fd == -1) {
                    perror("open");
            
                }

                // Get the size of the file
                struct stat stat_buf;
                if (fstat(fd, &stat_buf) == -1) {
                    perror("fstat");
                
                }


                // get index of file
                char *last_underscore = strrchr(file->d_name, '_');
                char *dot = strchr(last_underscore, '.');

                if(last_underscore != NULL && dot != NULL) {
                    // +1 to skip the underscore and dot-last_underscore-1 to exclude the dot
                    char index_str[dot - last_underscore];
                    strncpy(index_str, last_underscore + 1, dot - last_underscore - 1);
                    index_str[dot - last_underscore - 1] = '\0'; // Null terminate the string

                    int index = atoi(index_str); // Convert to integer

                    // Send the index first
                    char index_to_send[BUFSIZ];
                    sprintf(index_to_send, "%d", index);
                    ssize_t bytes_sent1 = write(*socketfd, index_to_send, strlen(index_to_send) + 1); // +1 for the null terminator
                    if (bytes_sent1 == -1) {
                        perror("write file index to socket ");
                    }
                } else {
                    printf("Could not extract index from file name: %s\n", file->d_name);
                }


                // IS IT ACCESSING IT CORRECTLY ?

                // Open the file
                // FILE* fileptr = fopen(filepath, "r");
                // if (fileptr == NULL) {
                //     perror("fopen");
                //     continue;
                // }

                // // Read the file and write its contents to stdout
                // char buffer[BUFSIZ];
                // size_t bytes_read;
                // while ((bytes_read = fread(buffer, 1, sizeof(buffer), fileptr)) > 0) {
                //     fwrite(buffer, 1, bytes_read, stdout);
                // }

                // fclose(fileptr);



                // Send the file over the socket
             char buffer[BUFSIZ];
            ssize_t bytes_read;
            while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
                ssize_t total_sent = 0;  // Total amount of data sent
                while (total_sent < bytes_read) {  // While there's data left to send
                    ssize_t bytes_sent = send(*socketfd, buffer + total_sent, bytes_read - total_sent, 0);
                    if (bytes_sent == -1) {
                        perror("send");
                        printf("FILE ERROR SENDING = %s \n", filepath);
                        break;
                    }
                    else {
                        total_sent += bytes_sent;  // Update total_sent
                        printf("FILE part sent = %s, bytes sent: %ld, total sent: %ld of %ld\n", filepath, bytes_sent, total_sent, bytes_read);
                    }
                }

                if (total_sent != bytes_read) {  // If not all data was sent
                    printf("ERROR: Not all data was sent for file: %s \n", filepath);
                    break;
                }
            }
                                
                 // Wait for "DONE" message
                        char response[5]; // "DONE" + '\0'
                        if (recv(*socketfd, response, sizeof(response), 0) < 0) {
                            perror("Failed to receive DONE message");
                          
                        }
                        if (strcmp(response, "DONE") != 0) {
                            printf("Unexpected message: %s\n", response);
                         
                        }
                            
                        else {
                          printf("RECIVED DONE !!!!\n");
                        }

                        // If the file was sent successfully, delete it
                        // if (offset == stat_buf.st_size) {
                            if(remove(filepath) == 0){
                                printf("Deleted successfully, file: %s\n", filepath);
                            }
                            else {
                                printf("Unable to delete the file: %s\n", filepath);
                            }
                        }
                    //}

        printf("PERFORMING CLEANUP ... \n");

        // // /// DELETE OPERATION
        //  rewinddir(directory);  // Reset directory stream to beginning
        // while ((file = readdir(directory)) != NULL) {
        //     if (file->d_type == DT_REG) { // Check if it's a regular file
        //         char filepath[1024];
        //         snprintf(filepath, sizeof(filepath), "%s/%s", dir, file->d_name);
        //         if(remove(filepath) == 0){
        //             printf("Deleted successfully, file: %s\n", file->d_name);
        //         }
        //         else{
        //             printf("Unable to delete the file: %s\n", file->d_name);
        //         }
        //     }
        // }
                    
                   
    
return 0;
    }
