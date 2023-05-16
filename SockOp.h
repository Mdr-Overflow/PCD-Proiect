#ifndef SOCKOP_H
#define SOCKOP_H

#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>

/**
 * Receive an image over a socket connection and save it to disk.
 *
 * @param socket The file descriptor for the socket connection.
 * @return 1 if the image is successfully received and saved, -1 otherwise.
 */
int receive_image(int socket, char * filename);

/**
 * Read an image from disk and send it over a socket connection.
 *
 * @param socket The file descriptor for the socket connection.
 */
int send_image(int socket, char * filename);

#endif /* SOCKOP_H*/
