#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>

int playDice() {
  long int ss = 0;
  int dice = (int) time( & ss) % 10 + 1;
  printf("%d\n", dice);
  return dice;
}

int main(int argc, char * argv[]) {
  char message[255];
  int server, portNumber;
  struct sockaddr_in servAdd; // server socket address
  int n, dice;
  pid_t pid;

  if (argc != 3) {
    printf("Call syntax: %s <IP Address> <Port Number>\n", argv[0]);
    exit(0);
  }

  if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0) { //create socket
    fprintf(stderr, "Cannot create socket\n");
    exit(1);
  }

  servAdd.sin_family = AF_INET;
  sscanf(argv[2], "%d", & portNumber);
  servAdd.sin_port = htons((uint16_t) portNumber);

  if (inet_pton(AF_INET, argv[1], & servAdd.sin_addr) < 0) { //connect with port
    fprintf(stderr, " inet_pton() has failed\n");
    exit(2);
  }

  if (connect(server, (struct sockaddr * ) & servAdd, sizeof(servAdd)) < 0) { //connect server with Ip
    fprintf(stderr, "connect() has failed, exiting\n");
    exit(3);
  }

  while (1) {
  if (read(server, message, 255) < 0) {
    fprintf(stderr, "read() error\n");
    exit(3);
  }

  if (!strcmp(message, "Game over: you won the game") || !strcmp(message, "Game over: you lost the game")) {
	if (!strcmp(message, "Game over: you won the game"))
		printf("I won the game\n");
	else printf("I lost the game\n");
	//close socket and exit
	close(server);
	exit(0);
  } else {
        fprintf(stderr, "%s\n", message);
	dice = playDice();
   	write(server, &dice, sizeof(dice));
  } 
   
  }
}
