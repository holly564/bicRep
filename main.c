/* PROGRAM:  main.c
   DATE:     2019-04-11
   TOPIC:    The program will randomly generate a number. The user will be prompted to guess the number, and will be
             told if their guess is correct or not.
   NOTES:    
*/

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "common.h"

int forkWithPipes(int *pdfRead, int *pdfWrite);
bool isUserGuessValid(uint8_t uiGuess);
void main_parent(int argc, char *argv[], int fdRead, int fdWrite);
void main_child(int fdRead, int fdWrite);
void doIdleTask();
void processInput(char *input);
void handleChildInterrupt(int sig);
extern bool parseArguments(int argc, char *argv[], uint8_t *pnMaxGuesses, uint8_t *pnMaxTime);

int main(int argc, char *argv[]) 
{    
  int fdRead;
  int fdWrite;
  int pid;
  
	//Create new process
	pid = forkWithPipes(&fdRead, &fdWrite);

	//Parent handles communication with user
	if (pid != 0) {
    main_parent(argc, argv,fdRead, fdWrite);
	}
	else{
    main_child(fdRead, fdWrite);
  }
  close (fdRead);
  close (fdWrite);
}

void main_parent(int argc, char *argv[], int fdRead, int fdWrite) {
  uint8_t guess = 0;
  uint8_t userGuess = 0;
  uint8_t maxGuesses = MAXUserGuess; 
  uint8_t timeOut = TIMEOUT; 
  char* strMessage = malloc (UCHAR_MAX * sizeof(char));
  char* strAnswer = malloc (UCHAR_MAX * sizeof(char));
  int rc;
  int n_read;
  int n_written;
   //Parse Cmd Line Args
  parseArguments(argc, argv, &maxGuesses, &timeOut);

  //Prompt user
    strMessage = "Enter a number between 0-";
    strcat(strMessage, (const char*)MAXUserGuess);
    strcat(strMessage, " (you have ");
    strcat (strMessage, (const char*)TIMEOUT);
    strcat (strMessage, " seconds to make a guess):");

    rc = scanf("%ui", &guess);
    //Validate 
    if (1 == rc){
      if (1 == isUserGuessValid(guess)){
      n_written = write(fdWrite, &guess, sizeof(guess) + 1);

      wait(NULL);
 
      n_read = read(fdRead, &strAnswer, UCHAR_MAX);

      //SIGUSR1 handler "You lost game"
      signal( SIGUSR1, handleChildInterrupt ); 
      //SIGUSR2 handler "Time is out"
      signal( SIGUSR2, handleChildInterrupt ); 

      userGuess = atoi((const char*)strAnswer);

      if (userGuess == guess)
        printf("Congratulations, you guessed the number (%d) correctly!\n", guess);
      else if (userGuess > guess) 
        printf("Our guess was too high\n");
      }
      else
        printf("Our guess was too low\n");
    }
  free(strMessage);
  free(strAnswer);
}

void main_child(int fdRead, int fdWrite) {
  uint8_t uiExpectedGuess = 0;
  char* strAnswer = malloc (UCHAR_MAX * sizeof(char));
  int flags;
  int n_read;
  int n_written;
  
  //Child must not block while reading
  flags = fcntl(fdRead, F_GETFL, 0);
  //Try to set non blocking flag
  if (fcntl(fdRead, F_SETFL, flags | O_NONBLOCK ))
    fprintf( stderr, "Could not set non-blocking flag\n");

  while(true){
    n_read = read(fdRead, &strAnswer, UCHAR_MAX);
    if (n_read>0)
      ;//processInput(strAnswer);
    else if (n_read == 0)
      ; //Pipe closed
    else if (errno == EAGAIN){
      doIdleTask();
      sleep(1);
    }
  }//endwhile
  
  n_written = write(fdWrite, &uiExpectedGuess, sizeof(uiExpectedGuess)+ 1);
  free(strAnswer);
}

int forkWithPipes(int *pdfRead, int *pdfWrite){
  static int parentToChildPipe[2];  //parent to child Pipe
  static int childToParentPipe[2];  //child to parent Pipe
  int* pfdRead;
  int* pfdWrite;

   if (-1 == pipe(parentToChildPipe)){
    fprintf(stderr, "Pipe Failed" ); 
    return -1;
  }
  if (-1 == pipe(childToParentPipe)){
    fprintf(stderr, "Pipe Failed" ); 
    return -1;
  }

    int pid = fork();   

    if(pid < 0) { 
        fprintf(stderr, "Fork Failed" ); 
        return 1; 
    }  

    if ( pid != 0 ) {
    	//Translating pipe to single entity
       close (parentToChildPipe[0]); //closes Reading half of parenttochildp
       close (childToParentPipe[1]); // closes Writing half of childtoparentpipe
      *pfdWrite = parentToChildPipe[1];
      *pfdRead = childToParentPipe[0];
    } 
    else {        
    close (parentToChildPipe[1]); //closes Writing half of parenttochildp
    close (childToParentPipe[0]); // closes Reading half of childtoparentpipe
    *pfdWrite = childToParentPipe[1];
    *pfdRead = parentToChildPipe[0];
    }
    return pid;
}

bool isUserGuessValid(uint8_t uiGuess){
	(uiGuess < MAXUserGuess && uiGuess > MINUserGuess) ? true : false;
}

void doIdleTask(){
  sleep(5);
  printf("Log Statement\n");
}

void processInput(char *input) {
  uint8_t guess = calcExpectedGuess();
  int PARENT_PID = getppid();

  //Assessing user's guess
  for (int i = MAX_TIME; i >= 0; i--)
  {
    doIdleTask();
    if (i <= 5)
    {
        kill (PARENT_PID, SIGUSR2);
    }
  }
}

int calcExpectedGuess() {
  uint8_t calcGuess = 0;

  srand(time(NULL));
  calcGuess = (rand() % 100);
  return calcGuess;
}

void handleChildInterrupt(int sig) {
    printf("Time is running out!");
}


