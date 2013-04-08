/* 
   server.c
   by mike urbach
   a TCP server for lab3
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#define BUFFLEN 1024
#define MAX_CONN 10

int main(int argc, char *argv[]) {
  struct sockaddr_in sockaddr_server, sockaddr_client;
  socklen_t client_len = sizeof(sockaddr_client);
  int recvnum, sockd, fd, count = 0;
  char buff[BUFFLEN];

  /* read command line args */
  if(argc != 2) {
    printf("%s: usage: server <port>\n", __FILE__);
    exit(1);
  }
  
  /* open up a socket */
  if((sockd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror(__FILE__);
    exit(errno);
  }

  /* fcntl(sockd, F_SETFL, O_NONBLOCK); */
  
  /* initialize our socket address struct and bind to the port */
  memset(&sockaddr_server, 0, sizeof(sockaddr_server));
  sockaddr_server.sin_family = AF_INET;
  sockaddr_server.sin_port = atoi(argv[1]);
  sockaddr_server.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(sockd, (struct sockaddr *) &sockaddr_server, sizeof(sockaddr_server)) == -1) {
    perror(__FILE__);
    exit(errno);
  }

  /* listen for connections */
  if(listen(sockd, MAX_CONN) == -1) {
    perror(__FILE__);
    exit(errno);
  }


  while(1) {
    /* accept for connection */
    if((fd = accept(sockd, (struct sockaddr *) &sockaddr_client, &client_len)) == -1) {
      perror(__FILE__);
      exit(errno);
    }

    printf("Accepted client\n");

    /* talk to the client */
    while(recv(fd, buff, BUFFLEN, 0) > 0) {
      printf("%s\n", buff);
  
      /* send a packet */
      if(send(fd, buff, BUFFLEN, 0) == -1) {
	perror(__FILE__);
	exit(3);
      }
    }
  }

  close(sockd);

  return 0;
}
