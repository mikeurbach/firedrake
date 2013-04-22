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
	status = recv(sock->tcp_sock, (&temp), 1, 0);

//Parse FIN (1 bit) (is this the last fragment in the message?)
//Parse RSV1, RSV2, and RSV3 (1 bit each) (Must be 0 unless 
//an extension is negotiated that defines meanings for non-zero values)
//Parse opcode (4 bits) (react according to RFC 6455)
	fin = (temp & 0x80) >> 7;
	//printf("fd_recv: fin is %d\n", fin);

	rsv1 = (temp & 0x40) >> 6;
	//printf("fd_recv: rsv1 is %d\n", rsv1);

	rsv2 = (temp & 0x20) >> 5;
	//printf("fd_recv: rsv2 is %d\n", rsv2);

	rsv3 = (temp & 0x10) >> 4;
	//printf("fd_recv: rsv3 is %d\n", rsv3);

	opcode = (temp & 0xF);
	//printf("fd_recv: opcode is %X\n", opcode);

//Parse mask (1 bit) (defines whether the "Payload data" is masked)


//Parse Payload length (7 bits, 7 + 16 bits, or 7 + 64 bits) (in network byte order)
//If 0-125 that is the payload length
//If 126, the following 2 bytes are interpreted as a 16 bit unsigned integer
//If 127, the following 8 bytes interpreted as a 64-bit unsigned integer with MSB = 0
	status = recv(sock->tcp_sock, (&payload_len), 1, 0);
	is_masked = (payload_len & 0x80) >> 7;
	//printf("fd_recv: is_masked is %d\n", is_masked);

	payload_len = (payload_len & 0x7F);
	//printf("fd_recv: payload_len is %d\n", payload_len);

	if(payload_len < PAYLOAD_EXT_16){
		final_payload_len = payload_len;
	}
	else if(payload_len == PAYLOAD_EXT_16){
		status = recv(sock->tcp_sock, (&ext_payload_len1), 2, 0);

		// byte are recevied in wrong order, so need to reverse them                                                                    
		ext_payload_len1 = (0xFF00 & (ext_payload_len1 << 8)) | (0x00FF & (ext_payload_len1 >> 8));
		final_payload_len = ext_payload_len1;
	}
	else if(payload_len == PAYLOAD_EXT_64){
		status = recv(sock->tcp_sock, (&ext_payload_len2), 8, 0);

		ext_payload_len2 = (ext_payload_len2 & 0x00000000000000FFUL) << 56 | (ext_payload_len2 & 0x000000000000FF00UL) << 40 |
		(ext_payload_len2 & 0x0000000000FF0000UL) << 24 | (ext_payload_len2 & 0x00000000FF000000UL) << 8 |
		(ext_payload_len2 & 0x000000FF00000000UL) >> 8 | (ext_payload_len2 & 0x0000FF0000000000UL) >> 24 |
		(ext_payload_len2 & 0x00FF000000000000UL) >> 40 | (ext_payload_len2 & 0xFF00000000000000UL) >> 56;
		final_payload_len = ext_payload_len2;
   }

	printf("fd_recv: final_payload_len is %d\n", final_payload_len);


//Parse Masking key (0 or 4 bytes) All frames from client to server are masked by this value
//(absent if the mask is 0)

	if(is_masked){
		status = recv(sock->tcp_sock, (&mask_key), 4, 0);
		//printf("fd_recv: mask is %X\n", mask_key);
	}


//Place Payload data in buffer and return status
 	status = recv(sock->tcp_sock, buff, final_payload_len, 0);

 	//printf("fd_recv: masked data is \"%s\"\n", buff);


 	if(is_masked){
 		unmask_payload(buff, final_payload_len, mask_key);
	}

	printf("fd_recv: unmasked data is \"%s\"\n", buff);

	sock->last_recv_opcode = opcode;

	if(opcode == PING){
		//send a pong
		status = fd_send(sock, buff, PONG);
	}
	else if(opcode == PONG){
		//check to make sure APPLICATION data is the same as was in the PING (how?)
	}
	else if(opcode == CONNECTION_CLOSE){
		//send a matching CLOSE message and close the socket gracefully (fd_close function?)
		status = fd_send(sock, buff, CONNECTION_CLOSE);

	}

	return (status);
}

static void unmask_payload(char* data, uint64_t len, uint32_t key){

	uint8_t	octet;

	for(int i = 0; i < len; i++){
		switch (i % 4){
			case 0: octet = (key & 0xFF);
					break;

			case 1: octet = (key & 0xFF00) >> 8;
					break;

			case 2: octet = (key & 0xFF0000) >> 16;
					break;

			case 3: octet = (key & 0xFF000000) >> 24;
					break;

		}

		data[i] = data[i] ^ octet;

	}

}

/*int main (int argc, char **argv){

	//a single frame masked text message that contains "hello"
	//0x81 0x85 0x37 0xfa 0x21 0x3d 0x7f 0x9f 0x4d 0x51 0x58

	return(EXIT_SUCCESS);
}*/
