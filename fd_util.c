
#include "fd.h"

fd_socket_t *fd_socket_new(void){
	fd_socket_t *new_sock = (fd_socket_t *) malloc(sizeof(fd_socket_t));
	
	//set defaults
	new_sock->tcp_sock = -1;
	new_sock->last_recv_opcode = -1;
	new_sock->is_open = false;

	return (new_sock);
}

void fd_socket_destroy(fd_socket_t *sock, struct ev_loop *loop){
	ev_io_stop(loop, &sock->read_w);
	fd_socket_close(sock);
	free(sock);

}

int fd_socket_close(fd_socket_t *sock){
  
  fd_log_i("closing socket with file descriptor: %d\n",sock->tcp_sock);
  
  remove_from_channel("chatroom", sock->tcp_sock);
  sock->is_open = false;
  return ( close(sock->tcp_sock) );

}

