#include "fd.h"

void onconnection(fd_socket_t *);
void ondata(fd_socket_t *, char *);
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

	/* set the data callback */
	socket->data_cb = ondata;

	/* add the socket to the "chatroom" channel */
	fd_join_channel(socket, "chatroom", onchannel);
}

void ondata(fd_socket_t *socket, char *buffer){
	printf("ondata invoked on socket with id %d\n", 
				 socket->tcp_sock);

	/* broadcast your message to the channel */
	//	fd_broadcast(socket, "chatroom", buffer, TEXT);

	/* echo server */
	fd_send(socket,buffer,TEXT);
}

void onchannel(fd_socket_t *socket, char *buffer, int msg_type){
	fd_send(socket, buffer, msg_type);
}
