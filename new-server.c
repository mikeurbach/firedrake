#include "server.h"

//#define DEBUG0

static void get_connections();
void* setup_client(void *args);

Client  *clients;
int   num_clients;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main (int argc, char **argv){

  //set the initial state
  NEW_N(clients, Client, LISTENQ);
  num_clients = 0;
  for(int i = 0; i < LISTENQ; i++){
    clients[i].id = 0;
  }

  get_connections();

  free(clients);
}

//runs the main loop that gets connections from clients and spawns new threads
static void get_connections(){
  int listenfd, connfd;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;

  //Create a socket
  //If sockfd<0 there was an error in the creation of the socket
  if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
    perror("Problem in creating the socket");
    exit(2);
  }


  //preparation of the socket address
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(SERV_PORT);

  //bind the socket
  bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

  //listen to the socket by creating a connection queue, then wait for clients
  listen (listenfd, LISTENQ);

  printf("%s\n","Server running...waiting for connections.");

  for ( ; ; ) {

    clilen = sizeof(cliaddr);
    //accept a connection
    connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

    printf("%s\n","Received request...");

    //if itÕs 0, the thread was created properly
    //make sure num_clients doesn't change during this!'
    if(num_clients < LISTENQ){
      setup_client((void *) &connfd);
    }
  }
}

void* setup_client(void *args){
  int n, connfd;
  char recvline[MAXLINE];

  printf ("%s\n","Thread created for dealing with client requests");

  connfd = *( (int *) args);

  n = recv(connfd, recvline, MAXLINE,0);

  if (n < 0){
    printf("%s\n", "Read error");
  }

  printf("String received from the client: %s.\n", recvline);

  return(NULL);
}
