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

	int opt = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);}


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
	struct stat	obj;
	int file_desc, file_size;

	stat(file_name, &obj);
	file_desc = open(file_name, O_RDONLY);
	 
	file_size = obj.st_size;
	int bytes_read = 0;

	char * BUFF = malloc(file_size + 1) ;
	

	//int ree = readall(file_desc, BUFF, file_size);
	
	//printf(" SIZE IS = %d\n",ree);

	printf(" BEFOR SEND SIZE OF BUFF IS = %lu\n",strlen(BUFF));

	send(socket_desc, &file_size, sizeof(int), 0);


	printf(" SIZE OF BUFF IS = %lu\n",strlen(BUFF));


	call_readthread(file_name, &socket_desc);


	printf("Sent FILE of size = %d\n",file_size);

	free(BUFF);
	return 1;
}

void performGET(char *file_name,int socket_desc){
	char request_msg[BUFSIZ], reply_msg[BUFSIZ];
	int file_size;
	char *data;
	int t;
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
	
		// File present at server.Start receiving the file and storing locally.
		printf("Recieving data\n");
	
		recv(socket_desc, &file_size, sizeof(int), 0);
		data = malloc(file_size+1);
		FILE *fp = fopen(file_name, "w"); 
		t = recv(socket_desc, data, file_size, 0);
		data[t] = '\0';
		fputs(data, fp);
		fclose(fp);
		printf("File %s recieved with size %d \n", file_name,t);
	}
	else
	{
		printf("File doesn't exist at server.ABORTING.\n");
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