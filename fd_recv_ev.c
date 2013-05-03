#include "fd.h"

#define PAYLOAD_EXT_16 126
#define PAYLOAD_EXT_64 127

static void unmask_payload(char* data, uint64_t len, uint32_t key);

int fd_recv(fd_socket_t *sock, char *buff){

  return 0;
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

/* 
   calls recv once, adding the bytes it got into
   the socket struct's buffer. returns the number of
   bytes that still need to be received.
*/
void fd_recv_nb(struct ev_loop *loop, ev_io *w, int revents){
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
  int	status = 0, fin = 0, rsv1 = 0, rsv2 = 0, rsv3 = 0, is_masked = 0, offset = 0, i;
  uint8_t	payload_len = 0, temp = 0;
  uint16_t ext_payload_len1 = 0, temp1 = 0;
  uint32_t mask_key = 0, temp3 = 0;
  uint64_t ext_payload_len2 = 0, temp2 = 0;
  unsigned int opcode = 0;
  char byte;
  fd_socket_t *socket = wtos(w, read_w);

	assert_event(EV_READ);

  /* increment recv count */
  socket->recvs += 1;

  /* call recv once, saving the byte count received in status */
  if((status = 
      recv(socket->tcp_sock, 
					 socket->buffer + socket->bytes_received, 
					 MAX_HEADER_LEN + MAX_MESSAGE_LEN - socket->bytes_received, 
					 0)) < 0){
    /* since we're nonblocking, these are ok */
    if(errno != EAGAIN && errno != EWOULDBLOCK){
      perror(__FILE__);
      exit(errno);
    }
    printf("fd_recv_nb invoked, but recv returned EAGAIN or EWOULDBLOCK\n");
    return;
  }

  printf("Bytes received in call #%d: %d\n",socket->recvs, status);
  /* update the number of received bytes on the socket */
  socket->bytes_received += status;

  /* if we have not processed the header */
  if(socket->bytes_expected == 0){
    /* if the first two bytes are available, 
       and we haven't determined the header length */
    if(socket->bytes_received > 2 && 
       socket->header_len == 0){
      /* process the first byte */
      byte = *(socket->buffer);
      temp = (uint8_t) byte;

      fin = (temp & 0x80) >> 7;
      rsv1 = (temp & 0x40) >> 6;
      rsv2 = (temp & 0x20) >> 5;
      rsv3 = (temp & 0x10) >> 4;
      opcode = (temp & 0xF);
      socket->last_recv_opcode = opcode;

      /* process the second byte */
      byte = *(socket->buffer + 1);
      temp = (uint8_t) byte;

      is_masked = (temp & 0x80) >> 7;
      payload_len = temp & 0x7F;

      /* assign the header_len on the socket */
      if(is_masked){
	if(payload_len < PAYLOAD_EXT_16)
	  socket->header_len = 6;
	else if(payload_len == PAYLOAD_EXT_16)
	  socket->header_len = 8;
	else if(payload_len == PAYLOAD_EXT_64)
	  socket->header_len = 14;
	else
	  /* what the hell is going on if we're here? */
	  printf("%s: unknown payload length\n", __FILE__);
      } else {
	/* why would it not be masked? 
	   i guess we should still do this */
	if(payload_len < PAYLOAD_EXT_16)
	  socket->header_len = 2;
	else if(payload_len == PAYLOAD_EXT_16)
	  socket->header_len = 4;
	else if(payload_len == PAYLOAD_EXT_64)
	  socket->header_len = 10;
	else
	  /* what the hell is going on if we're here? */
	  printf("%s: unknown payload length\n", __FILE__);
      }
    }
		
    /* if we have received the header,
       but we haven't determined the payload length*/
    if(socket->header_len > 0 &&
       socket->bytes_received >= socket->header_len){
      /* (re)scan the second byte */
      byte = *(socket->buffer + 1);
      temp = (uint8_t) byte;
      is_masked = (temp & 0x80) >> 7;
      payload_len = temp & 0x7F;

      /* determine payload length */
      if(payload_len < PAYLOAD_EXT_16){
	socket->bytes_expected = payload_len + socket->header_len;
	offset = 2;
      }
      else if(payload_len == PAYLOAD_EXT_16){
	/* scan 2 bytes out of the buffer */
	ext_payload_len1 = 0;
	for(i = 0; i < 2; i++){
	  byte = *(socket->buffer + 2 + i);
	  temp1 = (uint16_t) byte & 0xFF;
	  temp1 = temp1 << (8 - (8*i));
	  ext_payload_len1 |= temp1;
	}

	socket->bytes_expected = ext_payload_len1 + socket->header_len;
	offset = 4;
      }
      else if(payload_len == PAYLOAD_EXT_64){
	/* scan 8 bytes out of the buffer */
	ext_payload_len2 = 0;
	for(i = 0; i < 8; i++){
	  byte = *(socket->buffer + 2 + i);
	  temp2 = (uint64_t) byte & 0xFF;
	  temp2 = temp2 << (56 - (8*i));
	  ext_payload_len2 |= temp2;
	}

	socket->bytes_expected = ext_payload_len2 + socket->header_len;
	offset = 10;
      }

      /* save the mask key */
      if(is_masked){
	mask_key = 0;
	for(i = 0; i < 4; i++){
	  byte = *(socket->buffer + offset + i);
	  temp3 = (uint32_t) byte & 0xFF;
	  temp3 = temp3 << (24 - (8*i));
	  mask_key |= temp3;
	}
				
	/* Reverse order of the mask */
	mask_key = (0xFF000000 & (mask_key << 24)) | (0x00FF0000 & (mask_key << 8)) | 
	  (0x0000FF00 & (mask_key >> 8)) | (0x000000FF & (mask_key >> 24));

	socket->mask_key = mask_key;
      }
    }
  }


  printf("Total bytes_received: %d\n",socket->bytes_received);
  printf("Total bytes_expected: %d\n",socket->bytes_expected);
  /* printf("Opcode: %d\n", opcode); */

  /* if we have received the full payload */
  if(socket->bytes_received == socket->bytes_expected){
	  printf("Done receiving\n");

    /* unmask the payload */
    if(socket->mask_key){		  
      /* offset is the number of bytes needer for payload length */
      unmask_payload(socket->buffer + socket->header_len,
		     socket->bytes_received - socket->header_len,
		     socket->mask_key);
    }

    //		printf("fd_recv_nb: unmasked data is \"%s\"\n", socket->buffer + socket->header_len);

    if(opcode == PING){
      /* send a pong */
      status = fd_send(socket, socket->buffer, PONG);
    }
    else if(opcode == PONG){
      /* check to make sure APPLICATION data is the same as was in the PING (how?) */
    }
    else if(opcode == CONNECTION_CLOSE){
      /* send a matching CLOSE message and close the socket gracefully (fd_close function?) */
      printf("Close message received\n");
      status = fd_send(socket, socket->buffer, CONNECTION_CLOSE);
      //		  fd_socket_destroy(socket, loop);
    }

    //		printf("Receives: %d\n",socket->recvs);
		


    /* printf("About to send with header_len: %d\n", socket->header_len); */
    //		printf("Buffer is: %s\n", socket->buffer + socket->header_len);

    /* fd_send(socket, socket->buffer + socket->header_len, TEXT); */
		
    /* notify the "data available" callback that the recv is done */
		socket->data_cb(socket, socket->buffer + socket->header_len);

    /* Reset data in socket struct to get ready for next recv */
    memset(socket->buffer, 0, MAX_HEADER_LEN + MAX_MESSAGE_LEN);
    socket->recvs = 0;
    socket->bytes_expected = 0;
    socket->bytes_received = 0;
    socket->header_len = 0;
    socket->mask_key = 0;
  }
}
