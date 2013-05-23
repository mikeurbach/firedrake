
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

void fd_socket_destroy(fd_socket_t *sock, struct ev_loop *loop){
	ev_io_stop(loop, &sock->read_w);
	fd_socket_close(sock);
	free(sock);

}

/* Remove from channel list and from socket dict */
int fd_socket_close(fd_socket_t *sock){


  printf("Closing socket #: %d\n",sock->tcp_sock);

  //  close_all_channels();
  //  fd_close_channel("chatroom");
  
  /* Change to remove from channels given a sock ID */
  remove_from_all_channels(sock->tcp_sock);
  //  remove_from_channel("chatroom", sock->tcp_sock);
  remove_sock_from_hashtable(sock);

  fd_log_i("closing socket with file descriptor: %d\n",sock->tcp_sock);

  sock->is_open = false;
  return ( close(sock->tcp_sock) );

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

/* Remove socket from hastable */
void remove_sock_from_hashtable(fd_socket_t *sock){
  int slot = hash_sock(sock->tcp_sock, socket_hashtable->size);
  fd_socket_t *current, *prev;


  if (fd_lookup_socket(sock->tcp_sock) != NULL){
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

    printf("Removed sock id %d from hashtable\n", sock->tcp_sock);
  } 
  else
    printf("Atempted removal of socket with id %d failed: socket was not found\n", sock->tcp_sock);

}


void add_sock_to_hashtable(fd_socket_t *sock){
  int slot = hash_sock(sock->tcp_sock, socket_hashtable->size);

  printf("Adding socket %d to hashtable\n",sock->tcp_sock);

  /* put it in the hash table if it doesnt already exist*/
  if (fd_lookup_socket(sock->tcp_sock) == NULL){
    sock->next = socket_hashtable->table[slot];
    socket_hashtable->table[slot] = sock;
  }
}

