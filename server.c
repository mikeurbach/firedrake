#include "fd.h"

int main (int argc, char **argv){
  int port, listenfd, connfd;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;
  char buffer[MAX_MESSAGE_LEN];

	/* read the port from the command line */
	if(argc != 2){
		printf("usage: server <port>\n");
		exit(1);
	}
	port = atoi(argv[1]);

	/* call socket to get a file descriptor */
  if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
    perror(__FILE__);
    exit(errno);
  }

  /* prepare the socket struct */
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

	/* bind the socket */
  bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

  /* listen for sockets, taking up to LISTENQ connections */
  listen(listenfd, LISTENQ);

  printf("Server waiting for connections...\n");

  /* serve ad infinitum */
  while(1) {
    /* accept a connection */
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

    printf("Connection %d accepted...\n", connfd);

    /* complete the handshake */
    if(handshake(connfd)){
      printf("Handshake failed with connection %d...\n", connfd);
    }

    printf("Handshake completed with connection %d...\n", connfd);

    fd_socket_t *sock = malloc(sizeof(fd_socket_t));
    sock->tcp_sock = connfd;
    sock->last_recv_opcode = OPEN;
    sock->is_open = true;

    for(;;){
			memset(buffer, 0, MAX_MESSAGE_LEN);
    	fd_recv(sock, buffer);
			printf("Echoing %s\n", buffer);
			fd_send(sock, buffer, TEXT);
		}
  }
}

int handshake(int connfd) {
	char recvbuff[MAX_MESSAGE_LEN];
	char *line, *key, *encoded, *headers, *response;
	const unsigned char *digest;
	int recvbytes, respbytes, keylen, resplen;
	
	/* receive the initial HTTP request */
	recvbytes = recv(connfd, recvbuff, MAX_MESSAGE_LEN, 0);
	if(recvbytes < 0){
		perror(__FILE__);
		return 1;
	}

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

	/* compute an SHA-1 hash of the key */
	digest = SHA1((const unsigned char *) key, strlen(key), NULL);

	/* base64 encode the digest */
	encoded = b64_encode((char *) digest, strlen((char *) digest));

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

	/* send back the handshake */
	respbytes = send(connfd, response, resplen, 0);
	if(respbytes == -1){
		perror(__FILE__);
		return 1;
	}

	return 0;
}
