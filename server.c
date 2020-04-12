#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/signal.h>

#define MAXPENDING 5
#define BUFFSIZE 2048
#define MAX 2048

void action() {};
void player(char *, int, int *, int);

void ErrExit(char * mess) {
  perror(mess);
  exit(1);
}

void servicePlayers(int player1, int player2) {
  pid_t pid1, pid2;
  char * players[2] = {
    "TOTO",
    "TITI"
  };
  int sharedSegment;
  int * sharedArray;

  // get a shared segment (check for fail)
  sharedSegment = shmget(IPC_PRIVATE, 2 * sizeof(double), IPC_CREAT | 0600);
  sharedArray = shmat(sharedSegment, NULL, 0); // get your shared array

  if ((pid1 = fork()) == 0) 
	player(players[0], 0, sharedArray, player1);
 
  if ((pid2 = fork()) == 0) 
     player(players[1], 1, sharedArray, player2);

  sleep(1);
  signal(SIGUSR1, action); //install handler for the signal, handler here is action function doing nothing

  while (1) {
    printf("\nReferee: TOTO plays\n");
   
    kill(pid1, SIGUSR1);
    pause();

    printf("\nReferee: TITI plays\n");
    
    kill(pid2, SIGUSR1);
    pause();

    printf("\nReferee: Total points so far:\n TOTO: %d\n TITI: %d\n", sharedArray[0], sharedArray[1]);

    if (sharedArray[0] >= 50 || sharedArray[1] >= 50) {
	if (sharedArray[0] >= 50) {
		printf("Game over: TOTO won the game\n");
		write(player1, "Game over: you won the game", 255);
		write(player2, "Game over: you lost the game", 255);
        } else {
		printf("Game over: TITI won the game\n");
		write(player2, "Game over: you won the game", 255);
		write(player1, "Game over: you lost the game", 255);
	}

        kill(pid1,SIGTERM); //kill the child process pid1 to release the socket port
        kill(pid2,SIGTERM); //kill the child process pid2 to release the socket port
	exit(20);
    }
  }
}

void player(char * name, int playerId, int * sharedArray, int sd) {
  int points = 0, dice, status, n;
  long int ss = 0;
  pid_t pid;
  char message[255];

  while (1) {
    signal(SIGUSR1, action); // block myself
    pause();

    write(sd, "You can now play", 255);
    
        if (n = read(sd, &dice, sizeof(dice))) {
          
          //message[n] = '\0';
          printf("%d\n", dice);
          //sscanf(message, "%d", &dice); // Using sscanf to parse char to int
         
        }
      
    
   points += dice;
   sharedArray[playerId] = points;
    
    sleep(3);
    kill(getppid(), SIGUSR1);
  }
}

int main(int argc, char const * argv[]) {

  int sd, player1, player2, portNumber, status;
  struct sockaddr_in servAdd; // server socket address

  if (argc != 2) {
    printf("Call model: %s <Port Number>\n", argv[0]);
    exit(0);
  }

  sd = socket(AF_INET, SOCK_STREAM, 0);
  servAdd.sin_family = AF_INET;
  servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
  sscanf(argv[1], "%d", & portNumber);
  servAdd.sin_port = htons((uint16_t) portNumber);

  bind(sd, (struct sockaddr * ) & servAdd, sizeof(servAdd));
  listen(sd, 2);

  while (1) {
    printf("waiting for player 1\n");
    player1 = accept(sd, NULL, NULL);
    printf("Got client 1\n");

    printf("waiting for player 2\n");
    player2 = accept(sd, NULL, NULL);
    printf("Got client 2\n");
    
    if (!fork()) {
      servicePlayers(player1, player2);
    } else {
      wait(&status);
    }

    close(player1);
    close(player2);
   
    exit(0);
  }
}
