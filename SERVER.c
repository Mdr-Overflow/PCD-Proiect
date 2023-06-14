#include <limits.h>
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
#include <signal.h>


#include <sys/socket.h>

#include <netinet/in.h>

#include <netdb.h>

#include <stdbool.h>

#include <sqlite3.h> 


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

// RECV/SEND  PUT - SERVER , GET - CLIENT


typedef struct FTPthreadArgs{

int run_thread_FILE; //= 0;
pthread_mutex_t run_lock_FILE ;//= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t run_cond_FILE ;//= PTHREAD_COND_INITIALIZER;
char * filename;
int * socket_d;
char * Uname;

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


#define UNIXSOCKET "/tmp/unixds"
#define INETPORT   18081
#define SOAPPORT   18082

//// SERVER COMMONS 


#define h_addr h_addr_list[0]

 //#include <strings.h> /* e mai nou decat string.h, dar nu merge */
/* Definitii pentru programul server TCP */
/* 1. Porturtul TCP utilizat. */
#define SERV_TCP_PORT 5002
/* 2. Alte constante */
#define MAXLINE 512 /* nr. max. octeti de citit cu recv() */

#define MAXUNAME 30
#define LOGINMESSAGE "[0] Login Required !"
#define REQUIREMESAGGE "[0] To sign in write 1 , to create a new account write 0 !" 
#define USERNAMEMESSAGE "[0] Username: "
#define PASSMESSAGE "[0] Password: "
#define unameCreateMESSAGE "[0] Type in a desired Username: "
#define passCreateMESSAGE "[0] Type in a desired Password: "
#define UnameWRONGMessage "[0] Username is wrong or illegal !  Try again : "
#define PassWRONGMessage "[0] Password is wrong or illegal ! Try again : "
#define ILLEGALMESSAGE "ILLEGAL"
// Statements


char *create_schema = "CREATE TABLE IF NOT EXISTS UserTable(\
   UserName TEXT PRIMARY KEY,\
   Password TEXT\
);\
\
CREATE TABLE IF NOT EXISTS Permisiuni (\
   UserName TEXT,\
   Role TEXT,\
   PRIMARY KEY (UserName, Role),\
   FOREIGN KEY (UserName)\
   REFERENCES UserTable(UserName)\
   ON DELETE CASCADE\
   ON UPDATE CASCADE\
);\
\
CREATE TABLE IF NOT EXISTS File (\
   FileID INTEGER PRIMARY KEY AUTOINCREMENT,\
   FileName TEXT UNIQUE,\
   UserID TEXT,\
   FileDataID INTEGER,\
   FOREIGN KEY (UserID) REFERENCES UserTable(UserName)\
    ON DELETE CASCADE\
    ON UPDATE CASCADE,\
   FOREIGN KEY (FileDataID) REFERENCES FileData(FileDataID)\
    ON DELETE CASCADE\
    ON UPDATE CASCADE\
);\
\
CREATE TABLE IF NOT EXISTS FileData (\
   FileDataID INTEGER PRIMARY KEY AUTOINCREMENT,\
   FileBuffer BLOB\
);\
\
CREATE TABLE IF NOT EXISTS Logs (\
   LogID INTEGER PRIMARY KEY AUTOINCREMENT,\
   Protocol TEXT,\
   TimeStamp DATETIME,\
   RestMethods TEXT,\
   OperationType TEXT,\
   OperationDuration INTEGER,\
   UserName TEXT,\
   FileName TEXT,\
   FOREIGN KEY (UserName) REFERENCES UserTable(UserName) ON DELETE CASCADE ON UPDATE CASCADE\
   FOREIGN KEY (FileName) REFERENCES File(FileName) ON DELETE CASCADE ON UPDATE CASCADE\
);";


typedef struct client {
    char *username;
    int isAdmin;
    int socket_descriptor;
} client;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// monitorizam daca userul inchide brusc serverul folosind ctrl + c sau alte semnale 
// facem acest lucru pentru a nu lasa portul ocupat daca are loc asa ceva
// daca userul inchide serverul neasteptat , interceptam semnalul si trimitem 
// SIGQUIT catre 'main' process deoarece prin aceasta metoda socketul/rile nu raman ocupate in "SIG_WAIT"
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!


/// UTILS



void removeWhitespace(char *str) {
    char *src = str, *dst = str;
    while (*src != '\0') {
        if (*src == ' ' || *src == '\n') {
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}



//



void SetupDataBase(){

    
    sqlite3 *db;
    int rc = sqlite3_open("pcdProiect.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
   
    rc = sqlite3_exec(db, create_schema, NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error creating table: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
    
    char *sql = "INSERT INTO UserTable (UserName, Password) VALUES (?, ?)";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    
    rc = sqlite3_bind_text(stmt, 1, "Admin", -1, SQLITE_TRANSIENT);
    rc = sqlite3_bind_text(stmt, 2, "ROOT",-1,SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Error inserting data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        // exit(1);
    }
    sqlite3_finalize(stmt);

    sql = "INSERT INTO Permisiuni (UserName, Role) VALUES (?, ?)";
   
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
    rc = sqlite3_bind_text(stmt, 1, "Admin", -1, SQLITE_TRANSIENT);
    rc = sqlite3_bind_text(stmt, 2, "ADMIN",-1,SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Error inserting data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        // exit(1);
    }
    sqlite3_finalize(stmt);


    sqlite3_close(db);
    //free(sql);
                        }


char * CheckRole(char *Uname) {

    sqlite3 *db;
    int rc = sqlite3_open("pcdProiect.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    char *sql = "SELECT Role FROM Permisiuni WHERE UserName = ?";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    rc = sqlite3_bind_text(stmt, 1, Uname, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error binding value: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        char *role = strdup((char *)sqlite3_column_text(stmt, 0));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return role;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return NULL;
}




void getUserInfo(char * username, char * pass) {
    
  //  printf("PAAAAAARCHET \n");
    sqlite3 *db;
    int rc = sqlite3_open("pcdProiect.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
        }

     char *sql = "SELECT Password FROM UserTable WHERE UserName = ?";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    removeWhitespace(username);

    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    int step = sqlite3_step(stmt);

  
    //printf("%s <0000 \n",username); GETS HERE
    // if (step == SQLITE_ROW) { // WONT ENTER
        if ( step == SQLITE_ROW){
        strcpy(pass,sqlite3_column_text(stmt, 0));
      
        printf("%s\n ", username);
        printf("%s\n ", pass);
        }
    // } 
    // if nothing found
    else {
         // Problem WAS here
         strcpy(username,"NaN");
         strcpy(pass,"NaN");
        
     }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
   // printf("CCCCCCCCCC  \n");
    //free(Uname);
    //free(Pass);

}



int callback(void *NotUsed, int argc, char **argv, 
                    char **azColName) {
    
    NotUsed = 0;
    
    for (int i = 0; i < argc; i++) {

        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    
    printf("\n");
    
    return 0;
}


void encrypt ( char * pass) {


    // Encrypt pass then pass to 

}


bool isAuthUNAME(pid_t clientPID, char * ClientUserName ){

    printf("%s <--- \n",ClientUserName);
    // this shit down of here good shit
    char * pass = malloc(sizeof(char) * MAXUNAME);
    // pass = NULL; DONT DO THIS IF U WANT TO COPY ON TOP OF IT
    getUserInfo(ClientUserName,pass);
    printf("uname is = %s \n", ClientUserName);
    if ( strcmp(ClientUserName,"NaN") == 0 || strcmp(ClientUserName,ILLEGALMESSAGE) == 0 ){
        return false;
    }
    else true;
    
}

bool isAuthPASS(pid_t clientPID, char * ClientUserName , char * ClientPassword){

    char * pass = malloc(sizeof(char) * MAXUNAME);
    //pass = NULL;
    // Initial nu criptam ca sa fie mai usor !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    encrypt(ClientPassword);
    getUserInfo(ClientUserName,pass);
    printf("\nPASS IS = %s \n",pass);
    printf("\nTyped Pass IS = %s \n",ClientPassword);
    if ( strcmp(pass,ClientPassword) == 0){
        return true;
    }
    else false;

}



void PostUsername(char * Uname){

    sqlite3 *db;
    int rc = sqlite3_open("pcdProiect.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
      //  exit(1);
    }

    removeWhitespace(Uname);

    /*

    if(illegalCheck(pass)) strcpy(Uname , ILLEGALMESSAGE)

    */

    char *sql = "INSERT INTO UserTable(UserName) VALUES(?);" ;

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        //exit(1);
    }
    rc = sqlite3_bind_text(stmt, 1, Uname, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Error inserting data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        // exit(1);
    }
    sqlite3_finalize(stmt);


    sqlite3_close(db);

}


void PostPass(char * Pass, char * Uname){

    sqlite3 *db;
    int rc = sqlite3_open("pcdProiect.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
      //  exit(1);
    }

    removeWhitespace(Pass);

    char *sql = "UPDATE UserTable SET Password = ? WHERE UserName = ?;" ;

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        //exit(1);
    }
    rc = sqlite3_bind_text(stmt, 1, Pass, -1, SQLITE_TRANSIENT);
    rc = sqlite3_bind_text(stmt, 2, Uname, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Error inserting data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        // exit(1);
    }
    sqlite3_finalize(stmt);


    sqlite3_close(db);

}



bool createUNAME(pid_t uname , char * ClientUserName){
    
    PostUsername(ClientUserName);
    if ( strcmp(ClientUserName,ILLEGALMESSAGE) == 0  ||  ClientUserName == NULL){
        return false;
    }
    else true;
}



bool createPASS(pid_t uname , char * ClientUserName, char * Pass){
    
    PostPass(Pass, ClientUserName);
    // fails here
  
    if ( strcmp(Pass,ILLEGALMESSAGE) == 0  || Pass == NULL){
        return false;
    }
    else true;


}

//// SERVER COMMONS

// AUTH THREAD HANDLER 

// 0 - ADMIN , 1 - USER , 2 - API USER 

client * performAUTH(int sockfd, int clientType )
{



    
  signal(SIGINT, sigintHandler);
  signal(SIGTSTP, sigtstpHandler);

  int rc, clilen, newsockfd, childpid;
  char line[512];
  char echo_buffer[520] = "echo ";
  struct sockaddr_in cli_addr, serv_addr;
  struct hostent * he; // gethostbyname()
  char * NumeServer; // numele serverului luat din argv[0]
  char NumeHostServer[MAXHOSTNAME];

        memset(line, 0, sizeof(line));  
        /*** Preiau informatii despre Server, NumeHostServer si INET ADDRESS(IP) ***/
        /*** (este doar pentru a afla IP server, utilizat de catre clientul TCP la conectare) ***/


  


        int authstate = 0;
        char * Uname = malloc(sizeof(char) * MAXUNAME); 
        int stage = 0;

        // TO DO ON CLIENT PERFORM AUTH MSSG. SEND

       printf("Performing AUTH ");
       while ((rc = recv(sockfd, & line, MAXLINE, 0)) )  

        {
          line[strlen(line)+1] = '\0'; // set EOB - end buffer

            removeWhitespace(Uname);
            printf("line = %s\n",line );


                // We must require auth 

                // authstate is different for each client 
                // 0 - send mess
                // 1 - rec mess
                // 11 - send uname req
                // 111 - recv uname req , send pass req
                // 1111 - recv pass and correct -> 999

                // 10 send create uname req
                // 11 send create pass req

                // AICI UNDEVA

                if (authstate == 0){

                printf("::: Sending Login Request to client...  \n");
                send(newsockfd, LOGINMESSAGE, strlen(LOGINMESSAGE), 0);
                send(newsockfd, REQUIREMESAGGE, strlen(REQUIREMESAGGE), 0);                    
                authstate = 1;
                memset(line, 0, sizeof(line));  // resetez bufferul pt. linia receptionata
                continue; // receptionam urm. mesaj de la client
                }

        if (authstate == 1){    

                if(strstr(line, "1")){
                
                send(newsockfd, USERNAMEMESSAGE, strlen(USERNAMEMESSAGE), 0);
                printf("::: Sending UserName request to client...  \n");
              
                authstate = 11;
                memset(line, 0, sizeof(line));  // resetez bufferul pt. linia receptionata
                continue; // receptionam urm. mesaj de la client
                }
                                    
                                    
                else if (strstr(line, "0")){

                send(newsockfd, unameCreateMESSAGE, strlen(unameCreateMESSAGE), 0);
                printf("::: Sending UserNameCreate request to client...  \n");
                authstate = 10;
                memset(line, 0, sizeof(line));  // resetez bufferul pt. linia receptionata
                continue; // receptionam urm. mesaj de la client

            }
                                    
                  } // end if authstate


                if (authstate == 11){
                    
                    printf("%s",line); // it gets here
                    if ( isAuthUNAME(getpid(),line))
                    {

                        send(newsockfd, PASSMESSAGE, strlen(PASSMESSAGE), 0);
                        printf("::: Sending Pass request to client...  \n");
                        authstate = 111;
                        printf("%s",line);
                        strcpy(Uname,line);
                        memset(line, 0, sizeof(line));  // resetez bufferul pt. linia receptionata
                        continue; // receptionam urm. mesaj de la client
                    }
                    else {

                        send(newsockfd, UnameWRONGMessage, strlen(UnameWRONGMessage), 0);
                        printf("::: Sending uname not found request to client...  \n");
                        authstate = 11;
                        memset(line, 0, sizeof(line));  // resetez bufferul pt. linia receptionata
                        continue; // receptionam urm. mesaj de la client

                    }
                }

                if (authstate == 111){

                    if ( isAuthPASS(getpid(),Uname,line) == true)
                    {

                        send(newsockfd, "Login Successful", strlen("Login Successful"), 0);
                        printf("::: Sending Login Successful to client...  \n");
                        authstate = 999;
                        memset(line, 0, sizeof(line));  // resetez bufferul pt. linia receptionata
                        continue; // receptionam urm. mesaj de la client
                    }
                    else {

                        send(newsockfd, PassWRONGMessage, strlen(PassWRONGMessage), 0);
                        printf("::: Sending uname not found request to client...  \n");
                        authstate = 111;
                        memset(line, 0, sizeof(line));  // resetez bufferul pt. linia receptionata
                        continue; // receptionam urm. mesaj de la client

                    }

                }

                //////////////////////// CREATE 

                   if (authstate == 10){
                
                    if ( createUNAME(getpid(),line))
                    {

                        send(newsockfd, PASSMESSAGE, strlen(PASSMESSAGE), 0);
                        printf("::: Sending Pass request to client...  \n");
                        authstate = 101;
                        strcpy(Uname,line);
                        memset(line, 0, sizeof(line));  // resetez bufferul pt. linia receptionata
                        continue; // receptionam urm. mesaj de la client
                    }
                    else {

                        send(newsockfd, UnameWRONGMessage, strlen(UnameWRONGMessage), 0);
                        printf("::: Sending uname illegal request to client...  \n");
                        authstate = 10;
                        memset(line, 0, sizeof(line));  // resetez bufferul pt. linia receptionata
                        continue; // receptionam urm. mesaj de la client

                    }
                }

                if (authstate == 101){

                    if ( createPASS(getpid(),Uname,line) == true)
                    {

                        send(newsockfd, "Login Successful", strlen("Login Successful"), 0);
                        printf("::: Sending LogON Successful to client...  \n");
                        authstate = 999;
                        memset(line, 0, sizeof(line));  // resetez bufferul pt. linia receptionata
                        continue; // receptionam urm. mesaj de la client
                    }
                    else {

                        send(newsockfd, PassWRONGMessage, strlen(PassWRONGMessage), 0);
                        printf("::: Sending Password illegal to client...  \n");
                        authstate = 101;
                        memset(line, 0, sizeof(line));  // resetez bufferul pt. linia receptionata
                        continue; // receptionam urm. mesaj de la client

                    }

                }


                 
                 // DO AUTH IN CLIENT
                 
                    
        if (authstate == 999){
            
           char *role = CheckRole(Uname);
            if (role != NULL) {
        client *newClient = malloc(sizeof(client)); // Allocate memory for client
        newClient->username = strdup(Uname); // Copy username
        newClient->socket_descriptor = clientType; // Assign socket_descriptor
        if ( strcmp(role, "ADMIN") == 0 && clientType == 0)
        {
           newClient->isAdmin = 1; // Set isAdmin for ADMIN
           // pass it      
           return newClient;
        }
        else if ( strcmp(role, "USER") == 0 && clientType == 1 )
        {
            newClient->isAdmin = 0; // Set isAdmin for USER
            // pass it   
             return newClient;
        }
        else if ( strcmp(role, "APIUSER") == 0 && clientType == 2)
        {
            newClient->isAdmin = 0; // Set isAdmin for APIUSER
            // pass it   
             return newClient;
        }
        else 
        {
            free(newClient); // Deallocate memory
            // HANDLE ERROR
        }
        free(role); 
    } else {
        // HANDLE ERROR
    }        }



    free(Uname);

    return NULL;

    } // end isAuth()
      }





// INET AREA 


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
	



	// call_serverthread(file_name, socket, file_size);

	receive_imageSERVER(socket,file_name);


	printf("RECIEVED \n"); 

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



	// sendfile(socket_desc, file_desc, NULL, file_size);
	//	call_readthread(file_name, &socket_desc);
	 send_imageCLIENT(socket_desc, file_name);
	// send_file(file_name, socket_desc);


	printf("File %s sent\n",file_name);
	return true;
    
}


// /end INET AREA





void *inet_main (void *args) {
  

    signal(SIGINT, sigintHandler);
    signal(SIGTSTP, sigtstpHandler);


    // DO AUTH ()


	int socket_desc, socket_client, *new_sock, 
	c = sizeof(struct sockaddr_in);

	struct  sockaddr_in	server, client;

	// Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		perror("Could not create socket");
		// EXIT
	}
	int SERVER_PORT = INETPORT;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(SERVER_PORT);

	if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Bind failed");
		// EXIT
	}

	const struct linger linger_val = { 1, 600 };
    setsockopt(socket_desc, SOL_SOCKET, SO_LINGER, &linger_val, sizeof(linger_val));

    char NumeHostServer[MAXHOSTNAME];
    memset(NumeHostServer, 0, sizeof(NumeHostServer));
    gethostname(NumeHostServer, MAXHOSTNAME);
    printf("\n----TCPServer startat pe hostul: %s\n", NumeHostServer);
    int con = 0;

    ThreadARGS = (FTPthreadArgs_t **)malloc(MAXCON * sizeof(FTPthreadArgs_t *));
    for (int i = 0 ;i< MAXCON ; i++)
    {
      ThreadARGS[i] = (FTPthreadArgs_t*) malloc(sizeof(FTPthreadArgs_t));      
    }

    struct hostent *he = gethostbyname(NumeHostServer);
    if (he == NULL)
    {
        perror("Failed to get hostname");
        // return 1;
    }
    printf("\t(TCPServer INET ADDRESS (IP) este: %s)\n", inet_ntoa(*(struct in_addr *)he->h_addr));

    if (listen(socket_desc, MAXCON) < 0)
    {
        perror("Listen failed");
        // return 1;
    }

    printf("---TCPServer %d: ++++ astept conexiune clienti pe PORT: %d++++\n\n", INETPORT, SERVER_PORT);


    
    
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

         // PERFORM AUTH FOR "USER" : 1
         struct client* client = performAUTH(socket_client, 1);
          if ( client != NULL) {
        
                ARG->Uname = client->username;   
          }  
          else {
            continue;
            //AND MESSAGE TO CLIENT
          }

		pthread_create(&sniffer_thread, NULL, ConnectionHandler, (void*) ARG);
		
	}

	//pthread_join(sniffer_thread, NULL);
	 
	if (socket_client<0)
	{
		perror("Accept failed");
		// return 1;
	}

    
    free(ThreadARGS);

    for (int i = 0 ; i< MAXCON ; i++)
     free(ThreadARGS[i]);
    



  return NULL;
}


// Thread functions
void *unix_main (void *args) {
  return NULL;
}


void *soap_main (void *args) {
  // SOAP request handling code here
  return NULL;
}




int main () {
  int iport, sport;
  pthread_t unixthr, inetthr, soapthr;
  
  pthread_t auth_thread;
  
  
  
  
  unlink (UNIXSOCKET);

  //// SETUP 

    SetupDataBase();

//     // CHECK IF IT WHNT PAST AUTH
//     int authstate = 0;
//     char * Uname = malloc(sizeof(char) * MAXUNAME); 
//     int stage = 0;



//     client* newClient = malloc(sizeof(client));  // Allocate memory for the new client
//     newClient->username = malloc(20 * sizeof(char));  // Allocate memory for the username string
//     strcpy(newClient->username, "Alice");  // Copy the string into the username field
//     newClient->isAdmin = 1;  // Set the isAdmin field
//     newClient->socket_descriptor = sockfd;  // Assign the socket descriptor


//   // AUTH ZONE 
//     if(pthread_create(&auth_thread, NULL, auth_thread_handler, (void*) &newsockfd) < 0)
//     {
//         perror("Could not create thread");
//         // return 1;
//     }


  
  pthread_create (&unixthr, NULL, unix_main, (void*) UNIXSOCKET);
  iport = INETPORT;
  pthread_create (&inetthr, NULL, inet_main, (void*) &iport);
  sport = SOAPPORT;
  pthread_create (&soapthr, NULL, soap_main, (void*) &sport);

  pthread_join (unixthr, NULL);
  pthread_join (inetthr, NULL);
  pthread_join (soapthr, NULL);

  unlink (UNIXSOCKET);

  return 0;
}