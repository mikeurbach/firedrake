#include "fd.h"

/* create and return a blank hash table, given a size */
fd_channel_hash init_channels(int size){
	fd_channel_hash hashtable = malloc(sizeof(struct _fd_channel_hash));
	fd_channel_node *table = 
		(fd_channel_node *) calloc(size, sizeof(struct _fd_channel_node));
	hashtable->size = size;
	hashtable->table = table;
	return hashtable;
}

fd_channel_node lookup_channel(char *key){
	int slot; 
	fd_channel_node node;

	/* check if hashtable has been initialized yet */
	if(hashtable == NULL)
		hashtable = init_channels(HASH_SIZE);

	slot = hash(key, hashtable->size);

	/* look through the nodes at this slot */
	for(node = hashtable->table[slot];
			node != NULL && strcmp(node->key, key);
			node = node->next)
		;

	return node;
}

fd_channel_node create_channel(char *key){
	int slot;
	fd_channel_node node = lookup_channel(key);

	slot = hash(key, hashtable->size);

	/* only create a channel if it doesn't already exist */
	if(node == NULL){
		/* set up this channel's node */
		node = malloc(sizeof(struct _fd_channel_node));
		memset(node, 0, sizeof(struct _fd_channel_node));
		node->key = strdup(key);
		node->buffer = malloc(MAX_MESSAGE_LEN);
		memset(node->buffer, 0, MAX_MESSAGE_LEN);

		/* put it in the hash table */
		node->next = hashtable->table[slot];
		hashtable->table[slot] = node;
	}

	return node;
}

/* remove a socket from channel given a channel name and socket id */
void remove_from_channel(char *key, int sock){
  int slot;
  fd_channel_node node = lookup_channel(key);
  fd_channel_watcher current, prev;
  struct ev_loop *loop = EV_DEFAULT;

  if (node != NULL){
    current = node->watchers;
    prev = NULL;

    /* Find the channel watcher matching the given socket id */
    for(current; 
	current != NULL && current->socket->tcp_sock != sock;  
	current = current->next)
      prev = current;
    

    if (current == NULL) {
    	log_file = fopen(LOG_FILE, "a");
    	fprintf(log_file, "ERROR in remove_from_channel: socket with sockid %d was not found in channel with name: %s\n", sock, key);
    	fclose(log_file);
    }

    /* remove watcher from list of watchers in channel */
    else {
    	log_file = fopen(LOG_FILE, "a");
    	fprintf(log_file, "MESSAGE in remove_from_channel: removing sockid %d from channel with name: %s\n", current->socket->tcp_sock, key);
    	fclose(log_file);
    	
      if (prev == NULL) {
	node->watchers = current->next;
      }
      else {
	prev->next = current->next;
      }
      ev_io_stop(loop, &current->w);
      free(current);
    }
  }
  else{
	  
  	log_file = fopen(LOG_FILE, "a");
  	fprintf(log_file, "ERROR in remove_from_channel: node for channel with name %s was null\n", key);
  	fclose(log_file);
	  
  }
}



void close_channel(char *key){
  int slot;
  fd_channel_node node = lookup_channel(key);

  slot = hash(key, hashtable->size);

  if (node == NULL){
	  
	log_file = fopen(LOG_FILE, "a");
	fprintf(log_file, "ERROR in close_channel: the node you are attempting to delete does not exist.\n");
	fclose(log_file);	  
    return;
  }
  else {
    
  } 

}



/* broadcast a message to a channel */
int fd_broadcast(fd_socket_t *socket, char *key, char *buffer, 
								 int msg_type){
	int counter;
	fd_channel_node channel = lookup_channel(key);
	fd_channel_watcher node;
	struct ev_loop *loop = EV_DEFAULT;

	channel->buffer = realloc(channel->buffer, strlen(buffer) + 1);
	fd_strcat(channel->buffer, buffer, 0);

	/* loop through all the watchers in this channel */
	for(counter = 0, node = channel->watchers; node != NULL; 
			counter++, node = node->next){
		/* realloc may have changed this channel's buffer's location */
		node->buffer = channel->buffer;

		/* if it's not ourself, use libev to send the message */
		if(node->socket != socket){
			node->msg_type = msg_type;
			ev_feed_event(loop, &node->w, EV_CUSTOM);
		}
	}

	return counter;
}

/* callback to invoke when there is a message to the channel */
void fd_channel_listener(struct ev_loop *loop, ev_io *w, int revents){
	assert_event(EV_CUSTOM);

	fd_channel_watcher watcher = (fd_channel_watcher) w;
	watcher->cb(watcher->socket, watcher->buffer, watcher->msg_type);
}

void fd_join_channel(fd_socket_t *socket, char *key, 
										 void (*cb)(fd_socket_t *, char *, int)){
	struct ev_loop *loop = EV_DEFAULT;
	fd_channel_node channel = lookup_channel(key);
	fd_channel_watcher watcher = malloc(sizeof(struct _fd_channel_watcher));
	
	/* initialize the watcher */
	watcher->socket = socket;
	watcher->tcp_sock = socket->tcp_sock;
	watcher->msg_type = -1;
	watcher->buffer = channel->buffer;
	watcher->cb = cb;
	
	/* add the watcher to the channel's list of watchers */
	watcher->next = channel->watchers;
	channel->watchers = watcher;

	/* bind the watcher to EV_CUSTOM events */
	ev_io_init(&watcher->w, fd_channel_listener, watcher->tcp_sock, EV_READ);
	ev_io_start(loop, &watcher->w);
}

/* Peter Weinberger's hash function, from the dragon book */
int hash(char *s, int size){
	unsigned h = 0, g;
  char *p;

  for (p = s; *p != '\0'; p++){
		h = (h << 4) + *p;
		if ((g = (h & 0xf0000000)) != 0)
			h ^= (g >> 24) ^ g;
	}

  return h % size;
}
