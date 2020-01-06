#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>



int changeTheSpot(char**);
int getTheStatus(int, int, int);
void exitTheThings(char*, int []);
void addPid(int [], int);
void backgroundWait(int []);
int backgroundEnable = 1;

void catchSIGTSTP(int signo)
{
	if (backgroundEnable == 1){
		char* message = "Entering foreground-only mode\n";
		write(STDOUT_FILENO, message, 29);
		backgroundEnable = 0;
	}
	else{
		char* message = "Leaving foreground-only mode\n";
		write(STDOUT_FILENO, message, 30);
		backgroundEnable = 1;
	}
	
}

void main()
{
	struct sigaction SIGTSTP_action = {0};
	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	//SIGINT_action.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);
	signal(SIGINT, SIG_IGN); 

	int numCharsEntered = -5; // How many chars we entered
	int currChar = -5; // Tracks where we are when we print out every char
	size_t bufferSize = 0; // Holds how large the allocated buffer is
	char* lineEntered = NULL; // Points to a buffer allocated by getline() that holds our entered string + \n + \0
	
	char owd[1024]; //original working directory
	getcwd(owd, sizeof(owd)); // get the path to the directory that the program was run in
	int lastExit = 3; // 3 = no commands run, 1 = built in commnad, 0 = forked a child
	pid_t spawnPid = -5;
	int childExitStatus = -5;

	
	
	
	
	int pidArray[100];
	int p;
	for(p=0; p<100; p++){
		pidArray[p] = -5;
	}
	
	
	
	
	while(1){
		// Get input from the user
		int builtInExit = -5;
		backgroundWait(pidArray);
		while(1)	
		{
			printf(": ");
			fflush(stdout);
			numCharsEntered = getline(&lineEntered, &bufferSize, stdin); // Get a line from the user
			if (numCharsEntered == -1)
				clearerr(stdin);
			else
				break; // Exit the loop - we've got input
		}
		lineEntered[strcspn(lineEntered, "\n")] = '\0'; // Remove the trailing \n that getline adds
		
		//THIS IS WHERE EXPANDING $$ NEEDS TO GO
		
		pid_t pidInt = getpid();
		char* pidStr;
		pidStr = malloc(1000*sizeof(char));
		sprintf(pidStr, "%d", pidInt);
		int pidLen = strlen(pidStr);
		
		
		char buffer[1024] = { 0 };
		char *insert_point = &buffer[0];
		const char *tmp = lineEntered;
		char* lineEnteredFixed;
		lineEnteredFixed = malloc(3000 * sizeof(char)); // make it really big incase the pid bloats the input

		while (1) {
			const char* spot = strstr(tmp, "$$");

			if (spot == NULL) { //found the last spot where $$ shows up
				strcpy(insert_point, tmp);
				break;
			}

        // copy part before $$
			memcpy(insert_point, tmp, spot - tmp);
			insert_point += spot - tmp;

        // copy pid
			memcpy(insert_point, pidStr, pidLen);
			insert_point += pidLen;

        // adjust pointer
			tmp = spot + 2;
		}

    // coppy fixed string out of buffer into it's final spot
		strcpy(lineEnteredFixed, buffer);
		//printf(lineEnteredFixed);

		
		char* temp;
		int index=0, i=0;
		char** lineArray;
		lineArray = malloc(512 * (sizeof(char*))); // 512 is max number of arguments
		for(i=0; i<512; i++){
			lineArray[i] = NULL;
		}
		
		
		temp = strtok(lineEnteredFixed, " ");
		while(temp != NULL){
			lineArray[index] = malloc(50*(sizeof(char)));
			strcpy(lineArray[index], temp);
			strcat(lineArray[index], "\0");
			temp = strtok(NULL, " ");
			index++;
		}
		int numToks = index + 1; // number of things input on the command line
		
		if(lineArray[0] != NULL && lineArray[0][0] != '#'){
			if(strcmp(lineArray[0],"cd") == 0){
				builtInExit = changeTheSpot(lineArray);
				lastExit = 1;
			}
			else if(strcmp(lineArray[0],"status") == 0){
				builtInExit = getTheStatus(lastExit, builtInExit, childExitStatus);
				lastExit = 1;
			}
			else if(strcmp(lineArray[0],"exit") == 0){
				exitTheThings(owd, pidArray);
			}
			else{
				
				
				int background = 0;
				int output = 0;
				int input = 0;
				int numArgs = 0;
				int iter = 0;
				int newIn;
				int newOut; 

				
				while(1){
					if(lineArray[iter] == NULL){
						break;
					}
					else if(strcmp(lineArray[iter],"<") == 0){
						// set redirection flag, increase iter past filename
						iter++;
						newIn = open(lineArray[iter], O_RDWR, 0664);
						if(newIn == -1){
							printf("Unable to open input file\n");
							fflush(stdout);

						}
						input = 1;
						iter ++;
					}
					else if(strcmp(lineArray[iter], ">") == 0){
						//setredirection flag, increase iter past filename
						iter++;
						newOut = open(lineArray[iter], O_RDWR | O_CREAT | O_TRUNC, 0644);
						if(newOut == -1){
							printf("Unable to open output file\n");
							fflush(stdout);
							//exit(1);
						}
						output = 1;
						iter ++;
					}
					else{
						iter++;
						numArgs++;	
					}
				}
				
				if(strcmp(lineArray[iter-1],"&") == 0){ //and not in no-background-mode
					if(backgroundEnable == 1){
						background = 1;
					}
					//printf("Found a &\n");
					//fflush(stdout);
					//lineArray[iter - 1] = "";
					numArgs--;
					iter--;
					
				}
				
				
				lastExit = 0;
				spawnPid = -5;
				childExitStatus = -5;
				spawnPid = fork();
				switch (spawnPid) {
					case -1: { perror("Hull Breach!\n"); exit(1); break; }
					case 0: { // in child
						signal(SIGINT, SIG_DFL);
						pid_t currPid = getpid();
						if(background == 1){
							printf("background pid is %d\n", currPid);
							fflush(stdout);
						}
						char** arguments;
						arguments = malloc((numArgs*sizeof(char*))+1);
						arguments[numArgs] = NULL;
						int recordedArgs = 0;
						int i =0;
						while(i < iter){
							if(strcmp(lineArray[i],"<") == 0){
								i++;
								if(newIn == -1){
									exit(1);
								}
								int result = dup2(newIn, 0);
								close(newIn);
								i++;
							}
							else if(strcmp(lineArray[i],">") == 0){
								// increase i past filename do redirection
								i++;
								if(newOut == -1){

									exit(1);
								}
								int result = dup2(newOut, 1);
								close(newOut);
								i++;
							}
							else{
								arguments[recordedArgs] = malloc(150*sizeof(char));
								strcpy(arguments[recordedArgs], lineArray[i]);
								recordedArgs++;
								i++;	
							}
						}
						
						if(background == 1 && input ==0){
							//send to dev null
							int nullIn = open("/dev/null", O_RDONLY);
							dup2(nullIn, 0);
						}
						if(background ==1 && output == 0){
							// send to dev null
							int nullOut = open("/dev/null", O_WRONLY);
							dup2(nullOut, 0);
							
						}
						
						
						
						/*int m;
						for(m=0; m<numArgs; m++){
							printf(arguments[m]);
							fflush(stdout);
							printf("\n");
							fflush(stdout);
							
						} */
						
						
						
						execvp(arguments[0],arguments);
						perror("Could not run that command\n");
						exit(1);
						break;
					}
					default: {
						//printf("PARENT(%d): Sleeping for 2 seconds\n", getpid());
						sleep(1); // without this errors happen and i don't know why
						//printf("PARENT(%d): Wait()ing for child(%d) to terminate\n", getpid(), spawnPid);
						
						if(background == 0){
							pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
							if (!WIFEXITED(childExitStatus)){
								int termSignal = WTERMSIG(childExitStatus);
								printf("terminated by signal %d\n", termSignal);
								fflush(stdout);
							}

						}
						else if(background == 1){
							// add child pid to array of child pids
							addPid(pidArray, spawnPid);
						}
						
						break;
					}
				}
			}
		}
		
		free(lineEntered); // Free the memory allocated by getline() or else memory leak
		lineEntered = NULL;
	}
}



int changeTheSpot(char** lineArray){
	if(lineArray[1] == NULL){
		return chdir(getenv("HOME"));
	}
	else{
		return chdir(lineArray[1]);
	}
}

int getTheStatus(int lastExit, int builtInExit, int childExitStatus){
	if(lastExit == 3){
		printf("no commads run yet\n");
		fflush(stdout);
	}
	else if(lastExit == 1){
		printf("exit value %d\n", builtInExit);
		fflush(stdout);
	}
	else if(lastExit == 0){
		if (WIFEXITED(childExitStatus)){
			//printf("The process exited normally\n");
			int exitStatus = WEXITSTATUS(childExitStatus);
			printf("exit value %d\n", exitStatus); 
			fflush(stdout);
		}
		else {
			int termSignal = WTERMSIG(childExitStatus);
			printf("terminated by signal %d\n", termSignal);
			fflush(stdout);
		}
	}
	
}


void exitTheThings(char* owd, int pidArray[]){
	chdir(owd);
	int i;
	for(i=0; i<100; i++){
		if(pidArray[i] != -5){
			kill(pidArray[i], SIGTERM);
		}
	}
	exit(0);
}

void backgroundWait(int pidArray[]){
	int i,childExitMethod;
	pid_t result;
	
	for(i=0; i<100; i++){
		if(pidArray[i] != -5){
			result = waitpid(pidArray[i], &childExitMethod, WNOHANG);
			//if it exited, see how it did and print, same for signals, the set index to -5
			
			if(result !=0 && result != -1){
				if (WIFEXITED(childExitMethod)){
			//printf("The process exited normally\n");
					int exitStatus = WEXITSTATUS(childExitMethod);
					printf("Background pid %d is done exit value %d\n", pidArray[i], exitStatus); 
					fflush(stdout);
				}
				else {
					int termSignal = WTERMSIG(childExitMethod);
					printf("Background pid %d terminated by signal %d\n", pidArray[i], termSignal);
					fflush(stdout);
				}
				pidArray[i] = -5;
			}
		}
	}
}

void addPid(int pidArray[], int newPid){
	int i;
	for(i=0; i<100; i++){
		if(pidArray[i] == -5){
			pidArray[5] = newPid;
		}
	}
}


