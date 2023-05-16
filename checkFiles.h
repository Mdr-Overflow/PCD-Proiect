#ifndef CHECKFILES_H
#define CHECKFILES_H

#include <dirent.h>

int check_send_and_delete_files(const char* dir, int num, int * socketfd);

#endif /* CHECKFILES_H */
