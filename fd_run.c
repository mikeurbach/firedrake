#define EV_STANDALONE 1
#include "ev.c"
#include "fd.h"

int fd_run (int port, void(*callback)(fd_socket_t *socket)){
  int listenfd, optval, flags;
  fd_socket_t *server = malloc(sizeof(fd_socket_t));
  struct sockaddr_in servaddr;
  struct ev_loop *loop = EV_DEFAULT;
  ev_signal sigint_w;
	pthread_t *log_thread = malloc(sizeof(pthread_t));

	/* call socket to get a file descriptor */
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
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

	/* make the socket reuse the address */
	optval = 1;
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
								&optval, sizeof(optval)) < 0){
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

	/* attach the accept callback to the server socket struct */
	server->accept_cb = callback;

	/* initialize our accept watcher */
	server->tcp_sock = listenfd;
	ev_io_init(&server->read_w, accept_callback, listenfd, EV_READ);
	ev_io_start(loop, &server->read_w);

	/* check if the hashtable has been initialzed yet */
	if(hashtable == NULL)
		hashtable = init_channels(HASH_SIZE);

	/* setup logging */
	log_file = fopen(LOG_FILE,"a");								
	log_queue = qopen();
	pthread_create(log_thread, NULL, fd_log, NULL);
	
	/* Initialive watcher to catch SIGINT to close gracefully */
	ev_signal_init(&sigint_w, fd_close, SIGINT);
	ev_signal_start(loop, &sigint_w);

	fd_log_i("Event loop started, waiting for connections...\n");
	
	/* start the loop */
	ev_run(loop, 0);

	return 0;
}

/* close gracefully */
void fd_close(struct ev_loop *loop, ev_signal *w, int revents){

  /* close all channels */
  fd_log_m("Closing gracefully\n");

  /* close all open sockets */


  /* end the event loop */
  ev_break(loop, EVBREAK_ALL);
  
	/* close the log queue */
	qclose(log_queue);

  /* close the log file */
  fd_log_close;
}

void accept_callback(struct ev_loop *loop, ev_io *w, int revents){
	int connfd;
	fd_socket_t *client, *server = wtos(w, read_w);
  struct sockaddr_in cliaddr;
  socklen_t clilen = sizeof(cliaddr);
	
	/* accept a connection */
	if((connfd = accept(server->tcp_sock, (struct sockaddr *) &cliaddr, &clilen)) == -1){
		/* since we're nonblocking, these are ok */
		if(errno != EAGAIN && errno != EWOULDBLOCK){
			perror(__FILE__);
			exit(errno);
		}
		
		fd_log_w("callback invoked, but accept returned EAGAIN or EWOULDBLOCK, returning to ev_loop\n");

		return;
	}

	fd_log_m("connection %d accepted...\n", connfd);

	/* set up our client struct */
	client = malloc(sizeof(fd_socket_t));
	memset(client, 0, sizeof(fd_socket_t));
	client->tcp_sock = connfd;
	client->buffer = malloc(MAX_MESSAGE_LEN);
	memset(client->buffer, 0, MAX_MESSAGE_LEN);

	/* invoke the user's callback on the fresh socket, 
	   before the handshake has begun */
	server->accept_cb(client);

	/* start the handshake when the socket is ready */
	ev_io_init(&client->read_w, handshake_callback_r, connfd, EV_READ);
	ev_io_start(loop, &client->read_w);
}

void handshake_callback_r(struct ev_loop *loop, ev_io *w, int revents) {
	char *line, *key, *encoded, *headers, *response;
	const unsigned char *digest;
	int keylen, resplen;
	fd_socket_t *client = wtos(w, read_w);
	
	/* receive the initial HTTP request */
	if(recv(client->tcp_sock, client->buffer, MAX_MESSAGE_LEN, 0) < 0){
		/* since we're nonblocking, these are ok */
		if(errno != EAGAIN && errno != EWOULDBLOCK){
			perror(__FILE__);
			exit(errno);
		}
		
		fd_log_w("callback invoked, but recv returned EAGAIN or EWOULDBLOCK, returning to ev_loop\n");

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

	resplen = strlen(headers) + strlen(encoded) + strlen("\r\n\r\n") + 1;
	response = malloc(resplen);
	memset(response, 0, resplen);
	strcpy(response, headers);
	strcat(response, encoded);
	strcat(response, "\r\n\r\n");

	/* store the response in the client struct's buffer */
	strncpy(client->buffer, response, MAX_MESSAGE_LEN);

	free(response);
	free(key);

	/* finish the handshake when the socket is ready */
	ev_io_stop(loop, &client->read_w);
	ev_io_init(&client->write_w, handshake_callback_w, client->tcp_sock, EV_WRITE);
	ev_io_start(loop, &client->write_w);
}

void handshake_callback_w(struct ev_loop *loop, ev_io *w, int revents){
	fd_socket_t *client = wtos(w, write_w);

	if(send(client->tcp_sock, client->buffer, strlen(client->buffer), 0) == -1){
		/* since we're nonblocking, these are ok */
		if(errno != EAGAIN && errno != EWOULDBLOCK){
			perror(__FILE__);
			exit(errno);
		}

		fd_log_w("callback invoked, but send returned EAGAIN or EWOULDBLOCK, returning to ev_loop\n");

		return;
	}

	fd_log_m("handshake completed with connection %d...\n", client->tcp_sock);

	/* stop waiting for a handshake write, initialize echo read */
	ev_io_stop(loop, &client->write_w);

	client->just_opened = 1;
	client->buffer = realloc(client->buffer, MAX_HEADER_LEN);
	memset(client->buffer, 0, MAX_HEADER_LEN);
	client->recvs = 0;
	ev_io_init(&client->read_w, fd_recv_nb, client->tcp_sock, EV_READ);
	ev_io_start(loop, &client->read_w);
}
