#include "chunking.h"
#include <stdio.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define MAXFILE 100
#define FILENAME 100


#include "chunkread.h"

#include "chunkread.c"

#include "SockOp.h"

#include "SockOp.c"

#include "socket_utils.c"

#include "socket_utils.h"



void performGET(char *file_name,int socket_desc);
void performPUT(char *file_name,int socket_desc);
int SendFileOverSocket(int socket_desc, char* file_name);
void performMGET(int server_socket);
void performMPUT(int server_socket);



int main(int argc , char **argv)
{


	if(argc!=3){
		printf("Invalid arguments\n");
		return 0;
	}
	int socket_desc;
	struct sockaddr_in server;
	char request_msg[BUFSIZ], reply_msg[BUFSIZ], file_name[BUFSIZ];
	
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		perror("Could not create socket");
		return 1;
	}

	// ////////// FOARTE IMPORTANT ZICE SA LINGERUIE DACA NU SE TRANSMITE TOT !!!!!!!!!!!!!!
	// ///
	// //


	// const struct linger linger_val = { 1, 600 };
    // setsockopt(socket_desc, SOL_SOCKET, SO_LINGER, &linger_val, sizeof(linger_val));

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100;

		if (setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
			perror("setsockopt");
		
		}

				
		int flags = fcntl(socket_desc, F_GETFL, 0);
		if (flags == -1) {
			perror("fcntl F_GETFL");

		}

		// Clear the non-blocking flag
		flags &= ~O_NONBLOCK;
		if (fcntl(socket_desc, F_SETFL, flags) == -1) {
			perror("fcntl F_SETFL");
		
		}


	char SERVER_IP[BUFSIZ];
	int SERVER_PORT;
	strcpy(SERVER_IP,argv[1]);
	SERVER_PORT=atoi(argv[2]);
	server.sin_addr.s_addr = inet_addr(SERVER_IP);
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);

	// Connect to server
	if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Connection failed");
		return 1;
	}

	int choice = 0;
	while(1)
	{
		printf("Enter a choice:\n1- GET\n2- PUT\n3- MGET\n4- MPUT\n5- EXIT\n");
		scanf("%d", &choice);
		switch(choice)
		{
			case 1:
				printf("Enter file_name to get: ");
				scanf("%s", file_name);
				performGET(file_name,socket_desc);
				break;
			case 2:
				printf("Enter file_name to put: ");
				scanf("%s", file_name);
				performPUT(file_name,socket_desc);
				break;
			case 3:
				performMGET(socket_desc);	
				break;
			case 4:
				performMPUT(socket_desc);
				break;
			case 5:
				strcpy(request_msg,"EXIT");
				write(socket_desc, request_msg, strlen(request_msg));	
				return 0;
			default: 
				printf("Incorrect command\n");
		}
	}
	return 0;
}

ssize_t readall(int fd, char *buf, size_t bytes)
 {
     ssize_t bytes_read = 0;
     ssize_t n=0;

     do {
         if ((n = read(fd,
                       &buf[bytes_read],
                       bytes - bytes_read)) == -1)
         {
             if (errno == EINTR)  // resume on INTR
                 continue;
             else
                 return -1;
         }
         if (n == 0){
             printf("\n!!!!!!!!!!!!! n = 0 !!!!!!!!!!!!!!\n");
			 return bytes_read;
		 }
		 
         bytes_read += strlen(buf);
		 printf("\n!!!!!!!!!!!!! BUFF , %ld !!!!!!!!!!!!!!\n", bytes_read);
     } while (bytes_read < bytes);
     return bytes_read;
 }

int SendFileOverSocket(int socket_desc, char* file_name)
{
	int file_desc, file_size;

	printf("Sending File...\n");
	// stat(file_name, &obj);



	// sendfile(socket_desc, file_desc, NULL, file_size);
	//	call_readthread(file_name, &socket_desc);
	// send_imageSERVER(socket_desc, file_name);
	// send_file(file_name, socket_desc);


	printf("File %s sent\n",file_name);

	return 1;
}

 char* get_extension(const char* filename) {
     char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return ""; // Return empty string if no dot found or dot is the first character
    return dot + 1;
}

void performGET(char *file_name,int socket_desc){
	char request_msg[BUFSIZ], reply_msg[BUFSIZ] , filename_received[BUFSIZ];
	int file_size;
	char *data;
	int ignore = 0;
	int t;
	int mode = 1;

	
	if( access( file_name, F_OK ) != -1 )
	{
		int abortflag = 0;
		printf("File found on server. Press 1 to GET. Press any other key to abort.\n");
		scanf("%d", &abortflag);
		if(abortflag!=1)
			return;
	}
	// Get a file from server
	strcpy(request_msg, "GET ");
	strcat(request_msg, file_name);
	write(socket_desc, request_msg, strlen(request_msg));
	recv(socket_desc, reply_msg, 2, 0);
	reply_msg[2] = '\0';
	printf("%s\n", reply_msg);
	if (strcmp(reply_msg, "OK") == 0)
	{	

	if (mode == 0){
				//EXTRACT EXTENSION

			char * extension = get_extension(file_name);

						// Extract filename from path
			char *shortfile = strrchr(file_name, '/');
			if (shortfile) {
				shortfile++;  // Skip the '/'
			} else {
				shortfile = file_name;
			}

			// Copy filename to another string for modification
			char modifiedFilename[128];
			strncpy(modifiedFilename, file_name, sizeof(modifiedFilename) - 1);
			modifiedFilename[sizeof(modifiedFilename) - 1] = '\0';  // Null-terminate the string

			// Replace '.' with 'D'
			for (int i = 0; modifiedFilename[i]; i++) {
				if (modifiedFilename[i] == '.') {
				modifiedFilename[i] = '@';
				}
			}



		printf("GOT REPLY MESSAGE\n");
		char dirName[256];
        snprintf(dirName, sizeof(dirName), "ChunksTEMP_%s", file_name);
        mkdir(dirName, 0777); // Create directory with read/write/search permissions for owner and group, and with read/search permissions for others.

		printf("WENT PAST MKDIR\n");

		  // Receive the size of the file
        char file_size_str[BUFSIZ];
        recv(socket_desc, file_size_str, BUFSIZ, 0);
        file_size = atoi(file_size_str);  // Convert the received string to int
		
		int count = 0;
		printf("GOT FILESIZE =%d",file_size);

		int num_chunks = file_size / 8000 + 1; // Calculate the number of chunks that will be received

		printf("NUM CHUNKS = %d",num_chunks);
		int *received_chunks = calloc(num_chunks, sizeof(int)); // Allocate an array to keep track of received chunks

		while(1) {

			 printf("INSIDE COUNT WHILE\n");


			printf("GOT HERE...\n");
			// Receive data
			int receivedBytes = 0;
			// while(receivedBytes < 8000) {
				

					printf("IN RECV. WHILE\n");

					// Prepare to receive chunk index
					int index_received = -1;
					char index_buffer[BUFSIZ];


					

					// Receive the index from the server
					recv(socket_desc, index_buffer, BUFSIZ, 0);
					index_buffer[BUFSIZ - 1] = '\0';  // Null-terminate the received string to make sure it's a valid C-string

					// Convert received string to integer
					index_received = atoi(index_buffer);

					printf("INDEX RECV : %d",index_received);

					 if (received_chunks[index_received] == 1) {
						// This chunk was already received, skip it
						continue;
					}

			char outputFilename[128];
			snprintf(outputFilename, sizeof(outputFilename), "%s/%s_%d.%s", dirName, modifiedFilename ,index_received, extension);

			
			FILE *chunkFile = fopen(outputFilename, "w");
			if(chunkFile == NULL) {
				perror("Failed to open chunk file");
			}


				


				printf("RECV. DATA \n");
				char * buffer = malloc(BUFSIZ * sizeof(char));
			char * buffer_ptr = buffer;
			int bytes_left = 8000; // expected size of data to receive
			int bytes_received = 0; // number of bytes received so far
			fd_set read_fds; // fd_set for select call
			struct timeval timeout; // timeout for select call

			while (bytes_left > 0) {
				FD_ZERO(&read_fds); // clear the set
				FD_SET(socket_desc, &read_fds); // add our socket to the set

				// Set the timeout as desired
				timeout.tv_sec = 5;  // 5 seconds timeout
				timeout.tv_usec = 0;

				int select_status = select(socket_desc + 1, &read_fds, NULL, NULL, &timeout);

				if (select_status < 0) {
					perror("select");
					break;
				} 
				else if (select_status == 0) {
					printf("recv timeout\n");
					break;
				} 
				else {
					// select_status > 0 means data is available to read
					int bytes = recv(socket_desc, buffer_ptr, bytes_left, 0);
					
					if (bytes < 0) {
						perror("Failed to receive data");
						return;
					}
					else if (bytes == 0) {
						printf("Connection closed\n");
						break;
					}
					else {
						printf("RECV. b = %d \n",bytes);
					}

					bytes_received += bytes;
					bytes_left -= bytes;
					buffer_ptr += bytes;
				}
			}


		
			// The entire file has been received, send "DONE" message
			char *msg = "DONE";
			if (send(socket_desc, msg, strlen(msg), 0) < 0) {
				perror("Failed to send DONE message");
			}
			else {
				printf("SENT DONE MESSAGE. \n");
			}


				

			// IGNORE ERROR HERE

			// ACK RESPONSE FOR TCP IGNORE
			// if ( receivedBytes >= 10 && ignore == 0){
			printf("FILE WITH INDEX %d GOT THIS : %s\n\n" ,index_received , buffer);

			writeChunkToFile(outputFilename, buffer, bytes_received);

				
				//}
			fclose(chunkFile);	
			receivedBytes += bytes_received;
			
			// }

				// Mark this chunk as received
			received_chunks[index_received] = 1;

			// Check if all chunks have been received
			int all_received = 1;
			for (int i = 0; i < num_chunks; i++) {
				if (received_chunks[i] == 0) {
					// This chunk has not been received yet
					all_received = 0;
					break;
				}
			}


			ignore = 0;
			count++;
		}

		// runReconstruct("DONE.jpg", dirName, count, 8000);
		printf("COUNT IS = %d\n", count);
		reconstructFile("DONE2.jpg", dirName, count, 8000);



		

		free(data);
		printf("Done receiving data\n");


	
	// else
	// {
	// 	printf("File doesn't exist at server.ABORTING.\n");
	// }
	}
	else {
		

		receive_image(socket_desc,file_name);
		//receive_file(socket_desc,file_name);



	}
	}

}

void performPUT(char *file_name,int socket_desc)
{
	int	file_size, file_desc,c,t;
	char *data;
	char request_msg[BUFSIZ], reply_msg[BUFSIZ],client_response[2];
	// Get a file from server
	strcpy(request_msg, "PUT ");
	strcat(request_msg, file_name);
	printf("Trying to PUT %s to server. \n",file_name );
	if (access(file_name, F_OK) != -1)
	{
		// Sending PUT request to server.
		write(socket_desc, request_msg, strlen(request_msg));
		t = recv(socket_desc, reply_msg, BUFSIZ, 0);
		reply_msg[t]='\0';

            struct stat	obj;
	        stat(file_name, &obj);
            file_size = obj.st_size;
		if (strcmp(reply_msg, "OK") == 0)
		{
			// Everything is fine and send file
        
	
            printf("[OK] Sending File... of size = %d \n" ,file_size);
			SendFileOverSocket(socket_desc, file_name);
		}
		else if(strcmp(reply_msg, "FP") == 0)
		{
			// File present at server.
			printf("File exists in server. Do you want to overwrite? 1/0\n");
			scanf("%d", &c);
			if(c)
			{
				// User says yes to overwrite. Send Y and then data
				printf("Overwriting %s\n",file_name );
				strcpy(client_response, "Y");

				write(socket_desc, client_response, strlen(client_response));
                
				printf("Sending File... of size = %d \n" ,file_size);
				SendFileOverSocket(socket_desc, file_name);
			}
			else
			{
				printf("Not sending %s file to server\n",file_name);
				// User says No to overwrite. Send N and exit
				strcpy(client_response, "N");
				write(socket_desc, client_response, strlen(client_response));
				return;
			}
		}
		else{
			// Server can't create file.
			printf("Server can't create file...\n");
		}
	}
	else
	{
		// File not found locally hence abort.
		printf("File not found locally...\n");
		return;
	}
}
void performMGET(int socket_desc){
	printf("Performing MGET\n");

	char ext[BUFSIZ],request_msg[BUFSIZ],file_name[BUFSIZ];
	char *data;
	int file_size;
	char reply[BUFSIZ];
	int r;
	printf("Type Extension :\n");
	fflush(stdout);
	scanf("%s",ext);

	//Send Server Command to get all files with given extension 
	strcpy(request_msg,"MGET ");
	strcat(request_msg,ext);
	int l = write(socket_desc, request_msg, strlen(request_msg));
	int file_name_size=0;
	while(1){
		int t = recv(socket_desc,file_name,BUFSIZ,0);
		file_name[t]='\0';
		if(strcmp(file_name,"END")==0)
			break;
		printf("Recieving %s\n",file_name);

		// If file present at server ask user for overwrite or not.
		if( access( file_name, F_OK ) != -1 )
		{
			int abortflag = 0;
			printf("File already exists. Press 1 to overwrite. Press any other key to abort.\n");
			scanf("%d", &abortflag);
			if(abortflag!=1){
				// Send "NO" if user doesn't want to overwrite
				strcpy(reply,"NO");
				write(socket_desc,reply,2);
				printf("Not Overwriting %s \n",file_name);
				continue;
			}
			printf("Overwriting %s\n",file_name );
		}
		// Send "OK" if user wants to overwrite
		strcpy(reply,"OK");
		write(socket_desc,reply,2);
		
		// Checking if file present at server.This would be "OK" only
		recv(socket_desc, reply, 2, 0);
		if (strcmp(reply, "OK") == 0)
		{
		
			// Recieving file size, creating file, getting data and writing in file.
			recv(socket_desc, &file_size, sizeof(int), 0);
			data = malloc(file_size+1);
			FILE *fp = fopen(file_name, "w"); 
			r = recv(socket_desc, data, file_size, 0);
			data[r] = '\0';
			fputs(data, fp);
			fclose(fp);
			printf("File %s received with size %d\n", file_name,r);
		}   //////////// HERE
		else if (strcmp(reply, "OK") == 0 ){
			printf("File is being overwritten by another client .\n" );
        }
        else 
            {
                    printf("It seems some error occured .\n" );

            }
	}
	printf("MGET Complete\n");

}

void performMPUT(int server_socket){
	printf("Performing MPUT\n");
	
	char ext[BUFSIZ],request_msg[BUFSIZ];
	printf("Type Extension\n"); 
	scanf("%s",ext);
	

	DIR *d;
   	char *p1,*p2;
    int ret;
    struct dirent *dir;
    d = opendir(".");
    char full_name[100];
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
        	strcpy(full_name,dir->d_name);

            p1=strtok(dir->d_name,".");
            p2=strtok(NULL,".");
            if(p2!=NULL && strcmp(p2,ext)==0)
           		performPUT(full_name,server_socket);
      }
        closedir(d);
	}
    printf("MPUT Complete\n");

}