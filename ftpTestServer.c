
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <netdb.h>

#include "socket_utils.h"
#include "socketqueue.h"
#include "socketqueue.c"
#include "chunkread.h"
#include "chunkread.c"

#include "SockOp.h"

#include "SockOp.c"

#include "socket_utils.c"

#include "socket_utils.h"



#define CMD_SIZE 100
#define MAXFILE 100
#define FILENAME 100
#define MAXHOSTNAME 256
#define MAXCON 20




typedef struct FTPthreadArgs{

int run_thread_FILE; //= 0;
pthread_mutex_t run_lock_FILE ;//= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t run_cond_FILE ;//= PTHREAD_COND_INITIALIZER;
char * filename;
int * socket_d;

}FTPthreadArgs_t;


FTPthreadArgs_t ** ThreadARGS;

void *ConnectionHandler(void *socket_desc);
char* GetArgumentFromRequest(char* request);
bool SendFileOverSocket(int socket_desc, char* file_name);

void sigintHandler(int sig)
{
    printf("Received SIGINT signal. Exiting.\n");
    exit(0);
}

void sigtstpHandler(int sig)
{
    printf("Received SIGTSTP signal. Exiting.\n");
    exit(0);
}

int main(int argc, char **argv)
{
    signal(SIGINT, sigintHandler);
    signal(SIGTSTP, sigtstpHandler);

	if(argc!=2){
		printf("Invalid arguments\n");
		return 0;
	}

	int socket_desc, socket_client, *new_sock, 
	c = sizeof(struct sockaddr_in);

	struct  sockaddr_in	server, client;

	// Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		perror("Could not create socket");
		return 1;
	}
	int SERVER_PORT = atoi(argv[1]);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(SERVER_PORT);

	if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Bind failed");
		return 1;
	}

	const struct linger linger_val = { 1, 600 };
    setsockopt(socket_desc, SOL_SOCKET, SO_LINGER, &linger_val, sizeof(linger_val));

	// int opt = 1;
    // if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    //     perror("setsockopt failed");
    //     exit(EXIT_FAILURE);}


    char NumeHostServer[MAXHOSTNAME];
    memset(NumeHostServer, 0, sizeof(NumeHostServer));
    gethostname(NumeHostServer, MAXHOSTNAME);
    printf("\n----TCPServer startat pe hostul: %s\n", NumeHostServer);
    // NUMBER OF CONN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    int con = 0;

        // LEAST ANNOYING C CODE 
        ThreadARGS = (FTPthreadArgs_t **)malloc(MAXCON * sizeof(FTPthreadArgs_t *));
    for (int i = 0 ;i< MAXCON ; i++)
        {
          ThreadARGS[i] = (FTPthreadArgs_t*) malloc(sizeof(FTPthreadArgs_t));      

        }

    struct hostent *he = gethostbyname(NumeHostServer);
    if (he == NULL)
    {
        perror("Failed to get hostname");
        return 1;
    }
    printf("\t(TCPServer INET ADDRESS (IP) este: %s)\n", inet_ntoa(*(struct in_addr *)he->h_addr));

    if (listen(socket_desc, MAXCON) < 0)
    {
        perror("Listen failed");
        return 1;
    }

    printf("---TCPServer %s: ++++ astept conexiune clienti pe PORT: %d++++\n\n", argv[0], SERVER_PORT);
    
	while (socket_client = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c))
	{
		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = socket_client;
		printf("socket is %d",socket_client);

        if (con < 10){
          con++;
        }
        else {
          con = 0;
        }

        int run_thread_FILE = 0 ;
        pthread_mutex_t run_lock_FILE = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t run_cond_FILE = PTHREAD_COND_INITIALIZER;
        char * filename = (char *)malloc(FILENAME);

        FTPthreadArgs_t  *ARG = (FTPthreadArgs_t *) malloc(sizeof(FTPthreadArgs_t));
        ARG->filename = filename;
        ARG->run_cond_FILE = run_cond_FILE;
        ARG->run_lock_FILE = run_lock_FILE;
        ARG->run_thread_FILE = 0;
        ARG->socket_d = new_sock;

        ThreadARGS[con] = ARG;

		pthread_create(&sniffer_thread, NULL, ConnectionHandler, (void*) ARG);
		
	}

	//pthread_join(sniffer_thread, NULL);
	 
	if (socket_client<0)
	{
		perror("Accept failed");
		return 1;
	}

    
    free(ThreadARGS);

    for (int i = 0 ; i< MAXCON ; i++)
     free(ThreadARGS[i]);

	return 0;
}

int GetCommandFromRequest(char* request)
{
	char cmd[CMD_SIZE];
	strcpy(cmd, request);
	int i = 0;
	while(request[i] != ' ' && request[i] != '\0')
		i++;
	if(request[i] == '\0')
		return 7;
	else
	{
		strncpy(cmd, request, i-1);
		cmd[i] = '\0';
	}
		
	if(!strcmp(cmd, "GET"))
		return 1;
	else if(!strcmp(cmd, "PUT"))
		return 2;
	else if(!strcmp(cmd, "MGET"))
		return 3;
	else if(!strcmp(cmd, "MPUT"))
		return 4;
	else if(!strcmp(cmd, "SHOW"))
		return 5;
	else if(!strcmp(cmd, "SELECT"))
		return 6;
	else if(!strcmp(cmd, "EXIT"))
		return 7;
	return 0;
}


void performGET(char *file_name, int socket)
{
	char server_response[BUFSIZ];
	printf("Performing GET request of client\n");

	// Check if file present
	if (access(file_name, F_OK) != -1)
	{
		//File is present on server
		//Send "OK" message
		strcpy(server_response, "OK");
		write(socket, server_response, strlen(server_response));
		

		//   // Get the file size
        // struct stat st;
        // stat(file_name, &st);
        // int file_size = st.st_size;

        // // Send the file size
        // sprintf(server_response, "%d", file_size);
        // write(socket, server_response, strlen(server_response));


		//Send File + SIZE
		SendFileOverSocket(socket, file_name);
	}
	else
	{

		printf("File not present at server.ABORTING.\n");

		// Requested file does not exist, notify the client
		strcpy(server_response, "NO");
		write(socket, server_response, strlen(server_response)); 
	}
}

void performPUT(char *file_name, int socket, 
 FTPthreadArgs_t *args )
{
	int c,r;

    // set up local copies of the mutex things
    // int * run_thread_FILE -> args.run_thread_FILE;
    // pthread_mutex_t * run_lock_FILE = args.run_lock_FILE;
    // pthread_cond_t * run_cond_FILE = args.run_cond_FILE;
    args->filename = file_name;


	printf("Performing PUT request of client\n");

	char server_response[BUFSIZ], client_response[BUFSIZ];
	if(access(file_name, F_OK) != -1)
	{
		// Notifing client that file is present at server
		strcpy(server_response, "FP");
		write(socket, server_response, strlen(server_response));
		
		// Getting the users choice to override or not 
		r = recv(socket, client_response, BUFSIZ, 0);
		client_response[r]='\0';

		if(!strcmp(client_response, "N")){
			printf("User says don't overwrite\n");
			return;
		}
		printf("User says to overwrite the file.\n");

       
	}
	else
	{
		// Send acknowledgement "OK"
		strcpy(server_response, "OK");
		write(socket, server_response, strlen(server_response));
	}

   // Lock the mutex before accessing the file
    // check if file is accessed by any of the possible threads
     // Send a message to the client until it's done
     // DURERE DE CAP MASIVA !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          pthread_mutex_lock(&(args->run_lock_FILE));
          printf("BIG PROBLEM");
          for (int i = 0 ; i< MAXCON ; i++) {
            while ((ThreadARGS[i]->run_thread_FILE) == 1 && strcmp(file_name , ThreadARGS[i]->filename) == 0){
                                pthread_cond_wait(&(args->run_cond_FILE), &(args->run_lock_FILE));
                                printf("Waiting on file %s , for thread %d" , file_name , i);
            }
                }

         args->run_thread_FILE = 1;
         


	// Getting File 
	

	int file_size;
	char *data;
	// Recieving file size and allocating memory
	recv(socket, &file_size, sizeof(int), 0);

    printf("Received file size = %d\n",file_size);
	
    // printf("AAAAAAAAAAA \n"); TRECE 

	// Creating a new file, receiving and storing data in the file.
	
	//FILE *fp = fopen(file_name, "w");


	//r = recv(socket, data, file_size, MSG_WAITALL); // MSG_WAITALL

	call_serverthread(file_name, socket, file_size);
	

	printf("RECIEVED \n"); 

	//data[r] = '\0';
	//printf("Size of file recieved is %d\n",r);

    // 
   // printf("size of data = %ld" , strlen(data));

	// r = fputs(data, fp);
	// fclose(fp);

        // UNLOCK MUTEX AND SHARE THAT BY SIGNAL
        args->run_thread_FILE = 0;
        pthread_mutex_unlock(&(args->run_lock_FILE));
        args->filename = "NaN";
           for (int i = 0 ; i< MAXCON ; i++)
                 pthread_cond_signal(&(ThreadARGS[i]->run_cond_FILE));
        
}
void performMGET(int socket,char* file_ext){

	printf("Performing MGET request of client\n");
	DIR *d;
  	char *p1,*p2;
    int ret;
    char server_response[BUFSIZ],reply[BUFSIZ];

    struct dirent *dir;
    d = opendir(".");
    char full_name[BUFSIZ];
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
        	strcpy(full_name,dir->d_name);
            p1=strtok(dir->d_name,".");
            p2=strtok(NULL,".");
            if(p2!=NULL && strcmp(p2,file_ext)==0)
            {

					write(socket, full_name, strlen(full_name));
     				int t = recv(socket, reply, 2, 0);
     				if(!strcmp(reply,"OK"))
						performGET(full_name,socket);
            }
        }
        closedir(d);

        // End MGET Request by sending "END"
        strcpy(server_response,"END");
        write(socket, server_response, strlen(server_response));
    }
}

void performMPUT(int server_socket, FTPthreadArgs_t * args) {
    printf("Performing MPUT\n");

    char ext[BUFSIZ], request_msg[BUFSIZ];
    printf("Type Extension:\n");
    scanf("%s", ext);

    DIR *d;
    char *p1, *p2;
    int ret;
    struct dirent *dir;
    d = opendir(".");
    char full_name[100];
    if (d) {
          clock_t start_time = clock();
                            
        while ((dir = readdir(d)) != NULL) {
            strcpy(full_name, dir->d_name);

            args->filename = full_name;



            p1 = strtok(dir->d_name, ".");
            p2 = strtok(NULL, ".");
            if (p2 != NULL && strcmp(p2, ext) == 0)
                performPUT(full_name, server_socket,args);
        }
        closedir(d);
    }
    printf("MPUT Complete\n");
} 

// Callback when a new connection is set up
void *ConnectionHandler(void * ARGS)
{

    FTPthreadArgs_t *args = (FTPthreadArgs_t *)ARGS;

    
    // socket
    int  socket_desc = *(args->socket_d);


    // mutex and cond
 

	int	choice, file_desc, file_size;
	int socket = socket_desc;

	char reply[BUFSIZ], file_ext[BUFSIZ],server_response[BUFSIZ], client_request[BUFSIZ], file_name[BUFSIZ];
	char *data;
	while(1)
	{	printf("\nWaiting for command\n");
		int l = recv(socket, client_request, BUFSIZ, 0);
		client_request[l]='\0';
		printf("Command Recieved %s\n",client_request );
		choice = GetCommandFromRequest(client_request);
		switch(choice)
		{
			case 1:
				strcpy(file_name, GetArgumentFromRequest(client_request));
				performGET(file_name, socket);
				break;
			case 2:
				strcpy(file_name, GetArgumentFromRequest(client_request));
				performPUT(file_name, socket, args);
				break;
			case 3:
				strcpy(file_ext, GetArgumentFromRequest(client_request));
				performMGET(socket,file_ext);
				break;
			case 4:
            	strcpy(file_ext, GetArgumentFromRequest(client_request));
                performMPUT(socket,args);
			case 5 : 
				// showFile(socket);
				break;
			case 6:
				// selectFile();
				break;
			case 7:
				//free(socket_desc);   
				pthread_exit(NULL);
		}
	}
// //	free(socket_desc);   
	pthread_exit(NULL);
}

char* GetArgumentFromRequest(char* request)
{
	char *arg = strchr(request, ' ');
	return arg + 1;
}

bool SendFileOverSocket(int socket_desc, char* file_name)
{
	struct stat	obj;
	int file_desc, file_size;

	printf("Sending File...\n");
	stat(file_name, &obj);

	
	// file_size = obj.st_size;
    // // Open file
	// file_desc = open(file_name, O_RDONLY);

    // Send file size

    // printf("file size is = %d\n", file_size);             ////////////////////// WTF ??????????

	
	// write(socket_desc, &file_size, sizeof(int));
	// Send File

	// sendfile(socket_desc, file_desc, NULL, file_size);
	//	call_readthread(file_name, &socket_desc);
	//send_image(socket_desc, file_name);
	send_file(file_name, socket_desc);


	printf("File %s sent\n",file_name);
	return true;
    
}