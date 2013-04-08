#include "fd.h"

int recv(fd_socket_t *sock, char *buff){
	int	status,
		len;

	char	header[HEADER_SIZE];
	
status = recv(sock->tcp_sock, void (header), HEADER_SIZE, 0);

len = (header & 0xFF00) >> 8

//do some stuff with the header to figure out how long the payload is, and receive that
//amount of data
/*
    0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+

*/

//Parse FIN (1 bit) (is this the last fragment in the message?)

//Parse RSV1, RSV2, and RSV3 (1 bit each) (Must be 0 unless 
//an extension is negotiated that defines meanings for non-zero values)

//Parse opcode (4 bits) (react according to RFC 6455)

//Parse mask (1 bit) (defines whether the "Payload data" is masked)

//Parse Payload length (7 bits, 7 + 16 bits, or 7 + 64 bits) (in network byte order)
//If 0-125 that is the payload length
//If 126, the following 2 bytes are interpreted as a 16 bit unsigned integer
//If 127, the following 8 bytes interpreted as a 64-bit unsigned integer with MSB = 0

//Parse Masking key (0 or 4 bytes) All frames from client to server are masked by this value
//(absent if the mask is 0)

//Place Payload data in buffer and return status
	
status = recv(sock->tcp_sock, void (buff), msg_len, 0);

//

return (status);
}