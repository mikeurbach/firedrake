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

#include <stdlib.h>
#include <sys/sock.h>

#define MAX_HEADER_LEN 10
#define MAX_MESSAGE_LEN 100000 // need to figure out what this actually should be

int send(fd_sock *sock, char * buff){
  unsigned int header;
  int buf_size = sizeof(buff);
  char buff_to_send[MAX_HEADER_LEN + MAX_MESSAGE_LEN];
  char val;
  char header_str[MAX_HEADER_LEN];
  // build up the header

  // first 4 bits (FIN, RSV1-3) are always 0 
  // next 4 bits are opcode, we use 0x1 for text frame 
  header = 1 & 0xFF;
  
  // Use payload length to determine payload length bits 
  if (buf_size <= 125) {
    // data length bits are just the size
    header = header << 8;
    header = header | (0x7D & buf_size);
  } 
  else if (buf_size <= 65535){
    // first append the data length to indicate 16 bit length coming
    header = header << 8;
    header = header & 0x7E;

    // represent payload size in 16 bits and add to right hand side
    header = (header << 16) | (0xFFFF & buf_size)  
  }
  else {
    // first append the data length to indicate 64 bit length coming
    header = header << 8;
    header = header && 0x7F;

    // represent payload size in 64 bits and add to right hand side
    // What happens with messages bigger than 64 bits? 
    header = (header < 64) | (0x7FFFFFFFFFFFFFFF & buf_size)
  }

  // Now need to turn the header bits into ASCII representation to make header string
  // take 1 byte at a time (2 hex characters) and get ASCII value, prepend to header_str
  while (header > 0) {
    /* sprintf(val,%u,header & 0xFF); */
    val = (char)(header & 0xFF);
    strcat(val,header_str);
    header = header >> 8;
  }

  // prepend header to buffer
  buff_to_send = strcat(header_str,buff);

  // send the buffer with the correct header to socket
  send(sock->tcp_sock,buff_to_send);
  

}
