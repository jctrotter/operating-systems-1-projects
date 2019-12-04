#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
//	printf("%s\n", msg);
	perror(msg);
	exit(1);
} // Error function 

void printfunc(const char *msg){
	printf(msg);
}

int receiveSend(char *buffer, int establishedConnectionFD, int charsRead){ //returns 0 successful, 1 if read err, 2 if write err, -1 if received nothing
	if(strcmp(buffer, "")){
		if (charsRead < 0){
			error("Error reading from socket");
			return 1;
		}
		//send a success back to the client
		charsRead = send(establishedConnectionFD, "I am the server, I got your message", 39, 0); //send it
		if (charsRead < 0){
			error("Error writing to socket");
			return 2;
		}
		return 0;
	}
	else{
		return -1;
	}
}

int main(int argc, char *argv[]){
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, y;
	int isInvalid = 0;
	socklen_t sizeOfClientInfo;
	char buffer[256];
	char bigbuffer1[20000];
	char bigbuffer2[20000];
	char bigbuffer3[20000];
	char bigbuffer4[9334];
	char args[2][256];
	char alpha[27] =  {'A','B','C','D','E','F','G','H','I','J','K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' '}; // cipher text
	int numeric[27] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26}; // cipher numbers
	struct sockaddr_in serverAddress, clientAddress;
	FILE * fp;
	if (argc < 2){
		fprintf(stderr, "USAGE: %s port\n", argv[0]);
		exit(1);
	}
	
	//set up address struct for server
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); 		// clear out the struct
	portNumber = atoi(argv[1]); 										// get port number, convert to an int from a string
	serverAddress.sin_family = AF_INET; 								// create a network socket
	serverAddress.sin_port = htons(portNumber); 						// store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; 						// any address is allowed for connection to this process
	
	//set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); 					// create the socket
	if (listenSocketFD < 0){
		error("Error opening socket");
	}
	
	//enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){ // connect socket to port
		error("Error on binding");
	}
	listen(listenSocketFD, 5); 											// socket can now recieve up to 5 connections
	while(1){															// run the server until terminated by kill -TERM
		isInvalid = 0;													// reset our invalid flag
																		// accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); 						// get size of the address fo rthe client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); //Accept it
		if (establishedConnectionFD < 0){
			error("Error on accept");
		}
		memset(buffer, '\0', 256);
		charsRead = recv(establishedConnectionFD, buffer, 255, 0); 		// read the client's message from the socket
		if (!strcmp(buffer, "otp_dec\0")){
			perror("Error: Connect with otp_dec not allowed.");
			isInvalid = 1;
		}
		if (charsRead < 0){
			error("Error reading from socket");
		}
		charsRead = send(establishedConnectionFD, "enc", 3, 0); 		//send success back to socket
		if (charsRead < 0){
			error("Error writing to socket");
		}

		//---------encryption--------
		int rs = -5;
		int x;
		memset(args[0], '\0', 256);
		memset(args[1], '\0', 256);
		for(x = 0; x < 2; x++){ 										//get args from client. plaintext = 0, mykey = 1
			memset(buffer, '\0', 256);
			charsRead = recv(establishedConnectionFD, buffer, 255, 0); 	// read the client's message from the socket
			strcpy(args[x], buffer);
			rs = receiveSend(buffer, establishedConnectionFD, charsRead);
		}
		if(!strcmp(args[0], "Invalid plaintext")){
			perror("Error: plaintext invalid");
		}
		else{
			fp = fopen(args[1], "r"); 									//get the key from the file specified in the arg
			int keylen = 0;
			char fchar;
			while(fchar != '\n'){
				fchar = fgetc(fp);
				keylen++;												//get the key len
			}
			keylen--;													//above method adds an extra len, remove it
			rewind(fp);													//reset the file pointer
			char key[keylen];											//get the key content
			fgets(key, sizeof(key), fp);
			fclose(fp);

			int cipherlen = 0;
			fp = fopen(args[0], "r"); 									//open the plaintext
			fchar = fgetc(fp);											
			rewind(fp);
			while(fchar != '\n'){
				fchar = fgetc(fp);
				cipherlen++;											// get the plaintext len
			}
			cipherlen--;
			rewind(fp);

			char cipher[cipherlen];
			memset(cipher, '\0', cipherlen);
			char kchar;
			int fint, kint, cint, z; 									//file int, key int, cipher int
			if(cipherlen > keylen){
				perror("Error: Invalid key");
				isInvalid = 1;
			}
			for(x = 0; x < cipherlen; x++){								//for each character in the plaintext, 
				fchar = fgetc(fp);										// get our working file character
				kchar = key[x];											// get our working key character
				for(y = 0; y < sizeof(alpha); y++){
					if (fchar == alpha[y]){								// if our file character matches one of our set chars...
						fint = numeric[y]; 								// set file int to the corresponding numeric value
					}
					if (kchar == alpha[y]){ 							// if our key character matches a set char...
						kint = numeric[y];								// set key int to the corresponding numeric value
					}
				}
				cint = (fint+kint)%27;									// apply the key, store it to cipher int.
				for(y = 0; y < sizeof(alpha); y++){
					if (cint == numeric[y]){							// if our cipher int is in our list of numbers...
						cipher[x] = alpha[y];							// store the corresponding alphabet value in the working spot in the cipher string.
						break;
					}
				}
			}
			memset(buffer, '\0', 256);
			sprintf(buffer, "%d", cipherlen);							// convert our cipherlen int to a string to write it to the socket
			charsRead = send(establishedConnectionFD, buffer, strlen(buffer), 0);
			if (charsRead < 0){
				error("Error writing to socket");
			}
			memset(buffer, '\0', 256);
			charsRead = recv(establishedConnectionFD, buffer, 255, 0);	// read the confirmation from client that space has been allocated
			if (charsRead < 0){
				error("Error reading from socket");
			}
			if (isInvalid == 1){										// if something went wrong, set the first value to @. Client will read this and not print anything.
				cipher[0] = '@';
			}
			charsRead = send(establishedConnectionFD, cipher, strlen(cipher), 0); // send the ciphered text
			if (charsRead < 0){
				error("Error writing to socket");
			}
		}													

		close(establishedConnectionFD); //close the existing socket which is connected to the client. concurrently loop creating new sockets for each request
	}
	close(listenSocketFD); // close the listening socket
	return 0;
}
//otp_enc_d - server
