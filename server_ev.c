#include "fd.h"

void onconnection(fd_socket_t *);
void ondata(fd_socket_t *, char *);
void onend(fd_socket_t *);
void onchannel(fd_socket_t *, char *, int);

int main(int argc, char *argv[]){
	int port;

	/* read the port from the command line */
	if(argc != 2){
		printf("usage: server <port>\n");
		exit(1);
	}
	port = atoi(argv[1]);

	/* create one channel */
	create_channel("chatroom");

	/* start firedrake */
	fd_run(port, onconnection);
	
	return 0;
}

void onconnection(fd_socket_t *socket){
	printf("onconnection invoked on new socket with id %d\n", 
				 socket->tcp_sock);

	/* set the callbacks */
	socket->data_cb = ondata;
	socket->end_cb = onend;

	/* add the socket to the "chatroom" channel */
	fd_join_channel(socket, "chatroom", onchannel);
}

void ondata(fd_socket_t *socket, char *buffer){
	int length = socket->data == NULL ? 0 : strlen((char *) socket->data);

	printf("ondata invoked on socket with id %d\n", 
				 socket->tcp_sock);

	/* add the received data to our growing buffer */
	socket->data = realloc(socket->data, 
												 length + strlen(buffer) + 1);
	fd_strcat((char *) socket->data, buffer, length);
}

void onend(fd_socket_t *socket){
	/* broadcast your message to the channel */
	fd_broadcast(socket, "chatroom", (char *) socket->data, TEXT);

	printf("onend invoked on socket with id %d\n",
				 socket->tcp_sock);

	/* echo server */
	/* fd_send(socket, (char *) socket->data, TEXT); */
	
	socket->data = NULL;
}

void onchannel(fd_socket_t *socket, char *buffer, int msg_type){
	fd_send(socket, buffer, msg_type);
}
