#ifndef _FD_H
#define _FD_H


#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

typedef struct _fd_socket_t{
	int	sock_fd;
	

}fd_socket_t;


int recv(fd_socket_t *sock, char *buff);