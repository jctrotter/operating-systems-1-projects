#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


void error(const char *msg) {
	perror(msg);
//	printf("%s\n", msg);
	exit(0);
}

void checkCharsRead(int charsRead){												// slapping these bad boys into a function
	if (charsRead < 0){
		error("CLIENT: Error reading from socket");
	}
}

void checkCharsWritten(int charsWritten, int bufferLen){						// checkin charsWritten in this handy function to save space
	if (charsWritten < 0) {
			error("CLIENT: Error writing to socket");
		}
	if (charsWritten < bufferLen){
		printf("CLIENT: WARNING: Not all data written to socket!");
	}	
}


int main(int argc, char *argv[]){
	int socketFD, portNumber, charsWritten, charsRead, c;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	FILE * fp;
	char key_dict[27] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',' '};	//valid characters
	char buffer[256];
	char *localhost = "localhost";
	char fchar;
	if (argc < 3){
		fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
		exit(0);
	}
	//set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); 			// clearout the serverAddress struct
	portNumber = atoi(argv[3]); 											// get port number, convert to int from string
	serverAddress.sin_family = AF_INET; 									// create a network socket
	serverAddress.sin_port = htons(portNumber); 							// store port number
	serverHostInfo = gethostbyname(localhost);								// gethostbyname(argv[1]);
	if (serverHostInfo == NULL){
		fprintf(stderr, "CLIENT: Error, no such host\n");
		exit(0);
	}
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); //copy address
	
	//set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); 							// create and assign the socket
	if (socketFD < 0){
		error("CLIENT: Error opening socket");
	}
	
	//connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) { // connect the socket to the address
		error("CLIENT: Error connecting");
	}
	strcpy(buffer, "otp_dec\0");
																			// send message to server
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); 				// write to server
	if (charsWritten < 0) {
		error("CLIENT: Error writing to socket");
	}
	if (charsWritten < strlen(buffer)){
		printf("CLIENT: WARNING: Not all data written to socket!");
	}
																			// get return from server
	memset(buffer, '\0', sizeof(buffer)); 									// clear again
	charsRead = recv(socketFD, buffer, sizeof(buffer) -1, 0); 				// read data from socket, leaving null terminator
	
	if (charsRead < 0){
		error("CLIENT: Error reading from socket");
	}

	//----------------------------------------actions----------
	memset(buffer, '\0', sizeof(buffer));									// iterate through our bad boys to see if we have invalid characters
	fp = fopen(argv[1], "r");
	fchar = fgetc(fp);
	int x, y;
	int charIsValid = 0;
	while(fchar != '\n'){													// while we are not at the end
		charIsValid = 0;
		for(x = 0; x < sizeof(key_dict)-1; x++){							// check if the working character x is in the dict of valid chars
			if (fchar == key_dict[x]){
				charIsValid = 1;											// don't check the rest if it is
				break;
			}
		}
		if(charIsValid == 1){												// get out if it is valid
			break;															
		}
		fchar = fgetc(fp);													// continue otherwise
	}

	fclose(fp);
	int cipherlen;
	if(charIsValid == 1){ 													// send the filename and key
		strcpy(buffer, argv[1]);
		charsWritten = send(socketFD, buffer, strlen(buffer), 0); 			// send the filename
		checkCharsWritten(charsWritten, strlen(buffer));

		memset(buffer, '\0', sizeof(buffer)); 								// clear again
		charsRead = recv(socketFD, buffer, sizeof(buffer) -1, 0); 			// read data from socket, leaving null terminator
		checkCharsRead(charsRead);

		memset(buffer, '\0', sizeof(buffer));
		strcpy(buffer, argv[2]);
		charsWritten = send(socketFD, buffer, strlen(buffer), 0); 			// send the key
		checkCharsWritten(charsWritten, strlen(buffer));
		
		memset(buffer, '\0', sizeof(buffer));								// gettinf confirmation that key was sent to server
		charsRead = recv(socketFD, buffer, sizeof(buffer) -1, 0);
		checkCharsRead(charsRead);

		memset(buffer, '\0', sizeof(buffer));								// getting size of cipher to recieve
		charsRead = recv(socketFD, buffer, sizeof(buffer)-1, 0);
		checkCharsRead(charsRead);
		cipherlen = atoi(buffer);

		char ciphertext[cipherlen+1];
		

		memset(buffer, '\0', sizeof(buffer));								// sending confirmation of cipherlen receipt
		strcpy(buffer, "I got your cipherlen");
		charsWritten = send(socketFD, buffer, strlen(buffer), 0);
		checkCharsWritten(charsWritten, strlen(buffer));

		memset(ciphertext, '\0', sizeof(ciphertext));
		charsRead = recv(socketFD, ciphertext, sizeof(ciphertext)-1, 0);
		checkCharsRead(charsRead);
		if(ciphertext[0] == '@'){											// don't print if something went wrong in the server
			printf("\n");
		}
		else{
			printf("%s\n", ciphertext);
		}
	}
	else{ 																	// tell server it aint happening, plaintext invalid
		strcpy(buffer, "Invalid plaintext");
		charsWritten = send(socketFD, buffer, strlen(buffer), 0); 			// write to server
		checkCharsWritten(charsWritten, strlen(buffer));
		
		memset(buffer, '\0', sizeof(buffer)); //clear again
		charsRead = recv(socketFD, buffer, sizeof(buffer) -1, 0); 			// read data from socket, leaving null terminator
		checkCharsRead(charsRead);

	}
	close(socketFD); // close the socket
	return 0;
}