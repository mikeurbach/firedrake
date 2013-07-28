#include "fd.h"

void ondata(fd_socket_t *socket, char *message){
	fd_send(socket, message, TEXT);
}

void onconnection(fd_socket_t *socket){
	socket->data_cb = ondata;
}

void main(void){
	fd_run(8080, onconnection);
}
