//Dat Lam 	   105190512
//Riva Vaishnani   110009148

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

#define GAMEPOINT 100

void action() {}; //handler function for SIGUSR1 signal.
void player(char * , int, int * , int);

void servicePlayers(int player1Fd, int player2Fd) {
  pid_t pid1, pid2;
  char * playerNames[2] = {
    "TOTO",
    "TITI"
  };

  int sharedSegment;
  int * sharedArray;

  //get a shared segment (check for fail)
  sharedSegment = shmget(IPC_PRIVATE, 2 * sizeof(double), IPC_CREAT | 0600);
  //get your shared array  
  sharedArray = shmat(sharedSegment, NULL, 0);

  //fork 2 children, each of them for each one of players
  if ((pid1 = fork()) == 0)
    player(playerNames[0], 0, sharedArray, player1Fd);

  if ((pid2 = fork()) == 0)
    player(playerNames[1], 1, sharedArray, player2Fd);

  sleep(1);
  signal(SIGUSR1, action); //install handler for the signal, handler function does nothing. SIGUSSR1 used to trigger child and parent processes

  while (1) {
    printf("\nReferee: TOTO plays\n");
    kill(pid1, SIGUSR1); //send SIGUSR1 to pid1 to trigger it
    pause(); //suspend and wait for the signal SIGUSR1 from its child process

    printf("\nReferee: TITI plays\n");
    kill(pid2, SIGUSR1); //send SIGUSR1 to pid2 to trigger it
    pause(); //suspend and wait for the signal SIGUSR1 from its child process

    printf("\nReferee: Total points so far:\n TOTO: %d\n TITI: %d\n", sharedArray[0], sharedArray[1]);

    if (sharedArray[0] >= GAMEPOINT || sharedArray[1] >= GAMEPOINT) {
      if (sharedArray[0] >= GAMEPOINT) {
        printf("Game over: TOTO won the game\n");
        write(player1Fd, "Game over: you won the game", 255);
        write(player2Fd, "Game over: you lost the game", 255);
      } else {
        printf("Game over: TITI won the game\n");
        write(player2Fd, "Game over: you won the game", 255);
        write(player1Fd, "Game over: you lost the game", 255);
      }

      kill(pid1, SIGTERM); //kill the child process pid1 to release the socket port
      kill(pid2, SIGTERM); //kill the child process pid2 to release the socket port
      exit(20);
    }
  }
}

void player(char * name, int playerId, int * sharedArray, int sd) {
  int points = 0, dice, n;
  long int ss = 0;
  pid_t pid;
  char message[255];

  while (1) {
    signal(SIGUSR1, action); 
    pause(); //block myself

    //send a message to its client
    write(sd, "You can now play", 255);

    //wait for dice coming from clients
    if (n = read(sd, & dice, sizeof(dice))) {
      printf("%d\n", dice);
    }

    //calculate points and add it into shared array
    points += dice;
    sharedArray[playerId] = points;

    sleep(3);
    kill(getppid(), SIGUSR1); //send ISGUSR1 to its parent to wake up the parent
  }
}

int main(int argc, char const * argv[]) {

  int sd, player1Fd, player2Fd, portNumber, status;
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
    player1Fd = accept(sd, NULL, NULL);
    printf("Got client 1\n");

    printf("waiting for player 2\n");
    player2Fd = accept(sd, NULL, NULL);
    printf("Got client 2\n");

    if (!fork()) {
      servicePlayers(player1Fd, player2Fd);
    } else {
      wait( & status);
    }

    close(player1Fd);
    close(player2Fd);

    exit(0);
  }
}
