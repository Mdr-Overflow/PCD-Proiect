#ifndef SOCKET_UTILS_H_
#define SOCKET_UTILS_H_

#include <sys/types.h>


void read_bytes(int fd, void *buff, size_t size);

void write_bytes(int fd, void *buff, size_t size);

char *receive_file(int sfd, char *filename);

int send_file(char *filepath, int sfd);

char *get_filename_ext(const char *filename, char delimiter);

#endif // SOCKET_UTILS_H_