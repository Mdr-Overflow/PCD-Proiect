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

#include <netinet/tcp.h>  


#define MAXFILE 100
#define FILENAME 100
#define MAXLINE 1024

#define LOGINMESSAGE "[0] Login Required !"
#define REQUIREMESAGGE "[0] To sign in write 1 , to create a new account write 0 !" 
#define USERNAMEMESSAGE "[0] Username: "
#define PASSMESSAGE "[0] Password: "
#define unameCreateMESSAGE "[0] Type in a desired Username: "
#define passCreateMESSAGE "[0] Type in a desired Password: "
#define UnameWRONGMessage "[0] Username is wrong or illegal !  Try again : "
#define PassWRONGMessage "[0] Password is wrong or illegal ! Try again : "
#define ILLEGALMESSAGE "ILLEGAL"


#include "chunkread.h"

#include "chunkread.c"

#include "SockOp.h"

#include "SockOp.c"

#include "socket_utils.c"

#include "socket_utils.h"

#define MAX_BUFFER_SIZE 10240



void performGET(char *file_name,int socket_desc);
void performPUT(char *file_name,int socket_desc);
int SendFileOverSocket(int socket_desc, char* file_name);
void performMGET(int server_socket);
void performMPUT(int server_socket);


void performSHOW(int socket) {
    printf("Performing SHOW request\n");
    char buffer[BUFSIZ];
    int bytes_received;

   	int count = 0;
    while (1) {

		printf("INSIDE SHOW WHILE ...\n");

        memset(buffer, 0, sizeof(buffer)); 
        bytes_received = recv(socket, buffer, BUFSIZ-1, 0); 

        if(bytes_received <= 0){
            // Handle error
            perror("Failed to receive data from server");
           	continue;
        }


        if(strcmp(buffer, "END") == 0)
            break;

        printf("[%d]: %s\n", count,buffer);
		count++;
    }

    printf("End of SHOW\n");
}
void performSELECT(int socket) {
    printf("Requesting file selection\n");

    performSHOW(socket); // Call performSHOW first to get the file list

    char file_name[BUFSIZ];

    printf("Enter the name of the file you want to select:\n");
    scanf("%s", file_name);

    // Send file name to the server
    write(socket, file_name, strlen(file_name) + 1);

	// DO THE OPERATIONS MAGIC
	

}



int receive_image(int client_socket, const char *file_name) {

	printf("RECV. IMAGE \n");
 FILE *image_file = fopen(file_name, "wb");
    if (image_file == NULL) {
        perror("Failed to open image file");
        return -1;
    }
	printf("OPENED IMAGE \n");
    char *buffer = (char *)malloc(MAX_BUFFER_SIZE);
    if (buffer == NULL) {
        perror("Failed to allocate buffer");
        fclose(image_file);
        return -1;
    }
	printf("ALLOCATED BUFFER \n");
	
    int bytes_received;
    int total_received = 0;
	

	  struct timeval timeout = {2, 0};
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
        perror("Error on setting the socket options");
        exit(EXIT_FAILURE);
    }

	printf("RECV. IMAGE AND WRT. TO FILE");
    // Receive image data from server and write to file
    while ((bytes_received = recv(client_socket, buffer, MAX_BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes_received, image_file);
        total_received += bytes_received;
    }

	  if (bytes_received < 0) {
        // Timeout occurred
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // printf("recv() timed out.\n");
        }
        else {
            // perror("recv() failed");
            // exit(EXIT_FAILURE);
        }
    }

    printf("Total received: %d bytes\n", total_received);

    free(buffer);
    fclose(image_file);
    return 0;


}

int send_image(int client_socket, const char *file_name) {
    FILE *image_file = fopen(file_name, "rb");
    if (image_file == NULL) {
        perror("Failed to open image file");
        return -1;
    }

    char *buffer = (char *)malloc(MAX_BUFFER_SIZE);
    if (buffer == NULL) {
        perror("Failed to allocate buffer");
        fclose(image_file);
        return -1;
    }

    size_t bytes_read;

    // Read image data from file and send to server
    while ((bytes_read = fread(buffer, 1, MAX_BUFFER_SIZE, image_file)) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) < 0) {
            perror("Send failed");
            free(buffer);
            fclose(image_file);
            return -1;
        }
    }

    free(buffer);
    fclose(image_file);
    return 0;
}




int performAUTH(int socket_desc) {
    char server_reply[MAXLINE];
    char client_reply[MAXLINE];
    char input[MAXLINE];
    int authstate = 0;
    char Uname[MAXLINE];
    char password[MAXLINE];
      int sockfd, n;
  char recvline[MAXLINE];
  struct sockaddr_in serv_addr;
  struct hostent * he;


   
    while(1) {

            printf("Conectat la server de authentificare. Introduceti mesaje (\"exit\" pentru a inchide):\n");
    
         printf("\n>>> ");
    fflush(stdout);

    char sendline[MAXLINE + 1];
    fgets(sendline, MAXLINE, stdin);

    // Trimiterea mesajului catre server
    if (write(socket_desc, sendline, strlen(sendline)) < 0) {
      perror("EROARE client: nu pot sa trimit mesajul la server");
      close(socket_desc);
      exit(EXIT_FAILURE);
    }

    // Inchiderea conexiunii la server daca s-a introdus "quit"
    if (strncmp(sendline, "exit", 4) == 0) {
      printf("Clientul s-a deconectat de la server.\n");
      close(socket_desc);
      exit(0);
    }

    // Citirea raspunsului de la server
    // if ((n = read(socket_desc, recvline, MAXLINE)) < 0) {
    //   perror("EROARE client: nu pot sa citesc raspunsul de la server");
    //   close(socket_desc);
    //   exit(1);
    // }

// Citirea raspunsului de la server ( WAIT FOR 5 SECONDS)
   struct timeval timeout;
fd_set read_fds;
int fdmax = sockfd; // Assuming sockfd is your socket file descriptor

    while(1) {
    // Set up the file descriptor set.
    FD_ZERO(&read_fds);
    FD_SET(sockfd, &read_fds);

    // Set up the timeout. 5 seconds, 0 microseconds.
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    // Now call select. sockfd+1 is used because select checks up to fdmax-1.
    int retval = select(fdmax + 1, &read_fds, NULL, NULL, &timeout);
    
    if(retval == -1) {
        perror("select");
        // return;
    } else if(retval == 0) {
        // Timeout, do something, like resend the data or handle lost connection
        printf("Timeout reached, no data received.\n");
        continue; // If you want to retry
        // Or handle as per your application logic
    }

    // If we get here, there's data ready to be read on at least one descriptor
    if(FD_ISSET(sockfd, &read_fds)) {
    // ready to read
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));  // clear the buffer

    int bytes_received = recv(sockfd, buffer, sizeof(buffer)-1, 0);
    if(bytes_received < 0) {
        // handle error
        perror("Error reading from socket");
        continue;
    } else if(bytes_received == 0) {
        // handle disconnect
        printf("Server closed connection\n");
        break;
    } else {
        // data received successfully
        printf("Received: %s\n", buffer);
    }
}
}
    // else {
    //     printf("RET IS = %d",retval);
        
       
    // }
   

    printf("RECIVED :: %s",recvline);

    recvline[n] = '\0'; // adaugam terminatorul de sir

    printf("::: %s :::", recvline);
     // clear recvline buffer
    memset(recvline, 0, sizeof(recvline));

    // !!!!!!!!!! DACA serverul se inchide neasteptat va trimite "exit" catre client
    // clientul receptioneaza exit si se va inchide si el
    if(strncmp(recvline,"exit",4) == 0){
      close(socket_desc);
      fprintf(stderr,"Server closed unexpectedly");
      exit(1);
    }

  }
    // !!!!!!!!!! tratam inchiderea brusca a clientului prin semnal
   
    fprintf(stderr,"Manually closed with SIGINT or SIGSTPT");
    write(sockfd, "exit", strlen("exit"));

  // Inchiderea conexiunii TCP
  close(sockfd);




    }



int main(int argc , char **argv)
{

    // CHANGE STUFF HERE 


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

		// struct timeval timeout;
		// timeout.tv_sec = 0;
		// timeout.tv_usec = 100;

		// int flag = 1;
        // int result = setsockopt(socket_desc, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));

        // if (result < 0) {
        //     // Handle error here
        //     perror("Setting TCP_NODELAY error");
        //     // return -1;
        // }
				
		// int flags = fcntl(socket_desc, F_GETFL, 0);
		// if (flags == -1) {
		// 	perror("fcntl F_GETFL");

		// }

		// // Clear the non-blocking flag
		// flags &= ~O_NONBLOCK;
		// if (fcntl(socket_desc, F_SETFL, flags) == -1) {
		// 	perror("fcntl F_SETFL");
		
		// }


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

    // AFTER CONNECTING -> PERFORM AUTH FROM CLIENT SIDE 
    //
    //
    //     if(!performAUTH(socket_desc)) {
    //     printf("Auth failed\n");
    //     return 1;
    // }
														// /////

    ///
    ///

    ///
    //

	  int choice = 0;
    while(1)
    {
        printf("Enter a choice:\n1- GET\n2- PUT\n3- MGET\n4- MPUT\n5- SHOW\n6- SELECT\n7- EXIT\n");
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
                performSHOW(socket_desc);
                break;
            case 6:
                performSELECT(socket_desc);
                break;
            case 7:
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
	 send_image(socket_desc, file_name);
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
	else
	{
		printf("File doesn't exist at server.ABORTING.\n");
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
	
		receive_image(socket_desc,file_name);
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
			// recv(socket_desc, &file_size, sizeof(int), 0);
			// data = malloc(file_size+1);
			// FILE *fp = fopen(file_name, "w"); 
			// r = recv(socket_desc, data, file_size, 0);
			// data[r] = '\0';
			// fputs(data, fp);
			// fclose(fp);
			receive_image(socket_desc, file_name);

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
