#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>

pid_t active_processes[200];
pid_t foreground_process = -5;
int num_children = 0;
int num_args = 0;
int background = 1; // 1 = false, 0 = true
int exit_shell = 0;
int last_status;
int input_redirected = 1;
int output_redirected = 1;
int background_mode = 0; // 0 = off, 1 = on
int sig_or_status = 0; // signal = 0, status = 1

void backgroundRedirect(){
	int devnull = open("/dev/null", O_RDWR);
	if(input_redirected == 0){ // if input not redirected, set it to the void
		dup2(devnull, 0);
	}
	if(output_redirected == 0){// if output not redirected, set it to the void
		dup2(devnull, 1);
	}
}	

void backgroundSet(char args[512][2048]){
	if(!strcmp(args[num_args], "&")){ //if the last character is &..
		if(background_mode == 0){ //..and foreground mode is not toggled..
			background = 0;   //..set the background flag.
		}
		num_args--; //decrement the num_args global to effectively phase the character out of our arguments
	}
}

void handleRedirect(char args[512][2048]){ 
	int a;
	int b;
	int infd;
	int outfd;
	for(a = 0; a <= num_args; a++){
		if(!strcmp(args[a], "<")){
			if(strlen(args[a+1]) > 0){ // Argument for input redirection exists, set stdin.
				input_redirected = 0;
				infd = open(args[a+1], O_RDWR);
				if(infd >= 0){ // if its valid, set stdin and parse out "< arg" from the arguments
					dup2(infd, 0);
					b = a;
					while((b+2) <= num_args){
						strcpy(args[b],args[b+2]);
						b++;
					}
					num_args = num_args-2;	
					a = 0;
				}
				else{ // else notify the user
					printf("cannot open file for input\n");
					fflush(stdout);
					exit(1);
				}
			}
			else{ // if just < input, notify and parse it out.
				printf("No argument for input redirection! Ignoring...\n");
				fflush(stdout);
				b = a;
				while((b+1) <= num_args){
					strcpy(args[b], args[b+1]);
					b++;
				}
				num_args--;
				a = 0;
				
			}
		}
		if(!strcmp(args[a], ">")){
			if(strlen(args[a+1]) > 0){ // Argument exists, set stdout.
				output_redirected = 0;
        	                outfd = creat(args[a+1], 0644);
				if(outfd >= 0){ // if its valid, set stdout and parse out "> arg" from the arguments
	                       		dup2(outfd, 1); 
                       			b = a;
                	      		while((b+2) <= num_args){
						strcpy(args[b], args[b+2]);
                      			}
					num_args = num_args-2;    
					a = 0;
				}	
				else{ // else notify the user
					printf("cannot open file for output\n");
					fflush(stdout);
					exit(1);
				}
			}
			else{ // if just > input, notify user and parse it out
				printf("No argument for output redirection! Ignoring...\n");
				fflush(stdout);
				b = a;
				while((b+1) <= num_args){
					strcpy(args[b], args[b+1]);
					b++;
				}
				num_args--;
				a = 0;
			}
                }

	}
	backgroundSet(args); //handle &
}

void executeExit(){
	int i=0;
	for(i = 0; i < num_children; i++){ 
		if(active_processes[i] != -5){ //goes through the list of active processes and kills them all 
			printf("Killing process %i\n", active_processes[i]);
			kill(active_processes[i], SIGKILL);
		}
	}	
	exit_shell = -1; // breaks the shell loop.
	exit(0);
}

void executeCd(char *args){
	int dirStatus;
	char *dirPath = (char *) malloc(sizeof (char *) * (strlen(args)+strlen(getenv("PWD"))));
	char *dir;
	if(args[0] != '/'){ //if user did not input an absolute path, append current path to make it relative
		sprintf(dirPath, "%s/%s", getenv("PWD"), args);
	}
	if(!strcmp(args, "")){ //if user just input "cd", go home
		dirStatus = chdir(getenv("HOME"));
		setenv("PWD", getenv("HOME"), 1);
		dir = getenv("PWD");
		printf("%s\n", dir);
		fflush(stdout);
	}
	else{
		dirStatus = chdir(args); // change the directory
		if(dirStatus == -1){	 // if it failed, notify the user
			dir = getenv("PWD");
			printf("Could not change directories.\n%s\n", dir);
			fflush(stdout);
		}
		else{			// if it didn't, update PWD
			setenv("PWD", dirPath, 1);
			dir = getenv("PWD");
			printf("%s\n", dir);
			fflush(stdout);
		}
	}
}

void executeStatus(){ //checks the last_status and sig_or_status globals, prints accordingly
	if(sig_or_status == 1){
		printf("terminated by signal %i\n", last_status);
		fflush(stdout);
	}
	else if (sig_or_status == 0){
		printf("exit value: %i\n", last_status); 
		fflush(stdout);
	}
}

void execute(char args[512][2048]){
	pid_t spawnpid = -5;
	int childExitMethod = -5;
	struct sigaction ignore_action = {0};
	spawnpid = fork();
	switch(spawnpid){
		case -1:
			perror("RED ALERT HULL BREACH");
			fflush(stdout);
			exit(1);
			break;
		case 0:
			ignore_action.sa_handler = SIG_IGN; //CTRL-Z should not kill the child
			sigaction(SIGINT, &ignore_action, NULL);
			sigaction(SIGTSTP, &ignore_action, NULL);
			handleRedirect(args);
			int x;
			char *arguments[512];	
			for(x = 0; x <= num_args; x++){ //do some string fixing to prepare for execvp
				arguments[x] = args[x];
			}
			if (background == 0){		//call backgroundRedirect, which redirects to the void
				backgroundRedirect();
			}
			if (execvp(arguments[0], arguments) < 0){ //execvp is here
				perror("Could not execute!");
				fflush(stdout);
				exit(1);
			}
		default:
			backgroundSet(args);		//in the parent, check if we're in the background
			if (background == 0){		//if we are, add it to the list of processes, then move on
				printf("Background pid of %s is %i\n", args[0], spawnpid);
				fflush(stdout);
				active_processes[num_children] = spawnpid;
				num_children++;
			}
			if (background == 1){		//if we aren't, wait for the process to finish
				foreground_process = spawnpid;
				waitpid(spawnpid, &childExitMethod, 0);
				if (WIFEXITED(childExitMethod)){	// if exited normally, handle it
					last_status = WEXITSTATUS(childExitMethod);		
					foreground_process = -5;
					sig_or_status = 0;
				}
				else{					// if terminated, let CTRL-C handle it and set some flags
					foreground_process = -5;
					sig_or_status = 1;
				}
			}		
	}	
}

void handleInput(char args[512][2048]){ //input handler, handles comments and redirects to corresponding functions
	if(args[0][0] == '#'){
		return;
	}
	else if(!strcmp(args[0], "exit")){
		executeExit();
	}
	else if(!strcmp(args[0], "cd")){
		executeCd(args[1]);
	}
	else if(!strcmp(args[0], "status")){
		executeStatus();
	}
	else{	
		execute(args);
	}
}

int shellPrompt(){
	input_redirected = 0;
	output_redirected = 0;
	int k;
	int childExitMethod= -5;
	for(k = 0; k < num_children; k++){// check if forked children pids have finished.
		if(waitpid(active_processes[k], &childExitMethod, WNOHANG) != 0 && active_processes[k] != -5){
			printf("Background process %i completed! ", active_processes[k]); // if they've finished, print a message.
			fflush(stdout);	
			if (WIFEXITED(childExitMethod)){
				printf("exit status %i\n", WEXITSTATUS(childExitMethod)); // print exit status if finished normally
				fflush(stdout);
			}
			else{
				printf("terminated by %i\n", WTERMSIG(childExitMethod)); // print termination status if killed by signal
				fflush(stdout);
			}
			active_processes[k] = -5;
		}		
	}
	char userInput[2048];
	char args[512][2048];	
	printf(": ");
	fflush(stdout);
	fgets(userInput, 2048, stdin);	
	int iPtr = 0;
	int argCount = 0;
	int argSPtr = 0;
	while(userInput[iPtr] != '\0'){ //manually tokenize the input because I was having problems with strtok
		if(userInput[iPtr] == ' '){
			argCount++;
			argSPtr = 0;
		}
		else{
			if (userInput[iPtr] != '\n'){
				args[argCount][argSPtr] = userInput[iPtr];
				argSPtr++;
			}
		}
		iPtr++;
	}
	num_args = argCount;
	int ppid = getpid(); // get the current pid
	char *charpid = malloc(6); // malloc string pid
	sprintf(charpid, "%d", ppid); 
	int y;
	int a = 0;
	int tempStrLen;
	int b;
	int pidLen = strlen(charpid);
	int pidptr;
	for(y = 0; y < 512; y++){	// expand $$ to the shell pid before any handling takes place
		a = 0;
		pidptr = 0;
		tempStrLen = strlen(args[y]);
		while(a < 2000){
			if(args[y][a] == '$' && args[y][a+1] == '$'){
				while(tempStrLen > a+pidLen){ //shift the string to the right to allocate space for the pid
					if(tempStrLen+pidLen > 2048){
						printf("Cannot expand $$ to pid - out of range\n");
						fflush(stdout);
					}
					else{
						args[y][tempStrLen+pidLen] = args[y][tempStrLen];
						tempStrLen--;
					}
				}
				for(b = a; b < (a+pidLen); b++){
					args[y][b] = charpid[pidptr];
					pidptr++;
				}
			}
			a++;
		}
	}
	if(strcmp(args[0],"")){// if the user hits enter, do nothing and reprompt.
		handleInput(args);
	}
	for(y = 0; y < 512; y++){ //clear out our args for the next shell cycle
		memset(args[y], '\0', strlen(args[y]));
	}
	memset(userInput, '\0', strlen(userInput));
	return exit_shell;//exit_shell set to 0, -1 if user quits.
}

void catchSIGINT(int signo){ //kills the current foreground process
	if(foreground_process != -5){
		kill(foreground_process, SIGKILL);
	}
	last_status = signo;
	sig_or_status = 1;
	char* newline = "\n";
	write(STDOUT_FILENO, newline, 1);
	executeStatus();
}

void catchSIGTSTP(int signo){ // toggles foreground-only mode
	char* onMessage = "\nEntering foreground-only mode (& is now ignored)\n";
	char* offMessage = "\nExiting foreground-only mode.\n";
	if(background_mode == 0){
		write(STDOUT_FILENO, onMessage, 50);
		background_mode = 1; 
	}
	else if(background_mode == 1){
		write(STDOUT_FILENO, offMessage, 31);
		background_mode = 0;
	}
}

int main(){
	struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0}; // handling CTRL-C and CTRL-Z signal inputs
	
	SIGINT_action.sa_handler = catchSIGINT;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;

	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;
	
	sigaction(SIGINT, &SIGINT_action, NULL);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	int shell = -5;
	while(shell != -1){ // runs the shell until user quits
		background = 1;
		shell = shellPrompt();
	}
	return 0;
}
