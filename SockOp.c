#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

#include "stdlib.h"

#include<errno.h>


#include <stdio.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

void send_image2(int socket, char* file_name) {
    int file_fd;
    off_t offset = 0;
    struct stat file_stat;

    // Open the file for reading
    file_fd = open(file_name, O_RDONLY);
    if (file_fd == -1) {
        perror("Failed to open file");
        return;
    }

    // Get file stats
    if (fstat(file_fd, &file_stat) < 0) {
        perror("Failed to get file stats");
        return;
    }

    // Send file size
    char file_size_str[BUFSIZ];
    sprintf(file_size_str, "%ld", file_stat.st_size);
    if (send(socket, file_size_str, sizeof(file_size_str), 0) < 0) {
        perror("Failed to send file size");
        return;
    }

    // Send file
    ssize_t bytes_sent;
    while (offset < file_stat.st_size) {
        bytes_sent = sendfile(socket, file_fd, &offset, file_stat.st_size - offset);
        if (bytes_sent <= 0) {
            perror("Failed to send file");
            return;
        }
        printf("Sent %ld bytes from file's data, offset is now : %ld and remaining data = %ld\n", bytes_sent, offset, file_stat.st_size - offset);
    }

    close(file_fd);
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void receive_image2(int socket, char* file_name) {
    int file_size;
    FILE* received_file;
    int remain_data = 0;
    ssize_t len;
    char buffer[BUFSIZ];

    // Receive and print file size
    recv(socket, buffer, BUFSIZ, 0);
    file_size = atoi(buffer);
    fprintf(stdout, "\nFile size : %d\n", file_size);

    received_file = fopen(file_name, "w");
    if (received_file == NULL) {
        fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    remain_data = file_size;

    while ((remain_data > 0) && ((len = recv(socket, buffer, BUFSIZ, 0)) > 0)) {
        fwrite(buffer, sizeof(char), len, received_file);
        remain_data -= len;
        fprintf(stdout, "Receive %d bytes and we hope :- %d bytes\n", len, remain_data);
    }
    fclose(received_file);
}