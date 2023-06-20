
/**
 * Madaras Andrei - Iulian
 * IA3 2023, subgrupa 3
 * Tema 5 (nr temei)
 * 
 * Acest program face asta (creaza un server ce utilizeaza socketuri INET peste TCP) ::
 * 
 * Specificatii aplicatii server (1)

Server-ul va procesa, la fiecare pas, mesajul transmis, si va raspunde inapoi clientului astfel:
daca se primeste “time” => serverul va raspunde clientului cu timpul local de pe sistem
daca se primeste “user” => serverul va raspunde clientului cu numele utilizatorului curent
altfel, se primeste un mesaj “msg” => serverul raspunde “echo :: msg” (unde msg e un mesaj oarecare)

Conexiunea trebuie mentinuta, pana cand server-ul se opreste, sau pana cand clientul inchide conexiunea. 

 * 
 * 
 * 
 *
 * Am tratat urm. situatii limita care pot aparea in cazul programului:
    -Inchiderea brusca a serverului prin semnal de la user sau sistem
    -Inchiderea brusca a clientului prin semnal de la user sau sistem
    -Receprionarea eronata de caractere
    -Erori ce pot aparea in procesul preluarii celor 2 comenzi (time si user) de catre socketul serverului
 */




#include <stdio.h>

#include <stdlib.h> // pt. functiile exit()/ abort()

#include <unistd.h>

#include <signal.h>

#include <string.h>

#include <errno.h>

#include <sys/wait.h>

#include <sys/types.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <netdb.h>

#include <arpa/inet.h>

#include <resolv.h>

#include <time.h> /*for time struct and strftime*/

#include <stdbool.h>

#include <sqlite3.h> 


#define h_addr h_addr_list[0]

 //#include <strings.h> /* e mai nou decat string.h, dar nu merge */
/* Definitii pentru programul server TCP */
/* 1. Porturtul TCP utilizat. */
#define SERV_TCP_PORT 5002
/* 2. Alte constante */
#define MAXLINE 512 /* nr. max. octeti de citit cu recv() */
#define MAXHOSTNAME 100 /* nr. max. octeti nume host */
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


 void InsertIntoUserTable( void * User){







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



void sigintHandler(int sig_num)
{
    
    signal(SIGINT, sigintHandler);
    kill(getpid(),SIGQUIT);

}

void sigtstpHandler(int sig_num)
{
    
    signal(SIGTSTP, sigtstpHandler);
    kill(getpid(),SIGQUIT);
}


int main(int argc, char * argv[]) {

  signal(SIGINT, sigintHandler);
  signal(SIGTSTP, sigtstpHandler);

  int rc, sockfd, clilen, newsockfd, childpid;
  char line[512];
  char echo_buffer[520] = "echo ";
  struct sockaddr_in cli_addr, serv_addr;
  struct hostent * he; // gethostbyname()
  char * NumeServer; // numele serverului luat din argv[0]
  char NumeHostServer[MAXHOSTNAME];

  memset(line, 0, sizeof(line));  
  /*** Preiau informatii despre Server, NumeHostServer si INET ADDRESS(IP) ***/
  /*** (este doar pentru a afla IP server, utilizat de catre clientul TCP la conectare) ***/


  SetupDataBase();

  gethostname(NumeHostServer, MAXHOSTNAME); // aflam numele serverului
  printf("\n----TCPServer startat pe hostul: %s\n", NumeHostServer);
  he = gethostbyname(NumeHostServer); // aflam adresa de IP server/ probabil 127.0.0.1
  bcopy(he -> h_addr, & (serv_addr.sin_addr), he -> h_length);
  printf(" \t(TCPServer INET ADDRESS (IP) este: %s )\n",
    inet_ntoa(serv_addr.sin_addr)); // conversie adresa binarea in ASCII (ex. "127.0.0.1")
  /* numele procesului server luat de pe linia de comanda */
  NumeServer = argv[0];
  /*
  ** Open pentru un socket TCP
  (AF_INET: este din familia de protocoale Internet)
  (SOCK_STREAM: este de tipul stream socket)
  socket - crează un soclu fără nume cu caracteristicile cerute de tipul de comunicare dorit şi returnează
  un descripror de soclu
  */

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    perror("EROARE server: nu pot sa deschid stream socket");
  /*
   ** Asignarea unei adrese locale si port protocol spre care clientul poate trimite date.
   */

  
  bzero((char * ) & serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("127.0.1.1"); // INADDR_ANY binds the socket to all available interfaces.
  serv_addr.sin_port = htons(SERV_TCP_PORT); // SERV_TCP_PORT = 5001
  int option = 1;
  //setsockopt(sockfd, IPPROTO_TCP, SO_REUSEADDR, &option, sizeof(option));




  // UUUUUUUUUUUUUUUUUUUUUUUUU HERE ARE CLIENT PARAMS  


  int authstate = 0;
  char * Uname = malloc(sizeof(char) * MAXUNAME); 
  int stage = 0;

  /**
  bind - realizează legătura între un descriptor de soclu anterior creat şi o adresă locală finală de
  comunicare. Adresa locală finală este specificată atât prin adresa de IP cît şi prin numărul de port.
  **/
  if (bind(sockfd, (struct sockaddr * ) & serv_addr, sizeof(serv_addr)) < 0){
     perror("EROARE server: nu pot sa asignez un nume adresei locale");
     exit(EXIT_FAILURE);
  }
  printf("---TCPServer %s: ++++ astept conexiune clienti pe PORT: %d++++\n\n", NumeServer,
    ntohs(serv_addr.sin_port));
  /*
  ** Server gata de conexiune pentru orice client
  listen - plasează soclul serverului intr-un mod pasiv şi îl face pregătit pentru a accepta conexiunile care îi
  vin de la client
  */
  

  listen(sockfd, 5); // max. 5 conexiuni simultane
  for (;;) /* server cu conexiune – Server TCP*/ {
    /*
     * Bucla infinita.
     * Asteptarea conexiunea cu un client.
     */
    bzero((char * ) & cli_addr, sizeof(cli_addr));
    clilen = sizeof(cli_addr);
    /*
    accept - o conexiune actuală a procesului client este aşteptată
    */
    newsockfd = accept(sockfd, (struct sockaddr * ) & cli_addr, & clilen); // Asteptare conexiune din partea unui client

    if (newsockfd < 0) {
      perror("EROARE server: accept() esuat");
      exit(-1);
    }
    /* creare proces copil pentru coxiunea acceptata */
    if ((childpid = fork()) < 0) {
      perror("EROARE server: fork() esuat");

      exit(1);
    } else {
      if (childpid == 0) { /* proces copil */
        close(sockfd); /* close socket original */
        printf("\n---TCPServer%s ___client PID= %ld conectat\n", NumeServer, (long) getpid());   /* procesare cerere: 
                                                                                                    serverul citeste date de la client pe care afiseaza
                                                                                                    dupa care serverul trimite datele in ecou catre client */
        while (rc = recv(newsockfd, & line, MAXLINE, 0) )  

        {
          line[strlen(line)+1] = '\0'; // set EOB - end buffer


    /*

    if(illegalCheck(pass)){ Send("FUCK YOU"); Continue; }

    */

            removeWhitespace(Uname);
            printf("line = %s\n",line );

          //  printf("OOOOOOOOOOOOOOO\n");

        // We must require auth 

        // authstate is different for each client 
        // 0 - send mess
        // 1 - rec mess
        // 11 - send uname req
        // 111 - recv uname req , send pass req
        // 1111 - recv pass and correct -> 999

        // 10 send create uname req
        // 11 send create pass req

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


                 
                 
                 
                    
        if (authstate == 999){

    
                
                // trimite userul
                if (strstr(line, "user")) {
                char *username = getenv("USER");    // preiau utilizatorul serverului
                printf("::: Sending current user to client...  \n");
                send(newsockfd, username, strlen(username), 0);
                memset(line, 0, sizeof(line));  // resetez bufferul pt. linia receptionata
                continue; // receptionam urm. mesaj de la client
                }

                // trimite data si ora curenta
                if (strstr(line, "time")) {
                time_t timer;
                char buff[26];
                struct tm *tm_info;

                timer = time(NULL);
                tm_info = localtime(&timer);                          // preiau data de timp a serverului

                strftime(buff, 26, "%a %d %b %H:%M:%S", tm_info);     // salvez data de timp in formatul cerut
                printf("::: Sending current time to client.. \n");
                send(newsockfd, buff, strlen(buff), 0);
                memset(line, 0, sizeof(line));     // resetez bufferul pt. linia receptionata
                continue; // receptionam urm. mesaj de la client

                }

                    
                    if (strstr(line, "exit")) // exit daca este in linie
                    {  /* la introducere "exit" iesim din ciclu */
                    
                        printf("\n---TCPServer ___client PID= %ld disconnected\n", (long) getpid());
                        close(newsockfd);
                        memset(line, 0, sizeof(line));
                        exit(0); /* copilul ramane "defunct" si nu dispare daca parintele nu zice wait(NULL)*/
                    } 


                        // daca nu este una din comenzile predefinite serverul va trimite "echo" 
            

                        printf("::: Sending some echo to client...\n"); 
                        strncat(echo_buffer,line,strlen(line)-1);             // concateneaza "echo " la mesaj
                        send(newsockfd, echo_buffer, strlen(echo_buffer), 0); // trimit in ecou mesajul 
                        
                        memset(line, 0, sizeof(line));                        // resetez bufferul pt. linia receptionata
                        strcpy(echo_buffer,"echo ");                          // resetez bufferul pt. concatenare


        
        } // end isAuth()

            }; // end while- se reia ciclul infinit de recv
        
        
      
      
      
      } // end tratare copil
        

      {
        // tratare parinte
        wait(NULL);
        /* NU asteapta codul de retur al copilului
- “defunctii” nu dispar decat asa */
      } // end tratare parinte
    } // end tratare parinte/copil
  } // end for - se reia ciclul infinit
} // end main

