#include "fd.h"

void onconnection(fd_socket_t *);
void ondata(fd_socket_t *, char *);

int main(int argc, char *argv[]){
	int port;

	/* read the port from the command line */
	if(argc != 2){
		printf("usage: server <port>\n");
		exit(1);
	}
	port = atoi(argv[1]);

	/* start firedrake */
	fd_run(port, onconnection);
	
	return 0;
}

void onconnection(fd_socket_t *socket){
	printf("onconnection invoked on new socket with id %d\n", 
				 socket->tcp_sock);

	/* set the data callback */
	socket->data_cb = ondata;
}

void ondata(fd_socket_t *socket, char *buffer){
	printf("ondata invoked on socket with id %d\n", 
				 socket->tcp_sock);


	printf("server received: %s\n", buffer);

	/* echo server */
	fd_send(socket, buffer, TEXT);
}
