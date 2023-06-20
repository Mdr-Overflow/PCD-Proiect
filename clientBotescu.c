
/**
 * Madaras Andrei - Iulian
 * IA3 2023, subgrupa 3
 * Tema 5 (nr temei)
 * 
 * 
 * Acest program face asta (creaza un client pentru un server ce utilizeaza socketuri INET peste TCP) :
 * 
 *Specificatii aplicație client (1)

Clientul va folosi socket INET peste TCP si va comunica serverului un mesaj din cele de mai jos:

{time, user}, la fiecare pas.

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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#include <arpa/inet.h>

#include <resolv.h>



#define MAXLINE 512 // nr. max. octeti de citit sau trimis


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// monitorizam daca userul inchide brusc clientul folosind ctrl + c sau alte semnale 
// facem acest lucru pentru a nu lasa portul ocupat daca are loc asa ceva
static volatile sig_atomic_t keep_running = 1;

void sigintHandler(int sig_num)
{
    
    signal(SIGINT, sigintHandler);
    keep_running = 0;

}

void sigtstpHandler(int sig_num)
{
    
    signal(SIGTSTP, sigtstpHandler);
    keep_running = 0;

}

int main(int argc, char * argv[]) {

  signal(SIGINT, sigintHandler);
  signal(SIGTSTP, sigtstpHandler);


  int sockfd, n;
  char recvline[MAXLINE + 1];
  struct sockaddr_in serv_addr;
  struct hostent * he;

  if (argc != 3) {
    printf("Utilizare: %s <adresa_server> <port_server>\n", argv[0]);
    exit(1);
  }

  // Creare socket TCP
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("EROARE client: nu pot sa deschid stream socket");
    exit(1);
  }

  // Prelucrarea adresei IP a serverului
  he = gethostbyname(argv[1]);
  if (he == NULL) {
    perror("EROARE client: nu pot sa obtin adresa serverului");
    exit(1);
  }

  // Setarea informatiilor despre adresa serverului
  bzero((char * ) & serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy(he -> h_addr ,& serv_addr.sin_addr, he -> h_length);
  serv_addr.sin_port = htons(atoi(argv[2]));

  // Conectare la server
  if (connect(sockfd, (struct sockaddr * ) & serv_addr, sizeof(serv_addr)) < 0) {
    perror("EROARE client: nu pot sa ma conectez la server");
    close(sockfd);
    exit(1);
  }



  printf("Conectat la server. \n");
  printf("Sending test message ...  \n");
 
  //

  printf("Introduceti mesaje (\"exit\" pentru a inchide):\n");
  // TEST RECIEVE 
  char * testline = "test";
 
  if (write(sockfd, testline, strlen(testline)) < 0) {
      perror("EROARE client: nu pot sa trimit mesajul la server");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

  if ((n = read(sockfd, recvline, MAXLINE)) < 0) {
      perror("EROARE client: nu pot sa citesc raspunsul de la server");
      close(sockfd);
      exit(1);
    }

  recvline[n] = '\0'; // adaugam terminatorul de sir

  printf("::: %s :::", recvline);

  fflush(stdout);



  while (keep_running) {
    // Citirea mesajului de la tastatura

    printf("\n>>> ");
    fflush(stdout);

    char sendline[MAXLINE + 1];
    fgets(sendline, MAXLINE, stdin);

    // Trimiterea mesajului catre server
    if (write(sockfd, sendline, strlen(sendline)) < 0) {
      perror("EROARE client: nu pot sa trimit mesajul la server");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    // Inchiderea conexiunii la server daca s-a introdus "quit"
    if (strncmp(sendline, "exit", 4) == 0) {
      printf("Clientul s-a deconectat de la server.\n");
      close(sockfd);
      exit(0);
    }

    // Citirea raspunsului de la server
    if ((n = read(sockfd, recvline, MAXLINE)) < 0) {
      perror("EROARE client: nu pot sa citesc raspunsul de la server");
      close(sockfd);
      exit(1);
    }

    recvline[n] = '\0'; // adaugam terminatorul de sir

    printf("::: %s :::", recvline);

    // !!!!!!!!!! DACA serverul se inchide neasteptat va trimite "exit" catre client
    // clientul receptioneaza exit si se va inchide si el
    if(strncmp(recvline,"exit",4) == 0){
      close(sockfd);
      fprintf(stderr,"Server closed unexpectedly");
      exit(1);
    }

  }
    // !!!!!!!!!! tratam inchiderea brusca a clientului prin semnal
   
    fprintf(stderr,"Manually closed with SIGINT or SIGSTPT");
    write(sockfd, "exit", strlen("exit"));

  // Inchiderea conexiunii TCP
  close(sockfd);
  return 0;
}


/*
TEST 1:

        SERVER      
 @@@@@@@@@@@@@@@@@@@@@@@@@@                                                                                 
 
 ./serverBotescu         

----TCPServer startat pe hostul: vm-virtual-machine
        (TCPServer INET ADDRESS (IP) este: 127.0.1.1 )
---TCPServer ./serverBotescu: ++++ astept conexiune clienti pe PORT: 5002++++


---TCPServer./serverBotescu ___client PID= 152026 conectat
::: Sending some echo to client...
::: Sending some echo to client...
::: Sending some echo to client...
::: Sending some echo to client...
::: Sending current time to client.. 
::: Sending current user to client...  

---TCPServer ___client PID= 152026 disconnected
                           

        CLIENT
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

 ./clientBotescu 127.0.0.4 5002
Conectat la server. Introduceti mesaje ("exit" pentru a inchide):

>>> sasd
::: echo sasd :::
>>> as
::: echo as :::
>>> das
::: echo das :::
>>> dffdfd
::: echo dffdfd :::
>>> time
::: Sun 30 Apr 22:37:11 :::
>>> user
::: vm :::
>>> exit
Clientul s-a deconectat de la server.                             
                    

TEST 2:


        SERVER      
 @@@@@@@@@@@@@@@@@@@@@@@@@@  


 ---TCPServer./serverBotescu ___client PID= 152161 conectat
::: Sending some echo to client...
::: Sending some echo to client...
::: Sending some echo to client...

---TCPServer ___client PID= 152161 disconnected



            CLIENT
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

./clientBotescu 127.0.0.4 5002
Conectat la server. Introduceti mesaje ("exit" pentru a inchide):

>>> sd
::: echo sd :::
>>> ddsd
::: echo ddsd :::
>>> ^C
Manually closed with SIGINT or SIGSTPT


TEST 3:

        SERVER      
 @@@@@@@@@@@@@@@@@@@@@@@@@@  

---TCPServer./serverBotescu ___client PID= 152161 conectat
::: Sending some echo to client...
::: Sending some echo to client...
::: Sending some echo to client...

---TCPServer ___client PID= 152161 disconnected
^CQuit (core dumped)




*/