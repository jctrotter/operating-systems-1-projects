#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

typedef struct Room{
	char name[9]; //prev 14
	char type[10]; 
	int maxConnections;
	int numConnections;
	struct Room * connections[6];	
}Room;

typedef struct Map{
	Room ** table;
	int size;
}Map;

void initMap(struct Map * gameMap, int gameSize){
	assert(gameMap);
	gameMap->table = (struct Room **) malloc(sizeof(struct Room *) * gameSize);
	assert(gameMap->table != 0);
	gameMap->size = gameSize;
	int a;
	for(a = 0; a < gameSize; a++)
	{
		gameMap->table[a] = 0;
	}
}

void addRoomToMap(struct Map * gameMap, char* roomName, int currentRoomNum, int outbound){	
	char room[9];
	memset(room, '\0', sizeof(room));
	strcpy(room, roomName);
	struct Room *newRoom = (struct Room *) malloc(sizeof(struct Room)); // allocates memory to the newRoom object
	assert(newRoom);
	memset(newRoom->name, '\0', sizeof(newRoom->name)); //
	strcpy(newRoom->name, room);
	newRoom->maxConnections = outbound; // outbound = random int within bounds
	newRoom->numConnections = 0;
	memset(newRoom->type, '\0', sizeof(newRoom->type)); //
	if (currentRoomNum == 0){
		strcpy(newRoom->type, "START_ROOM");
	}
	else if (currentRoomNum > 0 && currentRoomNum < 6){
		strcpy(newRoom->type, "MID_ROOM");
	}
	else if (currentRoomNum == 6){
		strcpy(newRoom->type, "END_ROOM");
	}
	gameMap->table[currentRoomNum] = newRoom;
	assert(gameMap->table[currentRoomNum]);
	
}

int makeDir(){
	int pid = getpid(); // get the pid
        char* newpid = malloc(6); // allocate memory for the string that the pid will go to
        sprintf(newpid, "%d", pid); // put the pid into place
        char* onidpath = "./trotterj.rooms."; 
        char dirpath[100];
        strcpy(dirpath, onidpath);
        strcat(dirpath, newpid);
	mkdir(dirpath, 0700); // make the directory ./trotterj.rooms.[PID]
	return pid;
}

void createConnections(struct Map * gameMap){
	// OUTBOUND CONNECTING CRITERIA:
	// - 3-6 connections, at random
	// - must be doubly linked - A connects to B and B connects to A
	// - cannot connect to self
	// - cannot connect to the same place twicei
	int z;
	int c;
	int randVal, tempConnection, i, tempConnectionNumConnections;
	int max = 6;
	int min = 0;
	struct Room * workingRoom;
	int connected = 0; //var to check if room has been connected to yet
	for (z = 0; z < 7; z++) // for each room
	{
		workingRoom = gameMap->table[z]; 
		c = workingRoom->numConnections; 
		while(c < workingRoom->maxConnections) // while we have connections left
		{
			tempConnection = (rand() % (max + 1 - min)) + min;
			connected = 0;
	                if(tempConnection != z)	// if temp conneciton is not self...
			{				 
				for(i = 0; i < gameMap->table[tempConnection]->numConnections; i++) // and our current working name is not in the target list of names
				{	
					if (strcmp(gameMap->table[tempConnection]->connections[i]->name, workingRoom->name) == 0){  					
						connected = 1;
					}	
				}
				if(connected == 0)
				{
					//...set outbound connection...
					workingRoom->connections[workingRoom->numConnections] = gameMap->table[tempConnection];
					workingRoom->numConnections++;
					//...set returning connection.
					gameMap->table[tempConnection]->connections[gameMap->table[tempConnection]->numConnections] = workingRoom;	
					gameMap->table[tempConnection]->numConnections++;
					c++;
				}
			}
		}
	}
}

void makeRooms(struct Map * gameMap){
	/*Designate room file to create*/
        char* roomNames[10];
        roomNames[0] = "fire";			
        roomNames[1] = "water";				
        roomNames[2] = "magic";
        roomNames[3] = "sword";
        roomNames[4] = "dragon"; // TODO - doesn't read properly? segfaults somewhere
        roomNames[5] = "shield";
        roomNames[6] = "living";
        roomNames[7] = "electric";
        roomNames[8] = "coffee";
        roomNames[9] = "tear";
        int randVal, randBounded;
	int i = 0;
	int max = 9;
	int min = 0;
        int roomsMade[10]; // 0 if room is not made, 1 if made
	for (i = 0; i < 10; i++){
		roomsMade[i] = 0;
	}
	i = 0;
        int rooms[7]; // room order. ex 763 == electric, living, magic
        while (i < 7) // populates the rooms array.
        {	
                randVal = (rand() % (max + 1 - min)) + min; 
                if (randVal == 0 && roomsMade[0] != 1) //fire
                {
                        roomsMade[0] = 1;
                        rooms[i] = 0;
                        i++;
                }
                if (randVal == 1 && roomsMade[1] != 1) //water
                {
                        roomsMade[1] = 1;
                        rooms[i] = 1;
                        i++;
                }
                if (randVal == 2 && roomsMade[2] != 1) //magic
                {
                        roomsMade[2] = 1;
                        rooms[i] = 2;
                        i++;
                }
                if (randVal == 3 && roomsMade[3] != 1) //sword
       		{
                        roomsMade[3] = 1;
                        rooms[i] = 3;
                        i++;
                }
                if (randVal == 4 && roomsMade[4] != 1) //dragon  
                {
                        roomsMade[4] = 1;
                        rooms[i] = 4;
                        i++;
                }
                if (randVal == 5 && roomsMade[5] != 1) //shield
                {
                        roomsMade[5] = 1;
                        rooms[i] = 5;
                        i++;
                }
                if (randVal == 6 && roomsMade[6] != 1) //living
                {
                        roomsMade[6] = 1;
                        rooms[i] = 6;
                        i++;
                }
                if (randVal == 7 && roomsMade[7] != 1) //electric
                {
                        roomsMade[7] = 1;
                        rooms[i] = 7;
                        i++;
                }
                if (randVal == 8 && roomsMade[8] != 1) //coffee
                {
                        roomsMade[8] = 1;
                        rooms[i] = 8;
                        i++;
                }
                if (randVal == 9 && roomsMade[9] != 1) //tea
                {
                        roomsMade[9] = 1;
                        rooms[i] = 9;
                        i++;
                }
        }
	int j;
	max = 6;
	min = 3;
	for(j = 0; j < 7; j++)
	{
		randVal = (rand() % (max+1-min)) + min;
		addRoomToMap(gameMap, roomNames[rooms[j]], j, randVal);
	}
	return;
	

}

void makeFiles(struct Map * gameMap, char * directory){
	FILE * filePointer;
	char fileName[60];// needs to be 39
	int h, g;
	char* roomName = "ROOM NAME: ";
	char* connection = "CONNECTION ";
	char* roomType = "ROOM TYPE: ";
	char* name_room = "_room";
	char tempName[20];
	char gchar[1];
	char connectionName[100];
	int z;
	for(h = 0; h < gameMap->size; h++){ 
 		strcpy(fileName, directory); // assemble the filename location
		strcat(fileName, "/"); //1
		strcat(fileName, gameMap->table[h]->name); //9
		strcat(fileName, name_room); //6	
		filePointer = fopen(fileName, "w"); // open the assembled filename
		assert(filePointer);
		fputs(roomName, filePointer); // write the name to the file
		fputs(gameMap->table[h]->name, filePointer);
		fputs("\n", filePointer);
		for(g = 0; g < gameMap->table[h]->numConnections; g++){ //write the connections to the file
			strcpy(connectionName, connection);
			sprintf(gchar, "%d", g+1);
			strcat(connectionName, gchar);
			strcat(connectionName, ": ");
			strcpy(tempName, gameMap->table[h]->connections[g]->name);
			strcat(connectionName, tempName);
			fputs(connectionName, filePointer);
			fputs("\n", filePointer);
		}
		fputs(roomType, filePointer); //write the room type to the file
		fputs(gameMap->table[h]->type, filePointer);
		fputs("\n", filePointer);
		fclose(filePointer); // intermittent segfaults on closure
	}
}
int main(int argc, char *argv[]){
	srand(time(NULL));		
	int p_id = makeDir();
	struct Map * gameMap = (struct Map *) malloc(sizeof(struct Map));
	initMap(gameMap, 7);
	makeRooms(gameMap);
	createConnections(gameMap);
	char directory[100];
	char* charpid = malloc(6);
	sprintf(charpid, "%d", p_id);
	char* onid = "./trotterj.rooms."; //17. +7 for pid
	strcpy(directory, onid);
	strcat(directory, charpid);
	makeFiles(gameMap, directory);	
	
	return(0);
}
