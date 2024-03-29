
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
#include <sys/socket.h>

#include <netinet/in.h>

#include <netdb.h>


#define CMD_SIZE 100
#define MAXFILE 100
#define FILENAME 100
#define MAXHOSTNAME 256



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
    char NumeHostServer[MAXHOSTNAME];
    memset(NumeHostServer, 0, sizeof(NumeHostServer));
    gethostname(NumeHostServer, MAXHOSTNAME);
    printf("\n----TCPServer startat pe hostul: %s\n", NumeHostServer);

    struct hostent *he = gethostbyname(NumeHostServer);
    if (he == NULL)
    {
        perror("Failed to get hostname");
        return 1;
    }
    printf("\t(TCPServer INET ADDRESS (IP) este: %s)\n", inet_ntoa(*(struct in_addr *)he->h_addr));

    if (listen(socket_desc, 5) < 0)
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
		pthread_create(&sniffer_thread, NULL, ConnectionHandler, (void*) new_sock);
		pthread_join(sniffer_thread, NULL);
	}
	 
	if (socket_client<0)
	{
		perror("Accept failed");
		return 1;
	}

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
		return 6;
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
	else if(!strcmp(cmd, "EXIT"))
		return 6;
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
		
		//Send File
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

void performPUT(char *file_name, int socket)
{
	int c,r;
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

	// Getting File 
	
	int file_size;
	char *data;
	// Recieving file size and allocating memory
	recv(socket, &file_size, sizeof(int), 0);
	data = malloc(file_size+1);

	// Creating a new file, receiving and storing data in the file.
	FILE *fp = fopen(file_name, "w");
	r = recv(socket, data, file_size, 0);
	data[r] = '\0';
	printf("Size of file recieved is %d\n",r);
	r = fputs(data, fp);
	fclose(fp);
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

void performMPUT(int server_socket) {
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
      //    clock_t start_time = clock();
          
        while ((dir = readdir(d)) != NULL) {
            strcpy(full_name, dir->d_name);

            p1 = strtok(dir->d_name, ".");
            p2 = strtok(NULL, ".");
            if (p2 != NULL && strcmp(p2, ext) == 0)
                performPUT(full_name, server_socket);
        }
        closedir(d);
    }
    printf("MPUT Complete\n");
}

// Callback when a new connection is set up
void *ConnectionHandler(void *socket_desc)
{
	int	choice, file_desc, file_size;
	int socket = *(int*)socket_desc;

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
				performPUT(file_name, socket);
				break;
			case 3:
				strcpy(file_ext, GetArgumentFromRequest(client_request));
				performMGET(socket,file_ext);
				break;
			case 4:
            	strcpy(file_ext, GetArgumentFromRequest(client_request));
                performMPUT(socket);
			
				break;
			case 5:
				// showFile(socket);
				break;
			case 6:
				free(socket_desc);   
				return 0;
		}
	}
	free(socket_desc);   
	return 0;
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

	// Open file
	file_desc = open(file_name, O_RDONLY);
	// Send file size
	file_size = obj.st_size;
	write(socket_desc, &file_size, sizeof(int));
	// Send File
	sendfile(socket_desc, file_desc, NULL, file_size);

	printf("File %s sent\n",file_name);
	return true;
    
}
