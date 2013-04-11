#include "fd.h"

#define PAYLOAD_EXT_16 126
#define PAYLOAD_EXT_64 127

static void unmask_payload(char* data, uint64_t len, uint32_t key);

int fd_recv(fd_socket_t *sock, char *buff){
	
	int					status = 0,
						fin = 0,
						rsv1 = 0,
						rsv2 = 0,
						rsv3 = 0,
						is_masked = 0;

	uint8_t				payload_len = 0,
						temp = 0;

	uint16_t			ext_payload_len1 = 0;

	uint32_t			mask_key = 0;

	uint64_t			ext_payload_len2 = 0,
						final_payload_len = 0;

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
//Receive first byte of header
	status = recv(sock->tcp_sock, (&temp), 4, 0);

//Parse FIN (1 bit) (is this the last fragment in the message?)
//Parse RSV1, RSV2, and RSV3 (1 bit each) (Must be 0 unless 
//an extension is negotiated that defines meanings for non-zero values)
//Parse opcode (4 bits) (react according to RFC 6455)
	fin = (temp & 0x80) >> 7;
	rsv1 = (temp & 0x40) >> 6;
	rsv2 = (temp & 0x20) >> 5;
	rsv3 = (temp & 0x10) >> 4;

	opcode = (temp & 0xF);

//Parse mask (1 bit) (defines whether the "Payload data" is masked)


//Parse Payload length (7 bits, 7 + 16 bits, or 7 + 64 bits) (in network byte order)
//If 0-125 that is the payload length
//If 126, the following 2 bytes are interpreted as a 16 bit unsigned integer
//If 127, the following 8 bytes interpreted as a 64-bit unsigned integer with MSB = 0
	status = recv(sock->tcp_sock, (&payload_len), 1, 0);
	is_masked = (payload_len & 0x80) >> 7;
	payload_len = (payload_len & 0x7F);

	if(payload_len < PAYLOAD_EXT_16){
		final_payload_len = payload_len;
	}
	else if(payload_len == PAYLOAD_EXT_16){
		status = recv(sock->tcp_sock, (&ext_payload_len1), 2, 0);
		final_payload_len = ext_payload_len1;
	}
	else if(payload_len == PAYLOAD_EXT_64){
		status = recv(sock->tcp_sock, (&ext_payload_len2), 8, 0);
		final_payload_len = ext_payload_len2;
	}

//Parse Masking key (0 or 4 bytes) All frames from client to server are masked by this value
//(absent if the mask is 0)

	if(is_masked){
		status = recv(sock->tcp_sock, (&mask_key), 4, 0);
		//unmask the payload data
	}

//Place Payload data in buffer and return status
 	status = recv(sock->tcp_sock, buff, final_payload_len, 0);

 	if(is_masked){
 		unmask_payload(buff, final_payload_len, mask_key);
	}

	return (status);
}

static void unmask_payload(char* data, uint64_t len, uint32_t key){

	uint8_t	octet;

	for(int i = 0; i < len; i++){
		switch (i % 4){
			case 1: octet = (key & 0xFF);
					break;

			case 2: octet = (key & 0xFF00);
					break;

			case 3: octet = (key & 0xFF0000);
					break;

			case 4: octet = (key & 0xFF000000);
					break;

		}

		data[i] = data[i] ^ octet;

	}

}

int main (int argc, char **argv){

	//a single frame masked text message that contains "hello"
	//0x81 0x85 0x37 0xfa 0x21 0x3d 0x7f 0x9f 0x4d 0x51 0x58

	return(EXIT_SUCCESS);
}
