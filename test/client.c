#include "fd.h"


int fd_client_send(int sockfd, char *buff, int opcode, int fin);
static void mask_payload(char* data, int len, uint32_t key);
int fd_client_recv(int sockfd, char *buff);
int fd_client_handshake(int sockfd);
static void print_menu();
static int get_user_string(char *buffer);

int main (int argc, char **argv){
  int port, sockfd, fin;
  socklen_t serverlen;
  struct sockaddr_in servaddr;
  char buffer[MAX_MESSAGE_LEN];
  char choice;

  /* read the port from the command line */
  if(argc != 2){
    printf("usage: client <port>\n");
    exit(1);
  }
  port = atoi(argv[1]);

  /* call socket to get a file descriptor */
  if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
    perror(__FILE__);
    exit(errno);
  }

  /* prepare the socket struct */
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  serverlen = sizeof(servaddr);

  /* connect to server */
  if (connect(sockfd, (struct sockaddr *) &servaddr, serverlen) == -1) {
    perror(__FILE__);
    exit(errno);
  }

  /* Handshake */
  fd_client_handshake(sockfd);

  /* continually send entered text 
  while (fgets(buffer, MAX_MESSAGE_LEN, stdin) != NULL){

    printf("Sending: %s\n", buffer);
    fd_client_send(sockfd, buffer, 0);

    fd_client_recv(sockfd, buffer);
    printf("Received: %s\n", buffer);
  }*/

  print_menu();

	printf("fin bit: ");
	scanf("%d\n", &fin);

  while(scanf("%c\n", &choice) != EOF){
    
    memset(buffer, 0, MAX_MESSAGE_LEN);

    fflush(stdin);

    switch (choice){
		case '0': 
			close(sockfd);
			return(EXIT_SUCCESS);

		case '1': 
			get_user_string(buffer);
			fd_client_send(sockfd, buffer, CONTINUATION, fin);
			break;
							
		case '2': 
			get_user_string(buffer);
			fd_client_send(sockfd, buffer, TEXT, fin);
			break;

		case '3': 
			get_user_string(buffer);
			fd_client_send(sockfd, buffer, BINARY, fin);
			break;

		case '4': 
			fd_client_send(sockfd, NULL, CONNECTION_CLOSE, fin);
			break;

		case '5': 
			fd_client_send(sockfd, NULL, PING, fin);
			break;

		case '6': 
			fd_client_send(sockfd, NULL, PONG, fin);
			break;
    }

		if(fin){
			fd_client_recv(sockfd, buffer);
			printf("\nReceived: %s\n\n", buffer);
		}

    print_menu();
		printf("fin bit: ");
		scanf("%d\n", &fin);
  }

}


int fd_client_handshake(int sockfd){
  char *buffer, *key, *encoded, *headers;
  int bufflen, keylen = 16;

  headers =  
    "Get / HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Key: ";    

  /* generate and b64encode a bogus key */
  key = malloc(keylen);
  strcpy(key,"INeed16charsfour");
  encoded = b64_encode(key,keylen);

  bufflen = strlen(headers) + strlen(encoded) + strlen("\r\nSec-WebSocket-Version: 13\r\n");

  /* put everything together */
  buffer = malloc(bufflen);
  memset(buffer, 0, bufflen);
  strcpy(buffer,headers);
  strcat(buffer,encoded);
  strcat(buffer,
	 "\r\n"
	 "Sec-WebSocket-Version: 13\r\n");

  if (send(sockfd,buffer,bufflen,0) == -1){
    perror(__FILE__);
    exit(errno);
  }
  
  printf("Sent the handshake.\n");

  if (recv(sockfd,buffer,MAX_MESSAGE_LEN,0) == -1){
    perror(__FILE__);
    exit(errno);
  }

  printf("Receiceved the handshake.\n");

  return 0;
}

static int get_user_string(char *buffer){
  int received = 0;

	fflush(stdout);
 
	printf("Enter your message: ");

  scanf("%[^\n]s", buffer);

  fflush(stdin);

  return(received);
}


int fd_client_send(int sockfd, char *buff, int opcode, int fin){
  unsigned long long header, mask;
  int i = 0, skip, buf_size;
  char buff_to_send[MAX_HEADER_LEN + MAX_MESSAGE_LEN + 1];
  char val;

	if(buff != NULL)
		buf_size = strlen(buff);
	else
		buf_size = 0;

  // build up the header
  memset(buff_to_send, 0, MAX_HEADER_LEN + MAX_MESSAGE_LEN + 1);
  //  printf("buff passed into fd_send: %s, size: %d\n", buff, buf_size);

	printf("client sending with opcode: %x\n", opcode);
	
	/* assemble the first byte of the header */ 
	header = fin << 7 | opcode;

  // Use payload length to determine payload length bits 
  if (buf_size <= 125) {
    // data length bits are just the size

    header = header | ((0x80 | buf_size) << 8);


		/* stick the header in the buffer */

		/* first byte */
		val = (char)(header & 0xFF);
		buff_to_send[i++] = val;
		header = header >> 8;
		
		/* size byte */
		val = (char)(header & 0xFF);
		buff_to_send[i++] = val;
		header = header >> 8;

		skip = 2;
  } 
  else if (buf_size <= 65535){
    // first append the data length to indicate 16 bit length coming
    header = header | 0xFE00;

    // represent payload size in 16 bits and add to right hand side
    header = (header) | ((0xFFFF & buf_size) << 16);

		/* stick the header in the buffer */

		/* first byte */
		val = (char)(header & 0xFF);
		buff_to_send[i++] = val;
		header = header >> 8;
		
		/* size byte */
		val = (char)(header & 0xFF);
		buff_to_send[i++] = val;
		header = header >> 8;

		/* two bytes for message length */
		val = (char)((header & 0xFF00) >> 8);
		buff_to_send[i++] = val;
		val = (char)(header & 0xFF);
		buff_to_send[i++] = val;

		skip = 4;
  }
  else {
		int j;
		uint32_t temp, mask_key = 42;

    // first append the data length to indicate 64 bit length coming
    header = header | 0xFF00;

    // represent payload size in 64 bits and add to right hand side
    // What happens with messages bigger than 64 bits? 
    header = (header) | ((0x7FFFFFFFFFFFFFFF & buf_size) << 16);

		/* stick the header in the buffer */

		/* first byte */
		val = (char)(header & 0xFF);
		buff_to_send[i++] = val;
		header = header >> 8;
		
		/* size byte */
		val = (char)(header & 0xFF);
		buff_to_send[i++] = val;
		header = header >> 8;

		/* 8 bytes for message length */
		mask = 0xFF00000000000000;

		for(j=56;j >= 0; j = j - 8) {
			val = (char)((header & mask) >> j);
			buff_to_send[i++] = val;
			mask = mask >> 8;
		}
		
		skip = 10;

  }

  // add mask to buff 
  temp = mask_key;

  for (i; i < (skip + 4); i++){
    val = (char)(temp & 0xFF);
    buff_to_send[i] = val;
    temp = temp >> 8;
  }

  skip += 4;


  /* fd_strcat(buff_to_send, "mask",skip); */
  /* skip = skip + 4; */

  // Mask the payload
  mask_payload(buff, buf_size, mask_key);

  // prepend header to buffer
  fd_strcat(buff_to_send, buff, skip);
  //  printf("buff_to_send: %s\n",buff_to_send); 

  // send the buffer with the correct header to socket

  return send(sockfd, buff_to_send, skip + buf_size, 0);
}


void fd_strcat(char *output, char *buff, int skip) {
  int i = skip, j = 0;

	if(buff != NULL)
		for(i, j; j < strlen(buff); i++, j++)
			output[i] = buff[j];

  output[i] = '\0';

}


static void mask_payload(char* data, int len, uint32_t key){

	int i;
  uint8_t	octet;

  for(i = 0; i < len; i++){
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

int fd_client_recv(int sockfd, char *buff){
	int					status = 0,
						fin = 0,
						rsv1 = 0,
						rsv2 = 0,
						rsv3 = 0,
	  is_masked = 0,
	  offset = 0;

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
	status = recv(sockfd, (&temp), 1, 0);

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
	status = recv(sockfd, (&payload_len), 1, 0);
	is_masked = (payload_len & 0x80) >> 7;
	//printf("fd_recv: is_masked is %d\n", is_masked);

	payload_len = (payload_len & 0x7F);
	//printf("fd_recv: payload_len is %d\n", payload_len);

	if(payload_len < PAYLOAD_EXT_16){
		final_payload_len = payload_len;
	}
	else if(payload_len == PAYLOAD_EXT_16){
	  status = recv(sockfd, (&ext_payload_len1), 2, 0);
		

// byte are recevied in wrong order, so need to reverse them
		ext_payload_len1 = (0xFF00 & (ext_payload_len1 << 8)) | (0x00FF & (ext_payload_len1 >> 8));
		final_payload_len = ext_payload_len1;
	}
	else if(payload_len == PAYLOAD_EXT_64){

	  status = recv(sockfd, (&ext_payload_len2), 8, 0);
	  
	  ext_payload_len2 = (ext_payload_len2 & 0x00000000000000FFUL) << 56 | (ext_payload_len2 & 0x000000000000FF00UL) << 40 |
	    (ext_payload_len2 & 0x0000000000FF0000UL) << 24 | (ext_payload_len2 & 0x00000000FF000000UL) << 8 |
	    (ext_payload_len2 & 0x000000FF00000000UL) >> 8 | (ext_payload_len2 & 0x0000FF0000000000UL) >> 24 |
	    (ext_payload_len2 & 0x00FF000000000000UL) >> 40 | (ext_payload_len2 & 0xFF00000000000000UL) >> 56;		
	  final_payload_len = ext_payload_len2;
	}

	//printf("fd_client_recv: final_payload_len is %d\n", final_payload_len);


//Parse Masking key (0 or 4 bytes) All frames from client to server are masked by this value
//(absent if the mask is 0)

	if(is_masked){
		status = recv(sockfd, (&mask_key), 4, 0);
		//printf("fd_recv: mask is %X\n", mask_key);
	}


	//Place Payload data in buffer and return status
	while (offset < final_payload_len) {
	  status = recv(sockfd, buff + offset, final_payload_len - offset, 0);
	  if(status < 0){
		perror(__FILE__);
		return 1;
	  }
	  offset += status;	
	}


 	//printf("fd_recv: masked data is \"%s\"\n", buff);


 	if(is_masked){
 		mask_payload(buff, final_payload_len, mask_key);
	}

	//printf("fd_client_recv: unmasked data is \"%s\"\n", buff);

	if(opcode == PING){
		//send a pong
		status = fd_client_send(sockfd, buff, PONG, fin);
	}
	else if(opcode == PONG){
		//check to make sure APPLICATION data is the same as was in the PING (how?)
	}
	else if(opcode == CONNECTION_CLOSE){
		//send a matching CLOSE message and close the socket gracefully (fd_close function?)
		status = fd_client_send(sockfd, buff, CONNECTION_CLOSE, fin);

	}

	return (status);
}

static void print_menu(){
  printf("[0] Quit the client.\n");
  printf("[1] Send a continuation frame.\n");
  printf("[2] Send a text frame.\n");
  printf("[3] Send a binary frame.\n");
  printf("[4] Send a connection close frame.\n");
  printf("[5] Send a ping frame.\n");
  printf("[6] Send a pong frame.\n");
}
