#include "server.h"

//#define DEBUG0

static void get_connections(int);
void* setup_client(void *args);
void handshake(int);
void echo(int);

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

		/* now just be an echo server */
		echo(connfd);
  }
}

void handshake(int connfd) {
	char recvbuff[MAXLINE];
	char *line, *key, *encoded, *headers, *response;
	const unsigned char *digest;
	int recvbytes, respbytes, keylen, resplen;
	
	/* receive the initial HTTP request */
	recvbytes = recv(connfd, recvbuff, MAXLINE, 0);
	if(recvbytes < 0){
		perror(__FILE__);
		exit(errno);
	}

	printf("HTTP request received: %s\n", recvbuff);

	/* parse out the Sec-Websocket-Key header */
	line = strtok(recvbuff, "\r\n");
	while(line != NULL &&
				strncmp(line, HEADERKEY, (int) strlen(HEADERKEY)) != 0) {
		line = strtok(NULL, "\r\n");
	}
	
	/* get just the key out of the header */
	for( ; *line != ' '; line++);
	line++;

	/* concatenate the key and the magic string */
	keylen = strlen(line) + strlen(MAGICSTRING);
	key = malloc(keylen);
	memset(key, 0, keylen);
	strncpy(key, line, strlen(line));
	strncat(key, MAGICSTRING, strlen(MAGICSTRING));
	printf("Key before SHA-1: %s\n", key);

	/* compute an SHA-1 hash of the key */
	digest = SHA1((const unsigned char *) key, strlen(key), NULL);
	printf("SHA-1 digest: %s\n", digest);

	/* base64 encode the digest */
	encoded = b64_encode((char *) digest, strlen((char *) digest));
	printf("Base64 encoded: %s\n", encoded);

	/* assemble the response */
	headers = 
		"HTTP/1.1 101 Switching Protocols\r\n"			\
		"Upgrade: websocket\r\n"										\
		"Connection: Upgrade\r\n"										\
		"Sec-WebSocket-Accept: ";

	resplen = strlen(headers) + strlen(encoded) + strlen("\r\n\r\n");
	response = malloc(resplen);
	memset(response, 0, resplen);
	strcpy(response, headers);
	strcat(response, encoded);
	strcat(response, "\r\n\r\n");
	printf("Response assembled:\n%s\n", response);

	/* send back the handshake */
	respbytes = send(connfd, response, resplen, 0);
	if(respbytes == -1){
		perror(__FILE__);
		exit(errno);
	}

	printf("Response sent\n");
}

void echo(int connfd){
	char recvbuff[MAXLINE];
	int n;

	while(1){
		n = recv(connfd, recvbuff, MAXLINE, 0);
		if(n == -1){
			perror(__FILE__);
			exit(errno);
		}
		send(connfd, recvbuff, MAXLINE, 0);
	}
}
