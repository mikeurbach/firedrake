#include "fd.h"

int recv(fd_socket_t *sock, char *buff){
	int	status,
		len;

	char	header[HEADER_SIZE];
	
status = recv(sock->tcp_sock, void (header), HEADER_SIZE, 0);

len = (header & 0xFF00) >> 8

//do some stuff with the header to figure out how long the payload is, and receive that //amount of data

	
status = recv(sock->tcp_sock, void (buff), msg_len, 0);

//

return (status);
}