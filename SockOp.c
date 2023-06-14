#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

#include "stdlib.h"

#include<errno.h>


int receive_imageCLIENT(int socket, char * file_name)
{ // Start function

int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;
char request_msg[BUFSIZ], reply_msg[BUFSIZ] , filename_received[BUFSIZ];
	int file_size;

char imagearray[10241],verify = '1';
FILE *image;

if( access( file_name, F_OK ) != -1 )
	{
		int abortflag = 0;
		printf("File found on server. Press 1 to GET. Press any other key to abort.\n");
		scanf("%d", &abortflag);
		if(abortflag!=1)
			return -1 ;
	}
	// Get a file from server
	strcpy(request_msg, "GET ");
	strcat(request_msg, file_name);
	write(socket, request_msg, strlen(request_msg));
	recv(socket, reply_msg, 2, 0);
	reply_msg[2] = '\0';
	printf("%s\n", reply_msg);
    if (strcmp(reply_msg, "OK") == 0)
    {	

    char buffer[] = "Got it";

    //Send our verification signal

      do{
    stat = write(socket, &buffer, sizeof(int));
    }while(stat<0);



    // RECV FILE SIZE 
 // Receive the size of the file
        char file_size_str[BUFSIZ];
        recv(socket, file_size_str, BUFSIZ, 0);
        file_size = atoi(file_size_str);  // Convert the received string to int
		
      size = file_size;
    // do{
    // stat = write(socket, &buffer, sizeof(int));
    // }while(stat<0);

    printf("Reply sent\n");
    printf(" \n");

    image = fopen(file_name, "w");

    if( image == NULL) {
    printf("Error has occurred. Image file could not be opened\n");
    return -1; }

    //Loop while we have not received the entire file yet


    int need_exit = 0;
    struct timeval timeout = {10,0};

    fd_set fds;
    int buffer_fd, buffer_out;

    while(recv_size < size) {
    //while(packet_index < 2){

        FD_ZERO(&fds);
        FD_SET(socket,&fds);

        buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);

        if (buffer_fd < 0)
          printf("error: bad file descriptor set.\n");

        if (buffer_fd == 0)
          printf("error: buffer read timeout expired.\n");

        if (buffer_fd > 0)
        {
            do{
                  read_size = read(socket,imagearray, 10241);
                }while(read_size <0);

                /*printf("Packet number received: %i\n",packet_index);
                printf("Packet size: %i\n",read_size);*/


            //Write the currently read data into our image file
            write_size = fwrite(imagearray,1,read_size, image);
            //printf("Written image size: %i\n",write_size);

                if(read_size !=write_size) {
                    printf("error in read write\n");    }


                //Increment the total number of bytes read
                recv_size += read_size;
                packet_index++;
                /*printf("Total received image size: %i\n",recv_size);
                printf(" \n");
                printf(" \n");*/
        }

    }


      fclose(image);
      printf("Image successfully Received!\n");
      return 1;
      }
    else {

      
		  printf("File doesn't exist at server.ABORTING.\n");
	


      }
      return 0;
}


int receive_imageSERVER(int socket, char * file_name)
{ // Start function

int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;
char request_msg[BUFSIZ], reply_msg[BUFSIZ] , filename_received[BUFSIZ];
	int file_size;

char imagearray[10241],verify = '1';
FILE *image;

if( access( file_name, F_OK ) != -1 )
	{
		int abortflag = 0;
		printf("File found on server. Press 1 to GET. Press any other key to abort.\n");
		scanf("%d", &abortflag);
		if(abortflag!=1)
			return -1 ;
	}
	// Get a file from server
	strcpy(request_msg, "GET ");
	strcat(request_msg, file_name);
	write(socket, request_msg, strlen(request_msg));
	recv(socket, reply_msg, 2, 0);
	reply_msg[2] = '\0';
	printf("%s\n", reply_msg);
    if (strcmp(reply_msg, "OK") == 0)
    {	

    char buffer[] = "Got it";

    //Send our verification signal

      do{
    stat = write(socket, &buffer, sizeof(int));
    }while(stat<0);



    // RECV FILE SIZE 
 // Receive the size of the file
        char file_size_str[BUFSIZ];
        recv(socket, file_size_str, BUFSIZ, 0);
        file_size = atoi(file_size_str);  // Convert the received string to int
		
      size = file_size;
    // do{
    // stat = write(socket, &buffer, sizeof(int));
    // }while(stat<0);

    printf("Reply sent\n");
    printf(" \n");

    image = fopen(file_name, "w");

    if( image == NULL) {
    printf("Error has occurred. Image file could not be opened\n");
    return -1; }

    //Loop while we have not received the entire file yet


    int need_exit = 0;
    struct timeval timeout = {10,0};

    fd_set fds;
    int buffer_fd, buffer_out;

    while(recv_size < size) {
    //while(packet_index < 2){

        FD_ZERO(&fds);
        FD_SET(socket,&fds);

        buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);

        if (buffer_fd < 0)
          printf("error: bad file descriptor set.\n");

        if (buffer_fd == 0)
          printf("error: buffer read timeout expired.\n");

        if (buffer_fd > 0)
        {
            do{
                  read_size = read(socket,imagearray, 10241);
                }while(read_size <0);

                /*printf("Packet number received: %i\n",packet_index);
                printf("Packet size: %i\n",read_size);*/


            //Write the currently read data into our image file
            write_size = fwrite(imagearray,1,read_size, image);
            //printf("Written image size: %i\n",write_size);

                if(read_size !=write_size) {
                    printf("error in read write\n");    }


                //Increment the total number of bytes read
                recv_size += read_size;
                packet_index++;
                /*printf("Total received image size: %i\n",recv_size);
                printf(" \n");
                printf(" \n");*/
        }

    }


      fclose(image);
      printf("Image successfully Received!\n");
      return 1;
      }
    else {

      
		  printf("File doesn't exist at server.ABORTING.\n");
	


      }
      return 0;
}






//This function is to be used once we have confirmed that an image is to be sent
//It should read and output an image file


int send_imageCLIENT(int socket, char * file_name)
{
    FILE *picture;
    int size, read_size, stat, packet_index;
    char send_buffer[10240], read_buffer[256];
    packet_index = 1;

    picture = fopen(file_name, "r");
    printf("Getting Picture Size\n");

    if(picture == NULL) {
         printf("Error Opening Image File"); }

    fseek(picture, 0, SEEK_END);
    size = ftell(picture);
    fseek(picture, 0, SEEK_SET);
    printf("Total Picture size: %i\n",size);

    //Send Picture Size
    printf("Sending Picture Size\n");
    write(socket, (void *)&size, sizeof(int));

    //Send Picture as Byte Array
    printf("Sending Picture as Byte Array\n");

    do { //Read while we get errors that are due to signals.
       stat=read(socket, &read_buffer , 255);
       printf("Bytes read: %i\n",stat);
    } while (stat < 0);

    printf("Received data in socket\n");
    printf("Socket data: %s\n", read_buffer);

    while(!feof(picture)) {
    //while(packet_index = 1){
       //Read from the file into our send buffer
       read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, picture);

       //Send data through our socket
       do{
         stat = write(socket, send_buffer, read_size);
       }while (stat < 0);

       printf("Packet Number: %i\n",packet_index);
       printf("Packet Size Sent: %i\n",read_size);
       printf(" \n");
       printf(" \n");


       packet_index++;

       //Zero out our send buffer
       bzero(send_buffer, sizeof(send_buffer));
      }
      return 0;
}



int send_imageSERVER(int socket, char * file_name)
{
    FILE *picture;
    int size, read_size, stat, packet_index;
    char send_buffer[10240], read_buffer[256];
    packet_index = 1;

    picture = fopen(file_name, "r");
    printf("Getting Picture Size\n");

    if(picture == NULL) {
         printf("Error Opening Image File"); }

    fseek(picture, 0, SEEK_END);
    size = ftell(picture);
    fseek(picture, 0, SEEK_SET);
    printf("Total Picture size: %i\n",size);

    //Send Picture Size
    printf("Sending Picture Size\n");
    write(socket, (void *)&size, sizeof(int));

    //Send Picture as Byte Array
    printf("Sending Picture as Byte Array\n");

    do { //Read while we get errors that are due to signals.
       stat=read(socket, &read_buffer , 255);
       printf("Bytes read: %i\n",stat);
    } while (stat < 0);

    printf("Received data in socket\n");
    printf("Socket data: %s\n", read_buffer);

    while(!feof(picture)) {
    //while(packet_index = 1){
       //Read from the file into our send buffer
       read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, picture);

       //Send data through our socket
       do{
         stat = write(socket, send_buffer, read_size);
       }while (stat < 0);

       printf("Packet Number: %i\n",packet_index);
       printf("Packet Size Sent: %i\n",read_size);
       printf(" \n");
       printf(" \n");


       packet_index++;

       //Zero out our send buffer
       bzero(send_buffer, sizeof(send_buffer));
      }
      return 0;
}


