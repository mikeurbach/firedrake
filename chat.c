#include "fd.h"

void ondata(fd_socket_t *socket, char *message){
  fd_broadcast(socket, "chat", message, TEXT);
}

void onchannel(fd_socket_t *socket, char *message, int msg_type){
  fd_send(socket, message, msg_type);
}

void onconnection(fd_socket_t *socket){
  fd_join_channel(socket, "chat", onchannel);
  socket->data_cb = ondata;
}

void main(void){
  create_channel("chat");

  fd_run(8080, onconnection);
}
