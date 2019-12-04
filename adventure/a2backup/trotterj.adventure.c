#include <stdio.h>
#include <pthread.h>
#include <dirent.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define NUM_THREADS 	2

typedef struct Room{ 
        char name[9];
        char type[13];
	char connections[6][9];
}Room;

typedef struct PathLink{
	Room * value;
	struct PathLink * Next;
}PathLink;

typedef struct PathList{ // linked list 1 for all rooms
	struct PathLink * Head;
	struct Room * current;
	int size;
}PathList;

typedef struct RoomJourney{ 
	char* room;
	struct RoomJourney * Next;
}RoomJourney;

typedef struct GameState{ // linked list 2 for steps taken and order rooms visited
	int stepsTaken;
	RoomJourney * Head;
	char filePath[256];
}GameState;
//
//
//
//
void addToPathList(struct PathList * rooms, struct Room * workingRoom){
	struct PathLink * temp;
	temp = rooms->Head;
	if(!rooms->Head->value){
		rooms->Head->value = workingRoom;
		rooms->Head->Next = NULL;
	}
	else
	{
		while(temp->Next){
			temp = temp->Next;
		}
		struct PathLink * temp2 =(struct PathLink *) malloc(sizeof (struct PathLink));
		temp2->value = workingRoom;
		temp2->Next = NULL;
		temp->Next = temp2;
	}
}

void addToJourney(struct GameState * game, char* roomToAdd){
	struct RoomJourney * temp;
	temp = game->Head;
	if(!game->Head->room){
		game->Head->room = roomToAdd;
		game->Head->Next = NULL;
	}
	else
	{
		while(temp->Next){
			temp = temp->Next;
		}
		struct RoomJourney * temp2 =(struct RoomJourney *) malloc(sizeof (struct RoomJourney));	
		temp2->room = roomToAdd;
		temp2->Next = NULL;
		temp->Next = temp2;
	}
}

void printJourney(struct GameState * game){
	struct RoomJourney * temp;
	temp = game->Head;
	while(temp->Next){
		printf("%s\n", temp->room);
		temp = temp->Next;
	}
}

void initRoom(struct PathList * rooms, struct Room * workingRoom, char* filePath){ // this is fucked??
	//setup 
	FILE* filePointer;
	filePointer = fopen(filePath, "r");

	//name 
	int size;
	fseek(filePointer, 0L, SEEK_END); // get the size of the file, reset to beginning
	size = ftell(filePointer);
	rewind(filePointer);
	char buffer[size]; // create a buffer string, read the first line of the file into the buffer
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, size-1, filePointer);
	int i = 0;
	char bufferComparison = buffer[i];
	while(bufferComparison != ':'){ // iterate through the line until we hit ":"
		i++;
		bufferComparison = buffer[i];
	}
	i=i+2; // i in buffer = starting character of the name
	char newName[9];
	memset(newName, '\0', sizeof(newName));
	int i2 = 0;
	bufferComparison = buffer[i];
	char newLine = '\n';
	while(buffer[i] && buffer[i] != newLine){ // copy the value of buffer post ": " into newName
		newName[i2] = buffer[i];
		bufferComparison = buffer[i];
		i2++;
		i++;
	}
	memset(workingRoom->name, '\0', sizeof(workingRoom->name)); // put newName into the struct
	strcpy(workingRoom->name, newName);
	//connections
	char newConnection[9];
	int c = 0;
	while(size != ftell(filePointer)){ // if file pointer is not at end, we are looking at connections.
		memset(newConnection, '\0', sizeof(newConnection));
		memset(buffer, '\0', sizeof(buffer));
		fgets(buffer, size-1, filePointer);
		if(size != ftell(filePointer)){  // if file pointer is not at end, then its a connection line
			i = 0;
			bufferComparison = buffer[i];
			while(bufferComparison != ':'){
				i++;	
				bufferComparison = buffer[i];
			}
			i=i+2;
			i2 = 0;	
			bufferComparison = buffer[i];
			while(buffer[i] && buffer[i] != newLine){
				newConnection[i2] = buffer[i];
				bufferComparison = buffer[i];
				i2++;
				i++;
			}
			strcpy(workingRoom->connections[c], newConnection);
			c++;	
		}
	}
	//type
	char newType[13];
	if(size == ftell(filePointer)){ // if filepointer is at end, its room status line
		memset(newType, '\0', sizeof(newConnection));
		i = 0;
		bufferComparison = buffer[i]; //put the value of the line into the struct, same as above.
		while(bufferComparison != ':'){
			i++;
			bufferComparison = buffer[i];
		}
		i=i+2;
		i2=0;
		bufferComparison = buffer[i];
		while(buffer[i] && buffer[i] != newLine){
			newType[i2] = buffer[i];
			bufferComparison = buffer[i];
			i2++;
			i++;
		}
		memset(workingRoom->type, '\0', sizeof(workingRoom->type));
		strcpy(workingRoom->type, newType);
	}
	fclose(filePointer);
}

void initRoomList(struct PathList * rooms){
	assert(rooms);
	rooms->Head = (struct PathLink *) malloc(sizeof (struct PathLink));
	rooms->size = 0;	
}

void initGameList(struct GameState * game){
	assert(game);
	game->Head = (struct RoomJourney *) malloc(sizeof (struct RoomJourney));
	game->stepsTaken = 0;
}

void populateList(struct PathList * rooms, char * dirName){
	int i = 0;
	struct Room * newRoom;
	char * roomNames[9]; // hardcoded file names
	roomNames[0] = "fire_room";
	roomNames[1] = "water_room";
	roomNames[2] = "magic_room";
	roomNames[3] = "sword_room";
	roomNames[4] = "dragon_room";
	roomNames[5] = "shield_room";
	roomNames[6] = "living_room";
	roomNames[7] = "electric_room";
	roomNames[8] = "coffee_room";
	roomNames[9] = "tear_room";
	char filePath[50];
	char backslash[50] = "/";
	int successes = 0;
	char tempName[50];
	int breaker = 0;
	while(successes < 7 && i < 10){	// while we are in the set of values and we haven't filled our criteria
		memset(tempName, '\0', sizeof(tempName)); //generates directory and room names
		strcat(tempName, dirName);
		strcat(tempName, backslash);
		strcat(tempName, roomNames[i]);
		memset(filePath, '\0', sizeof(filePath));
		strcpy(filePath, tempName);
		if(access(filePath, F_OK) != -1) { // if directory/filename exists, read the data and make rooms
			newRoom = (struct Room *) malloc(sizeof (struct Room));
			initRoom(rooms, newRoom, filePath);
			addToPathList(rooms, newRoom);
			successes++;
    		}
		i++;
	}
}

void initGame(struct PathList * rooms, struct GameState * game){ // finds room with type START_ROOM, sets rooms->current to START
	initGameList(game);
	struct PathLink * tmp;
	tmp = rooms->Head;
	char* start = "START_ROOM";
	int cmp = 0;
	while(tmp){
		if(tmp->value->type[0] != start[0])
		{
			tmp = tmp->Next;
		}
		else{
			rooms->current = tmp->value;
			tmp = tmp->Next;
		} 
	}
	free(tmp);
}
int setCurrent(struct PathList * rooms, char * roomToFind, struct GameState * game){
	struct PathLink * tmp;
	tmp = rooms->Head;
	int i;
	int cmp;
	while(tmp){
		cmp = 0;
		for(i = 0; i < 3; i++){  // compares the first three characters of the query and the destination.
               		 if(tmp->value->name[i] != roomToFind[i]){
                	         cmp = 1;
        	         }
	         }

		if(cmp == 0 && rooms->current != tmp->value){ // if characters match and current is not itself, update current
			rooms->current = tmp->value;
			game->stepsTaken++;
                        addToJourney(game, rooms->current->name);

			return 0;
		}
		else{
			tmp = tmp->Next;
		}
	}
	return 1;
	
}

void playGame(struct PathList * rooms, struct GameState * game){  
	char userInput[9];
	char* end_room = "END_ROOM";
	char* time = "time";
	int setCurrentStatus = 1;
	int steps = -1;
	char* tear = "tear";
	while(strcmp(rooms->current->type, end_room) != 0){  //while we are not at the end room, play the game
		if(game->stepsTaken == steps){ 		     // we increment steps if we have a successful move, if they are the same there has been an invalid command.
			printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN\n");
		}
		memset(userInput, '\0', sizeof(userInput));
		printf("\nCURRENT LOCATION: %s\n", rooms->current->name);
		int i = 0;
		printf("POSSIBLE CONNECTIONS:");
		while(rooms->current->connections[i][0]){	// while rooms i is valid
			printf(" %s", rooms->current->connections[i]);
			if(rooms->current->connections[i][0]){  // if the room is valid, print a comma
				printf(",");
			}
			i++;
		}
		printf(".\nWHERE TO? >"); // else terminate with a period, prompt with WHERE TO?
		fgets(userInput, 9, stdin); // takes user input from stdin
		i = 0;
		steps = game->stepsTaken;	
		setCurrentStatus = 1; // 1 == fail, 0 = user input matches a value displayed, current set.
		while(rooms->current->connections[i][0]){
			if(strcmp(rooms->current->connections[i], userInput)){ // if userInput matches a valid connection
				setCurrentStatus = setCurrent(rooms, userInput, game);// set current room to user input. returns 0 if success
				break;
			}
		}
	}
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n", game->stepsTaken);
	printJourney(game);
	printf("%s\n", rooms->current->name);

}

void getDirName(struct GameState * game){
	int newestDirTime = -1;
	char targetDirPrefix[32] = "trotterj.rooms.";
	char newestDirName[256];
	memset(newestDirName, '\0', sizeof(newestDirName));
	DIR * dirToCheck;
	dirToCheck = opendir(".");
	struct dirent *fileInDir;
	struct stat dirAttributes;
	if(dirToCheck > 0){
		while((fileInDir = readdir(dirToCheck)) != NULL){
			if(strstr(fileInDir->d_name, targetDirPrefix) != NULL){
				stat(fileInDir->d_name, &dirAttributes);
				if((int)dirAttributes.st_mtime > newestDirTime){
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName, '\0', sizeof(newestDirName));
					strcpy(newestDirName, fileInDir->d_name);
				}
			}
		}

	}
	closedir(dirToCheck);
	strcpy(game->filePath, newestDirName);
}

int main(){
	struct PathList * rooms = (struct PathList *) malloc(sizeof (struct PathList)); // create the room list 
	struct GameState * game = (struct GameState *) malloc(sizeof (struct GameState)); // create the game list
	initRoomList(rooms); // initialize the room list
	char fileName[256]; // get the filepath
	getDirName(game);
	strcpy(fileName, game->filePath); 
	populateList(rooms, fileName); // populate the lsit
	initGame(rooms, game);  // init the game
	playGame(rooms, game); // play loop
	return 0;
 }	
