#include "fd.h"


#define hash_sock(sockid, size) (sockid % size)


fd_socket_t *fd_socket_new(void){
	fd_socket_t *new_sock = (fd_socket_t *) malloc(sizeof(fd_socket_t));
	
	//set defaults
	new_sock->tcp_sock = -1;
	new_sock->last_recv_opcode = -1;
	new_sock->is_open = false;

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
  sock->is_open = false;
  close(sock->tcp_sock);
  //  free(sock->buffer);
  //  free(sock->out_buffer);
  free(sock);
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

	while(1)
		while( (line = (char *) qget(log_queue)) != NULL )
			fprintf(log_file, line);
}

/* create and return a blank hash table, given a size */
fd_socket_hash init_socket_hashtable(int size){
	fd_socket_hash hashtable = malloc(sizeof(struct _fd_socket_hash));
	fd_socket_t **table = 
		(fd_socket_t **) calloc(size, sizeof(struct _fd_socket_t));
	hashtable->size = size;
	hashtable->table = table;
	return hashtable;
}

fd_socket_t *fd_lookup_socket(int sockid){
	int slot; 
	fd_socket_t *sock;

	/* check if hashtable has been initialized yet */
	if(socket_hashtable == NULL)
		socket_hashtable = init_socket_hashtable(HASH_SIZE);

	slot = hash_sock(sockid, socket_hashtable->size);

	/* look through the nodes at this slot */
	for(sock = socket_hashtable->table[slot];
	    sock != NULL && (sock->tcp_sock != sockid); 
	    sock = sock->next);
	//

	return sock;
}

/* remove all sockets from hashtable */
void destroy_all_sockets(){
  fd_socket_t *current, *prev;

  for(int i = 0; i < socket_hashtable->size; i++){
    current = socket_hashtable->table[i];

    /* if slot in hashtable has socket, remove all sockets in slot */
    if (current != NULL){
      prev = current;
      current = current->next;
      for (current; current != NULL; current = current->next){
	fd_socket_destroy(prev->tcp_sock);
	prev = current;
      }
      fd_socket_destroy(prev->tcp_sock);
    }    
  }

  /* now free the dictionary itself */
  free(socket_hashtable);

}



/* Remove socket from hastable */
void remove_sock_from_hashtable(fd_socket_t *sock){
  int slot = hash_sock(sock->tcp_sock, socket_hashtable->size);
  fd_socket_t *current, *prev;


  if (sock != NULL){
    current = socket_hashtable->table[slot];
    prev = NULL;

    for (current;
	 current->next != NULL && current->next->tcp_sock != sock->tcp_sock;
	 current = current->next)
      prev = current;
    
    /* if prev is null, sock is first in slot  */
    if (prev == NULL)
      socket_hashtable->table[slot] = current->next;
    else
      current->next = sock->next;

    fd_log_i("removed sock id %d from hashtable\n", sock->tcp_sock);
  } 
  else
    fd_log_w("atempted removal of socket with id %d failed: socket was not found\n", sock->tcp_sock);

}


void add_sock_to_hashtable(fd_socket_t *sock){
  int slot = hash_sock(sock->tcp_sock, socket_hashtable->size);

  fd_log_i("adding socket %d to hashtable\n",sock->tcp_sock);

  /* put it in the hash table if it doesnt already exist*/
  if (fd_lookup_socket(sock->tcp_sock) == NULL){
    sock->next = socket_hashtable->table[slot];
    socket_hashtable->table[slot] = sock;
  }
}
