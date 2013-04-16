/* fd_send.c by Matthew Diephuis

 Implement send function for fire drake websocket library


*/

/* 

   send(socket, buffer){
   
   based on buffer, determine how to structure header

   send modified buffer

   return success or failure

}

 */

#include "fd.h"

int fd_send(fd_socket_t *sock, char *buff, int opcode){
  unsigned int header;
  int i = 0, skip, buf_size = strlen(buff);
  char *buff_to_send;
  char val;
  char header_str[MAX_HEADER_LEN];
	memset(header_str,0,strlen(header_str));
  
	// build up the header

  printf("buff to send: %s, size: %d\n", buff, buf_size);

  // first 4 bits (FIN, RSV1-3) are always 0 
  // next 4 bits are opcode, we use 0x1 for text frame 
  
  header = 0x81;

  printf("first 4 header bits in decimal: %u and hex: %04x\n",header, header);
  
  // Use payload length to determine payload length bits 
  if (buf_size <= 125) {
    // data length bits are just the size
    printf("Header shifted 8 right in decimal: %u and hex: %012x\n",header, header);
    header = header | ((0x7D & buf_size) << 8);
    printf("header + small payload in decimal: %u and hex: %012x\n",header, header);

		/* stick the header in the buffer */

		/* first byte */
		val = (char)(header & 0xFF);
		header_str[i++] = val;
		header = header >> 8;
		
		/* size byte */
		val = (char)(header & 0xFF);
		header_str[i++] = val;
		header = header >> 8;

		skip = 2;
  } 
  else if (buf_size <= 65535){
    // first append the data length to indicate 16 bit length coming
    header = header | 0x7E00;

    // represent payload size in 16 bits and add to right hand side
    header = (header) | ((0xFFFF & buf_size) << 16);

		/* stick the header in the buffer */

		/* first byte */
		/* val = (char)(header & 0xFF); */
		/* header_str[i++] = val; */
		/* header = header >> 8; */
		
		/* /\* size byte *\/ */
		/* val = (char)(header & 0xFF); */
		/* header_str[i++] = val; */
		/* header = header >> 8; */

		/* /\* two bytes for message length *\/ */
		/* val = (char)((header & 0xFF00) >> 8); */
		/* header_str[i++] = val; */
		/* val = (char)(header & 0xFF); */
		/* header_str[i++] = val; */

		skip = 4;
  }
  else {
    // first append the data length to indicate 64 bit length coming
    header = header | 0x7F00;

    // represent payload size in 64 bits and add to right hand side
    // What happens with messages bigger than 64 bits? 
    header = (header) | ((0x7FFFFFFFFFFFFFFF & buf_size) << 16);

  }

  // Now need to turn the header bits into ASCII representation to make header string
  // take 1 byte at a time (2 hex characters) and get ASCII value, prepend to header_str
  /* int i = 0; */

  /* memset(header_str,0,strlen(header_str)); */
	/* while(header > 0){ */
  /*   /\* sprintf(val,%u,header & 0xFF); *\/ */
  /*   val = (char)(header & 0xFF); */
  /*   header_str[i++] = val; */
  /*   /\* strcat((char*)val,header_str); *\/ */
  /*   header = header >> 8; */
  /* } */

  /* header_str = strrev(header_str); */

  printf("Header_str: %s\n",header_str);
  // prepend header to buffer
  buff_to_send = malloc(MAX_HEADER_LEN + MAX_MESSAGE_LEN + 1);
  buff_to_send = strcat(header_str, buff);
  printf("buff: %s, buff_to_send: %s\n",buff,buff_to_send); 

  // send the buffer with the correct header to socket
  return send(sock->tcp_sock,buff_to_send,strlen(buff_to_send),0);
  

}

char *fd_strcat(char *header, char *buff, int skip) {
	char output[MAX_HEADER_LEN + MAX_MESSAGE_LEN + 1];
	int i,j;

	for(i = 0, j = 0; j < strlen(header); i++, j++){
		output[i] = header[j];
	}

	for(i = skip, j = 0; j < strlen(buff); i++, j++){
		output[i] = buff[j];
	}
	output[i] = '\0';

	return output;
}
