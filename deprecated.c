// 	if (mode == 0){
	// 		char * extension = get_extension(file_name);

	// 					// Extract filename from path
	// 		char *shortfile = strrchr(file_name, '/');
	// 		if (shortfile) {
	// 			shortfile++;  // Skip the '/'
	// 		} else {
	// 			shortfile = file_name;
	// 		}

	// 		// Copy filename to another string for modification
	// 		char modifiedFilename[128];
	// 		strncpy(modifiedFilename, file_name, sizeof(modifiedFilename) - 1);
	// 		modifiedFilename[sizeof(modifiedFilename) - 1] = '\0';  // Null-terminate the string

	// 		// Replace '.' with 'D'
	// 		for (int i = 0; modifiedFilename[i]; i++) {
	// 			if (modifiedFilename[i] == '.') {
	// 			modifiedFilename[i] = '@';
	// 			}
	// 		}



	// 	printf("GOT REPLY MESSAGE\n");
	// 	char dirName[256];
    //     snprintf(dirName, sizeof(dirName), "ChunksTEMP_%s", file_name);
    //     mkdir(dirName, 0777); // Create directory with read/write/search permissions for owner and group, and with read/search permissions for others.

	// 	printf("WENT PAST MKDIR\n");

	// 	  // Receive the size of the file
    //     char file_size_str[BUFSIZ];
    //     recv(socket_desc, file_size_str, BUFSIZ, 0);
    //     file_size = atoi(file_size_str);  // Convert the received string to int
		
	// 	int count = 0;
	// 	printf("GOT FILESIZE =%d",file_size);

	// 	int num_chunks = file_size / 8000 + 1; // Calculate the number of chunks that will be received

	// 	printf("NUM CHUNKS = %d",num_chunks);
	// 	int *received_chunks = calloc(num_chunks, sizeof(int)); // Allocate an array to keep track of received chunks

	// 	while(1) {

	// 		 printf("INSIDE COUNT WHILE\n");


	// 		printf("GOT HERE...\n");
	// 		// Receive data
	// 		int receivedBytes = 0;
	// 		// while(receivedBytes < 8000) {
				

	// 				printf("IN RECV. WHILE\n");

	// 				// Prepare to receive chunk index
	// 				int index_received = -1;
	// 				char index_buffer[BUFSIZ];

                    
					

	// 				// Receive the index from the server
	// 				recv(socket_desc, index_buffer, BUFSIZ, 0);
	// 				index_buffer[BUFSIZ - 1] = '\0';  // Null-terminate the received string to make sure it's a valid C-string

	// 				// Convert received string to integer
	// 				index_received = atoi(index_buffer);

	// 				printf("INDEX RECV : %d",index_received);

	// 				 if (received_chunks[index_received] == 1) {
	// 					// This chunk was already received, skip it
	// 					continue;
	// 				}

	// 		char outputFilename[128];
	// 		snprintf(outputFilename, sizeof(outputFilename), "%s/%s_%d.%s", dirName, modifiedFilename ,index_received, extension);

			
	// 		FILE *chunkFile = fopen(outputFilename, "w");
	// 		if(chunkFile == NULL) {
	// 			perror("Failed to open chunk file");
	// 		}


				


	// 			printf("RECV. DATA \n");
	// 			char * buffer = malloc(BUFSIZ * sizeof(char));
	// 		char * buffer_ptr = buffer;
	// 		int bytes_left = 8000; // expected size of data to receive
	// 		int bytes_received = 0; // number of bytes received so far
	// 		fd_set read_fds; // fd_set for select call
	// 		struct timeval timeout; // timeout for select call

	// 		while (bytes_left > 0) {
	// 			FD_ZERO(&read_fds); // clear the set
	// 			FD_SET(socket_desc, &read_fds); // add our socket to the set

		
	// 			timeout.tv_sec = 5;  // 5 seconds timeout
	// 			timeout.tv_usec = 0;

	// 			int select_status = select(socket_desc + 1, &read_fds, NULL, NULL, &timeout);

	// 			if (select_status < 0) {
	// 				perror("select");
	// 				break;
	// 			} 
	// 			else if (select_status == 0) {
	// 				printf("recv timeout\n");
	// 				break;
	// 			} 
	// 			else {
	// 				// select_status > 0 means data is available to read
	// 				int bytes = recv(socket_desc, buffer_ptr, bytes_left, 0);
					
	// 				if (bytes < 0) {
	// 					perror("Failed to receive data");
	// 					return;
	// 				}
	// 				else if (bytes == 0) {
	// 					printf("Connection closed\n");
	// 					break;
	// 				}
	// 				else {
	// 					printf("RECV. b = %d \n",bytes);
	// 				}

	// 				bytes_received += bytes;
	// 				bytes_left -= bytes;
	// 				buffer_ptr += bytes;
	// 			}
	// 		}


		
	// 		// The entire file has been received, send "DONE" message
	// 		char *msg = "DONE";
	// 		if (send(socket_desc, msg, strlen(msg), 0) < 0) {
	// 			perror("Failed to send DONE message");
	// 		}
	// 		else {
	// 			printf("SENT DONE MESSAGE. \n");
	// 		}


				

	// 		// IGNORE ERROR HERE

	// 		// ACK RESPONSE FOR TCP IGNORE
	// 		// if ( receivedBytes >= 10 && ignore == 0){
	// 		printf("FILE WITH INDEX %d GOT THIS : %s\n\n" ,index_received , buffer);

	// 		writeChunkToFile(outputFilename, buffer, bytes_received);

				
	// 			//}
	// 		fclose(chunkFile);	
	// 		receivedBytes += bytes_received;
			
	// 		// }

	// 			// Mark this chunk as received
	// 		received_chunks[index_received] = 1;

	// 		// Check if all chunks have been received
	// 		int all_received = 1;
	// 		for (int i = 0; i < num_chunks; i++) {
	// 			if (received_chunks[i] == 0) {
	// 				// This chunk has not been received yet
	// 				all_received = 0;
	// 				break;
	// 			}
	// 		}


	// 		ignore = 0;
	// 		count++;
	// 	}

	// 	// runReconstruct("DONE.jpg", dirName, count, 8000);
	// 	printf("COUNT IS = %d\n", count);
	// 	reconstructFile("DONE2.jpg", dirName, count, 8000);



		

	// 	free(data);
	// 	printf("Done receiving data\n");


	// }