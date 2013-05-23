/* fd_send_ev.c 
 * non-blocking send function
 */

#include "fd.h"

int fd_send(fd_socket_t *sock, char *buff, int opcode){
  struct ev_loop *loop = EV_DEFAULT;
  unsigned long long header, mask;
  int i = 0, skip, buf_size = strlen(buff);
  char val;

  /* first 4 bits (FIN, RSV1-3) are always 0  */
  /* next 4 bits are opcode, we use 0x1 for text frame  */
  header = 0x81;
  
  /* use payload length to determine payload length bits  */
  if (buf_size <= 125) {
		/* malloc our output buffer */
		sock->out_buffer = malloc(buf_size + 3);
		memset(sock->out_buffer, 0, buf_size);

    /* data length bits are just the size */
    header = header | ((0x7F & buf_size) << 8);

		/* stick the header in the buffer */

		/* first byte */
		val = (char)(header & 0xFF);
		sock->out_buffer[i++] = val;
		header = header >> 8;
		
		/* size byte */
		val = (char)(header & 0xFF);
		sock->out_buffer[i++] = val;
		header = header >> 8;

		skip = 2;
  } 
  else if (buf_size <= 65535){
		/* malloc our output buffer */
		sock->out_buffer = malloc(buf_size + 5);
		memset(sock->out_buffer, 0, buf_size);

    /* first append the data length to indicate 16 bit length coming */
    header = header | 0x7E00;

    /* represent payload size in 16 bits and add to right hand side */
    header = (header) | ((0xFFFF & buf_size) << 16);

		/* stick the header in the buffer */

		/* first byte */
		val = (char)(header & 0xFF);
		sock->out_buffer[i++] = val;
		header = header >> 8;
		
		/* size byte */
		val = (char)(header & 0xFF);
		sock->out_buffer[i++] = val;
		header = header >> 8;

		/* two bytes for message length */
		val = (char)((header & 0xFF00) >> 8);
		sock->out_buffer[i++] = val;
		val = (char)(header & 0xFF);
		sock->out_buffer[i++] = val;

		skip = 4;
  }
  else {
		int j;

		/* malloc our output buffer */
		sock->out_buffer = malloc(buf_size + 11);
		memset(sock->out_buffer, 0, buf_size);

    /* first append the data length to indicate 64 bit length coming */
    header = header | 0x7F00;

    /* represent payload size in 64 bits and add to right hand side */
    /* what happens with messages bigger than 64 bits?  */
    header = (header) | ((0x7FFFFFFFFFFFFFFF & buf_size) << 16);

		/* stick the header in the buffer */

		/* first byte */
		val = (char)(header & 0xFF);
		sock->out_buffer[i++] = val;
		header = header >> 8;
		
		/* size byte */
		val = (char)(header & 0xFF);
		sock->out_buffer[i++] = val;
		header = header >> 8;

		/* 8 bytes for message length */
		mask = 0xFF00000000000000;

		for(j=56;j >= 0; j = j - 8) {
			val = (char)((header & mask) >> j);
			sock->out_buffer[i++] = val;
			mask = mask >> 8;
		}
		
		skip = 10;

  }

  /* prepend header to buffer */
  fd_strcat(sock->out_buffer, buff, skip);

	sock->bytes_outgoing = strlen(buff) + skip;
	sock->bytes_sent = 0;
	sock->sends = 0;
	ev_io_init(&sock->write_w, fd_send_nb, sock->tcp_sock, EV_WRITE);
	ev_io_start(loop, &sock->write_w);

  return 0;
}

void fd_send_nb(struct ev_loop *loop, ev_io *w, int revents){
	int status;
	fd_socket_t *socket = wtos(w, write_w);

	assert_event(EV_WRITE);

	/* call send once, saving the number of bytes sent */
	if(socket->bytes_sent < socket->bytes_outgoing){
		if((status = 
				send(socket->tcp_sock, 
						 socket->out_buffer + socket->bytes_sent, 
						 socket->bytes_outgoing,
						 0)) < 0){
			/* since we're nonblocking, these are ok */
			if(errno != EAGAIN && errno != EWOULDBLOCK){
				perror(__FILE__);
				return;
			}
			
			fd_log_w("send invoked, but returned EAGAIN or EWOULDBLOCK\n");
		}
		
		socket->bytes_sent += status;
		++socket->sends;
		
		fd_log_i("bytes sent in call #%d: %d\n" 
					 "	Total bytes_sent: %d\n"
					 "	Total bytes_outgoing: %d\n",
					 socket->sends, status, 
					 (int) socket->bytes_sent, (int) socket->bytes_outgoing);
		

	}

	/* once we've sent everything */
	if(socket->bytes_sent == socket->bytes_outgoing){
		/* unplug from the event loop */
		ev_io_stop(loop, &socket->write_w);
		
		fd_log_i("done sending\n");

		free(socket->out_buffer);
		socket->bytes_outgoing = 0;
		socket->bytes_sent = 0;
		socket->sends = 0;
	}
}

void fd_strcat(char *output, char *buff, int skip) {
	int i,j;

	for(i = skip, j = 0; j < strlen(buff); i++, j++){
		output[i] = buff[j];
	}
	output[i] = '\0';

}
