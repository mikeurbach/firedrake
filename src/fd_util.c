/* 
    firedrake: an efficient library for writing websocket servers

    Copyright (C) 2013 Mike Urbach, Matt Diephui, Drew Jankowski

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

		Report bugs to <mikeurbach@gmail.com>
*/

#include "fd.h"


#define hash_sock(sockid, size) (sockid % size)


fd_socket_t *fd_socket_new(void){
	fd_socket_t *new_sock = (fd_socket_t *) malloc(sizeof(fd_socket_t));
	
	//set defaults
	new_sock->tcp_sock = -1;
	new_sock->__internal.last_recv_opcode = -1;
	new_sock->__internal.is_open = false;

	return (new_sock);
}

/* Remove from channel list and from socket dict */
void fd_socket_destroy(int sockid){
  struct ev_loop *loop = EV_DEFAULT;
  fd_socket_t *sock = fd_lookup_socket(sockid);
  ev_io_stop(loop, &sock->read_w);

  /* remove socket from channels and hashtable */
  fd_remove_from_all_channels(sockid);
  remove_sock_from_hashtable(sock);

  fd_log_i("closing socket with file descriptor: %d\n",sock->tcp_sock);

  /* cleanup the socket struct */
  sock->__internal.is_open = false;
  close(sock->tcp_sock);
  /* if (sock->buffer != NULL) */
  /*   free(sock->buffer); */
  /* if (sock->out_buffer != NULL) */
  /*   free(sock->out_buffer); */

	// TODO: we can't free part of a static table, what should we do?
  // free(sock);
}



void fd_log_write(int level, char *file, int line, char *fmt, ...){
	char output[MAX_LOG_LINE], message[MAX_LOG_LINE];
	va_list args;

	memset(output, 0, MAX_LOG_LINE);
	memset(message, 0, MAX_LOG_LINE);

	/* start the line with the appropriate prefix */
	switch (level) {
	case DEBUG:
		snprintf(output, MAX_LOG_LINE, "[DEBUG] %s:%d: ", file, line);
		break;
	case INFO:
		snprintf(output, MAX_LOG_LINE, "[INFO] %s:%d: ", file, line);
		break;
	case MESSAGE:
		snprintf(output, MAX_LOG_LINE, "[MESSAGE] %s:%d: ", file, line);
		break;
	case WARNING:
		snprintf(output, MAX_LOG_LINE, "[WARNING] %s:%d: ", file, line);
		break;
	case CRITICAL:
		snprintf(output, MAX_LOG_LINE, "[CRITICAL] %s:%d: ", file, line);
		break;
	case ERROR:
		snprintf(output, MAX_LOG_LINE, "[ERROR] %s:%d: ", file, line);
		break;
	}
	
	/* copy the message from the user code */
	va_start(args, fmt);
	vsnprintf(message, MAX_LOG_LINE, fmt, args);
	va_end(args);

	/* assemble the complete line */
	strncat(output, message, MAX_LOG_LINE - strlen(output) - 1);

	/* enqueue this line to be written to the log */
	qput(log_queue, (void *) strdup(output));
}

/* runs in its own thread so the main thread won't block */
void *fd_log(void *data){
	char *line;

	while(1){
		while( (line = (char *) qget(log_queue)) != NULL ){
			fprintf(log_file, line);
			fflush(log_file);
		}
	}
}

fd_socket_t *fd_lookup_socket(int sockid){
	return (fd_socket_t *) &socket_table[sockid];
}

/* remove all sockets from hashtable */
void destroy_all_sockets(){

  for(int i = 0; i < MAX_SOCKETS; i += sizeof(fd_socket_t))
		free((fd_socket_t *) &socket_table[i]);

  free(socket_table);
}

/* Remove socket from table */
void remove_sock_from_hashtable(fd_socket_t *sock){
  int slot = sock->id;

	// TODO: we are going to have to mark sock->id as an open slot
	// in a list somewhere for the socket allocator
	// free(sock); 
	fd_log_i("removed socket with id %d from table", slot);
}

/* add socket to table */
int add_sock_to_hashtable(int connfd){
	static int slot = 0;

	/* if we have room */
	if(slot < MAX_SOCKETS){
		/* initialize this socket's space */
		memset(&socket_table[slot], 0, sizeof(fd_socket_t));

		/* intitialize its socket, id, and buffer */
		socket_table[slot].tcp_sock = connfd;
		socket_table[slot].id = slot;
		socket_table[slot].__internal.buffer =
			malloc(MAX_MESSAGE_LEN);
		memset(socket_table[slot].__internal.buffer, 0, 
					 MAX_MESSAGE_LEN);

		fd_log_i("added socket with id %d to table\n", slot);

		return slot ++;
	} else {
		fd_log_c("could not add socket to table; table full");

		return -1;
	}
}
