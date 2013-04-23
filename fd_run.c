#define EV_STANDALONE 1
#include "ev.c"
#include "fd.h"

int fd_run (int port){
  int listenfd, flags;
	fd_socket_t *server = malloc(sizeof(fd_socket_t));
  struct sockaddr_in servaddr;
	struct ev_loop *loop = EV_DEFAULT;

	/* call socket to get a file descriptor */
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) <0){
    perror(__FILE__);
    exit(errno);
  }

	/* call fcntl to set it non blocking */
	flags = fcntl(listenfd, F_GETFL);
	flags |= O_NONBLOCK;
	if(fcntl(listenfd, F_SETFL, flags)){
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

	/* initialize our accept watcher */
	server->tcp_sock = listenfd;
	ev_io_init(&server->io, accept_callback, listenfd, EV_READ);
	ev_io_start(loop, &server->io);

	/* start the loop */
	ev_run(loop, 0);

  printf("Event loop started, waiting for connections...\n");

	return 0;
}

void accept_callback(struct ev_loop *loop, ev_io *w, int revents){
	int connfd;
	fd_socket_t *client, *server = (fd_socket_t *) w;
  struct sockaddr_in cliaddr;
  socklen_t clilen = sizeof(cliaddr);
	
	/* accept a connection */
	if((connfd = accept(server->tcp_sock, (struct sockaddr *) &cliaddr, &clilen)) == -1){
		/* since we're nonblocking, these are ok */
		if(errno != EAGAIN && errno != EWOULDBLOCK){
			perror(__FILE__);
			exit(errno);
		}
		printf("accept_callback invoked, but accept returned EAGAIN or EWOULDBLOCK\n");
		return;
	}

	printf("Connection %d accepted...\n", connfd);

	/* set up our client struct */
	client = malloc(sizeof(fd_socket_t));
	client->tcp_sock = connfd;

	/* start the handshake when the socket is ready */
	ev_io_init(&client->io, handshake_callback_r, connfd, EV_READ);
	ev_io_start(loop, &client->io);
}

void handshake_callback_r(struct ev_loop *loop, ev_io *w, int revents) {
	char *line, *key, *encoded, *headers, *response;
	const unsigned char *digest;
	int keylen, resplen;
	fd_socket_t *client = (fd_socket_t *) w;
	
	/* receive the initial HTTP request */
	if(recv(client->tcp_sock, client->buffer, MAX_MESSAGE_LEN, 0) < 0){
		/* since we're nonblocking, these are ok */
		if(errno != EAGAIN && errno != EWOULDBLOCK){
			perror(__FILE__);
			exit(errno);
		}
		printf("handshake_callback_r invoked, but recv returned EAGAIN or EWOULDBLOCK\n");
		return;
	}

	/* parse out the Sec-Websocket-Key header */
	line = strtok(client->buffer, "\r\n");
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

	/* store the response in the client struct's buffer */
	strncpy(client->buffer, response, MAX_MESSAGE_LEN);

	/* finish the handshake when the socket is ready */
	ev_io_stop(loop, &client->io);
	ev_io_init(&client->io, handshake_callback_w, client->tcp_sock, EV_WRITE);
	ev_io_start(loop, &client->io);
}

void handshake_callback_w(struct ev_loop *loop, ev_io *w, int revents){
	fd_socket_t *client = (fd_socket_t *) w;

	if(send(client->tcp_sock, client->buffer, strlen(client->buffer), 0) == -1){
		/* since we're nonblocking, these are ok */
		if(errno != EAGAIN && errno != EWOULDBLOCK){
			perror(__FILE__);
			exit(errno);
		}
		printf("handshake_callback_w invoked, but send returned EAGAIN or EWOULDBLOCK\n");
		return;
	}

	printf("Handshake completed with connection %d...\n", client->tcp_sock);

	/* stop waiting for a handshake read, initialize echo read */
	ev_io_stop(loop, &client->io);
	ev_io_init(&client->io, client_callback_r, client->tcp_sock, EV_READ);
	ev_io_start(loop, &client->io);
}

void client_callback_r(struct ev_loop *loop, ev_io *w, int revents){
	fd_socket_t *client = (fd_socket_t *) w;

	/* receive from the socket */
	memset(client->buffer, 0, MAX_MESSAGE_LEN);
	fd_recv(client, client->buffer);
	
	printf("Client %d callback received: %s\n", client->tcp_sock, client->buffer);

	/* stop waiting for an echo read, initialize echo write */
	ev_io_stop(loop, &client->io);
	ev_io_init(&client->io, client_callback_w, client->tcp_sock, EV_WRITE);
	ev_io_start(loop, &client->io);
}

void client_callback_w(struct ev_loop *loop, ev_io *w, int revents){
	fd_socket_t *client = (fd_socket_t *) w;

	fd_send(client, client->buffer, TEXT);

	printf("Client %d callback sent: %s\n", client->tcp_sock, client->buffer);

	/* stop waiting for an echo read, initialize echo write */
	ev_io_stop(loop, &client->io);
	ev_io_init(&client->io, client_callback_r, client->tcp_sock, EV_READ);
	ev_io_start(loop, &client->io);

}