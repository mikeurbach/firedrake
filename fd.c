#include "fd.h"

#define PAYLOAD_EXT_16 126
#define PAYLOAD_EXT_64 127

int recv(fd_socket_t *sock, char *buff){
	
	int					status = 0,
						len = 0,
						fin = 0,
						rsv1 = 0,
						rsv2 = 0,
						rsv3 = 0,
						is_masked = 0;

	int8_t				payload_len = 0;

	uint16_t			ext_payload_len1 = 0;

	uint32_t			mask_key = 0;

	uint64_t			ext_payload_len2 = 0;

	unsigned int 		opcode = 0;

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
status = recv(sock->tcp_sock, void (&fin), 1, 0);

//Parse RSV1, RSV2, and RSV3 (1 bit each) (Must be 0 unless 
//an extension is negotiated that defines meanings for non-zero values)
status = recv(sock->tcp_sock, void (&rsv1), 1, 0);
status = recv(sock->tcp_sock, void (&rsv2), 1, 0);
status = recv(sock->tcp_sock, void (&rsv3), 1, 0);

//Parse opcode (4 bits) (react according to RFC 6455)
status = recv(sock->tcp_sock, void (&opcode), 4, 0);

//Parse mask (1 bit) (defines whether the "Payload data" is masked)
status = recv(sock->tcp_sock, void (&is_masked), 1, 0);

//Parse Payload length (7 bits, 7 + 16 bits, or 7 + 64 bits) (in network byte order)
//If 0-125 that is the payload length
//If 126, the following 2 bytes are interpreted as a 16 bit unsigned integer
//If 127, the following 8 bytes interpreted as a 64-bit unsigned integer with MSB = 0
status = recv(sock->tcp_sock, void (&payload_len), 7, 0);

if(payload_len < PAYLOAD_EXT_16){

}
else if(payload_len == PAYLOAD_EXT_16){
	status = recv(sock->tcp_sock, void (&ext_payload_len1), 16, 0);
}
else if(payload_len == PAYLOAD_EXT_64){
	status = recv(sock->tcp_sock, void (&ext_payload_len2), 64, 0);
}

//Parse Masking key (0 or 4 bytes) All frames from client to server are masked by this value
//(absent if the mask is 0)

if(is_masked){
	status = recv(sock->tcp_sock, void (&mask_key), 32, 0);
	//unmask the payload data
}

//Place Payload data in buffer and return status

	return (status);
}