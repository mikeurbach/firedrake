#ifndef _FD_H
#define _FD_H


#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct _fd_socket_t{
	int	tcp_sock;


}fd_socket_t;


int fd_recv(fd_socket_t *sock, char *buff);

#endif
