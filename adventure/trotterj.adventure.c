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
	//printf("initRoom called\n");
	FILE* filePointer;
	filePointer = fopen(filePath, "r");

	//name - spaghetti code but it works. will clean up if I have time.
	int size;
	fseek(filePointer, 0L, SEEK_END);
	size = ftell(filePointer);
	rewind(filePointer);
	char buffer[size];
	memset(buffer, '\0', sizeof(buffer));
	fgets(buffer, size-1, filePointer);
	int i = 0;
	char bufferComparison = buffer[i];
	while(bufferComparison != ':'){
		i++;
		bufferComparison = buffer[i];
	}
	i=i+2; // i in buffer = starting character of the name
	char newName[9];
	memset(newName, '\0', sizeof(newName));
	int i2 = 0;
	bufferComparison = buffer[i];
	char newLine = '\n';
	while(buffer[i] && buffer[i] != newLine){
		newName[i2] = buffer[i];
		bufferComparison = buffer[i];
		i2++;
		i++;
	}
	memset(workingRoom->name, '\0', sizeof(workingRoom->name));
	strcpy(workingRoom->name, newName);
	//printf("NAME: %s\n", workingRoom->name);
	

	//connections
	char newConnection[9];
	int c = 0;
	while(size != ftell(filePointer)){
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
				//i2++;
				//i++;
				bufferComparison = buffer[i];
				i2++;
				i++;
			}
			strcpy(workingRoom->connections[c], newConnection);
			//printf("CONNECTION: %s\n", workingRoom->connections[c]);
			c++;	
		}
		//memset(newConnection, '\0', sizeof(newConnection));
		//memset(buffer, '\0', sizeof(buffer));
	}

	//type
	char newType[13];
	if(size == ftell(filePointer)){ // if filepointer is at end, its room status
		memset(newType, '\0', sizeof(newConnection));
		i = 0;
		bufferComparison = buffer[i];
		while(bufferComparison != ':'){
			i++;
			bufferComparison = buffer[i];
		}
		i=i+2;
		i2=0;
		bufferComparison = buffer[i];
		while(/*bufferComparison != newLine || i2 < 13*/buffer[i] && buffer[i] != newLine){
			newType[i2] = buffer[i];
			//i2++;
			//i++;
			bufferComparison = buffer[i];
			i2++;
			i++;
		}
		memset(workingRoom->type, '\0', sizeof(workingRoom->type));
		strcpy(workingRoom->type, newType);
		//printf("TYPE: %s\n", workingRoom->type);
	}
	fclose(filePointer);
}

void initRoomList(struct PathList * rooms){
	assert(rooms);
	rooms->Head = (struct PathLink *) malloc(sizeof (struct PathLink));
	rooms->size = 0;	
	//free
}

void initGameList(struct GameState * game){
	assert(game);
	game->Head = (struct RoomJourney *) malloc(sizeof (struct RoomJourney));
	game->stepsTaken = 0;
	//free
}

void populateList(struct PathList * rooms, char * dirName){
	 
	int i = 0;
	struct Room * newRoom;
	char * roomNames[9]; // is this getting initialized correctly?
	roomNames[0] = "fire_room";
	roomNames[1] = "water_room";
	roomNames[2] = "magic_room";
	roomNames[3] = "sword_room";
	roomNames[4] = "shield_room";
	roomNames[5] = "dragon_room";
	roomNames[6] = "living_room";
	roomNames[7] = "rainy_room";
	roomNames[8] = "coffee_room";
	roomNames[9] = "couch_room"; 
	char filePath[50];
	char backslash[50] = "/";
	int successes = 0;
	char tempName[50];
	while(successes < 7 && i < 10){
		memset(tempName, '\0', sizeof(tempName));
		strcat(tempName, dirName);
		strcat(tempName, backslash);
		strcat(tempName, roomNames[i]);
		memset(filePath, '\0', sizeof(filePath));
		strcpy(filePath, tempName);
		if(access(filePath, F_OK) != -1) {
   		 // file exists
   		 	printf("roomNames[9]   pre everything: %s\n", roomNames[9]);
			newRoom = (struct Room *) malloc(sizeof (struct Room));
			initRoom(rooms, newRoom, filePath); // ITS HAPPENING IN INITROOM TODO
			printf("roomNames[9]   post initroom(): %s\n", roomNames[9]);
			addToPathList(rooms, newRoom);
			printf("roomNames[9]   post addToPathList(): %s\n\n", roomNames[9]);
		
			successes++;
			
    		}
		i++;
		printf("successes: %i. i: %i. roomNames[i]:%s\n\n", successes, i, roomNames[i]);
	}
	if(i > 9 && successes < 7){
		printf("error - not enough valid files in %s\n", dirName);
	}
}

void initGame(struct PathList * rooms, struct GameState * game){ // finds room with type START_ROOM, sets rooms->current to START
	//printf("initGame called\n");
	initGameList(game);
	struct PathLink * tmp;
	tmp = rooms->Head;
	char* start = "START_ROOM";
	//printf("rooms->Head->value->name: %s\n", tmp->value->name);
	//printf("rooms->Head->value->type: %s\n", tmp->value->type);
	int cmp = 0;
	while(tmp){
		//printf("rooms->Head->value->name: %s\n", tmp->value->name);
	        //printf("rooms->Head->value->type: %s\n", tmp->value->type);
		if(tmp->value->type[0] != start[0])
		{
			tmp = tmp->Next;
		}
		else{
			//printf("current supposedly set\n");
			rooms->current = tmp->value;
			tmp = tmp->Next;
		} 
	}
	free(tmp);
}
int setCurrent(struct PathList * rooms, char * roomToFind, struct GameState * game){
	//printf("setcurrent called\n");
	struct PathLink * tmp;
	tmp = rooms->Head;
	int i;
	int cmp;
	while(tmp){
		cmp = 0;
		for(i = 0; i < 3; i++){
               		 if(tmp->value->name[i] != roomToFind[i]){
                	         cmp = 1;
        	         }
	         }

		if(cmp == 0 && rooms->current != tmp->value){
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

void playGame(struct PathList * rooms, struct GameState * game){ // TODO Implement timekeeping with mutexes.
	char userInput[9];
	char* end_room = "END_ROOM";
	char* time = "time";
	int setCurrentStatus = 1;
	int steps = -1;
	while(strcmp(rooms->current->type, end_room) != 0){
		if(game->stepsTaken == steps){
			printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN\n");
		}
		memset(userInput, '\0', sizeof(userInput));
		printf("\nCURRENT LOCATION: %s\n", rooms->current->name);
		int i = 0;
		printf("POSSIBLE CONNECTIONS:");
		while(rooms->current->connections[i][0]){
			printf(" %s", rooms->current->connections[i]);
			if(rooms->current->connections[i+1][0]){
				printf(",");
			}
			i++;
		}
		printf(".\nWHERE TO? >");
		fgets(userInput, 9, stdin);
		i = 0;
		steps = game->stepsTaken;	
		setCurrentStatus = 1; // 1 == fail, 0 = user input matches a value displayed, current set.
		if(strcmp(userInput, time)){
			
		}
		while(rooms->current->connections[i][0]){
			if(strcmp(rooms->current->connections[i], userInput)){ // if userInput matches a valid connection
				setCurrentStatus = setCurrent(rooms, userInput, game);// set current room to user input. returns 0 if successful
				//game->stepsTaken++;
				//addToJourney(game, rooms->current->name);
				break;
			}
			
			//if(setCurrentStatus != 0){
			//	printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN\n");
			//}
//			else{
//				game->stepsTaken++;
//				addToJourney(game, rooms->current->name);
//			}
	
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
					//printf("newer subdir: %s\n", fileInDir->d_name);
				}
			}
		}

	}
	closedir(dirToCheck);
	//printf("newest entry is %s\n", newestDirName);
	strcpy(game->filePath, newestDirName);
}

int main(){
	struct PathList * rooms = (struct PathList *) malloc(sizeof (struct PathList)); 
	struct GameState * game = (struct GameState *) malloc(sizeof (struct GameState));
	initRoomList(rooms);
	char fileName[256];
	getDirName(game);
	strcpy(fileName, game->filePath); // jank but w/e
	populateList(rooms, fileName);
	initGame(rooms, game); 
	//printf("CURRENT PLEASE SHOW NEXT TYPE: %s", rooms->current->type);
	playGame(rooms, game);
	return 0;
 }	
