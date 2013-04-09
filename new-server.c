#include "server.h"

//#define DEBUG0

static void get_connections(int);
void* setup_client(void *args);
void handshake(int);

Client  *clients;
int   num_clients, port;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main (int argc, char **argv){
	if(argc != 2){
		printf("usage: server <port>\n");
		exit(1);
	}

	port = atoi(argv[1]);

  //set the initial state
  NEW_N(clients, Client, LISTENQ);
  num_clients = 0;
  for(int i = 0; i < LISTENQ; i++){
    clients[i].id = 0;
  }

  get_connections(port);

  free(clients);
}

//runs the main loop that gets connections from clients and spawns new threads
static void get_connections(int port){
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
  servaddr.sin_port = htons(port);

  //bind the socket
  bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

  //listen to the socket by creating a connection queue, then wait for clients
  listen (listenfd, LISTENQ);

  printf("%s\n","Server running...waiting for connections.");

  for ( ; ; ) {

    clilen = sizeof(cliaddr);
    //accept a connection
    connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

    printf("%s\n","Connection accepted...");

    //if it's 0, the thread was created properly
    if(num_clients < LISTENQ){
      handshake(connfd);
    }
  }
}

void handshake(int connfd) {
	char recvbuff[MAXLINE];
	char *line, *key;
	int recvbytes;
	
	/* receive the initial HTTP request */
	recvbytes = recv(connfd, recvbuff, MAXLINE, 0);
	if(recvbytes < 0){
		perror(__FILE__);
		exit(errno);
	}

	printf("HTTP request received, initiating handshake...\n");

	printf("%s\n", recvbuff);

	/* parse out the Sec-Websocket-Key header */
	line = strtok(recvbuff, "\n");
	while(line != NULL &&
				strncmp(line, HEADERKEY, (int) strlen(HEADERKEY)) != 0) {
		line = strtok(NULL, "\n");
	}
	
	/* get just the key out of the header */
	for( ; *line != ' '; line++);
	line++;

	key = malloc(strlen(line) + strlen(MAGICSTRING) + 1024);
	memset(key, 0, sizeof(key));
	printf("line: %s\n", line);
	strcpy(key, line);
	printf("key after strcpy: %s\n", key);
	key = strcat(key, MAGICSTRING);
	printf("key after strcat: %s\n", key);
}
