#include "fd.h"

#define WELCOME_MSG "Welcome to the echo chatroom!"

void ondata(fd_socket_t *socket, char *message){
	fd_send(socket, message, TEXT);
}

void onconnection(fd_socket_t *socket){
	socket->data_cb = ondata;
}

int main(int argc, char **argv){
	fd_run(8080, onconnection);
	return(EXIT_SUCCESS);
}
