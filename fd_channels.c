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
	if(channel_hashtable == NULL)
		channel_hashtable = init_channels(HASH_SIZE);

	slot = hash(key, channel_hashtable->size);

	/* look through the nodes at this slot */
	for(node = channel_hashtable->table[slot];
			node != NULL && strcmp(node->key, key);
			node = node->next)
		;

	return node;
}

fd_channel_node create_channel(char *key){
  int slot;
  fd_channel_node node = lookup_channel(key);
  
  slot = hash(key, channel_hashtable->size);
  
  /* only create a channel if it doesn't already exist */
  if(node == NULL){
    /* set up this channel's node */
    node = malloc(sizeof(struct _fd_channel_node));
    memset(node, 0, sizeof(struct _fd_channel_node));
    node->key = strdup(key);
    
    /* put it in the hash table */
    node->next = channel_hashtable->table[slot];
    channel_hashtable->table[slot] = node;
  }
  else
    fd_log_w("Channel with name %s already exists\n", key);
  
  return node;
}


void fd_remove_from_all_channels(int sockid){
  fd_socket_t *socket = fd_lookup_socket(sockid);
  fd_channel_name *current = socket->__internal.channel_list;

  for (current; current != NULL; current = current->next)
    remove_from_channel(current->key, sockid);

}


/* remove a socket from channel given a channel name and socket id */
void remove_from_channel(char *key, int sock){
  fd_channel_node node = lookup_channel(key);
  fd_channel_watcher current, prev;
  struct ev_loop *loop = EV_DEFAULT;
  
  if (node != NULL){
    current = node->watchers;
    prev = NULL;
    
    /* Find the channel watcher matching the given socket id */
    for(current; 
	current != NULL && current->socket->id != sock;  
	current = current->next)
      prev = current;
    
    
    if (current == NULL) {
      fd_log_e("socket with sockid %d was not found in channel with name"
	       " %s\n", sock, key);
    }
    /* remove watcher from list of watchers in channel */
    else {
      fd_log_i("removing sockid %d from channel with name: %s\n", 
	       current->socket->id, key);
      
      if (prev == NULL) {
	node->watchers = current->next;
      }
      else {
	prev->next = current->next;
      }
      
      /* remove this channel from socket's list of channels */
      remove_channel_from_sock_list(current->socket, key);   
      
      free(current);
    }
  }
  else{
    fd_log_e("node for channel with name %s was null\n", key);
  }
}


void remove_channel_from_sock_list(fd_socket_t *socket, char *key){
  fd_channel_name *current, *prev;

  current = socket->__internal.channel_list;
  prev = NULL;

  /* Find the channel name node matching the key passed in */
  for(current; 
      current != NULL && strcmp(current->key, key);
      current = current->next)
    prev = current;

  if (current == NULL) {
    fd_log_w("channel name %s was not found in socket #%d's "
						 "channel list\n", key, socket->tcp_sock);
  }
  /* remove this channel node from list of channels */
  else {
    fd_log_i("removing channel %s from socket %d's channel list\n", 
						 current->key, socket->tcp_sock);
    if (prev == NULL) {
      socket->__internal.channel_list = current->next;
    }
    else {
      prev->next = current->next;
    }
  
    /* free the channel name struct */
    //    free(current->key);
    free(current);

  }
}

void fd_close_channel(char *key){
  int slot = hash(key, channel_hashtable->size);
  fd_channel_node node = lookup_channel(key);
  fd_channel_watcher current, prev;
  struct ev_loop *loop = EV_DEFAULT;

  fd_log_i("attempting to close channel %s\n", key);
  
  if (node == NULL){
    fd_log_e("the node you are attempting to delete does not exist.\n");
    return;
  }
  else {
    if (node->watchers == NULL){
      fd_log_w("no current watchers attached to channel %s\n", key);
    } else {
      prev = node->watchers;
      current = prev->next;
      
      /* go through watchers, remove each from list */
      for (current; current != NULL; current = current->next){
	remove_channel_from_sock_list(prev->socket, key);
	
	free(prev);
	prev = current;
      }
      
      /* remove final node after current is null */
      remove_channel_from_sock_list(prev->socket, key);
      free(prev);
    }
    
    /* now release the channel node */
    fd_channel_node pr, ch = channel_hashtable->table[slot];
    pr = NULL;
    
    for (ch;
	 ch != NULL && strcmp(ch->key, node->key);
	 ch = ch->next)
      pr = ch;
    
    /* if prev is null, channel node is first in slot  */
    if (pr == NULL)
      channel_hashtable->table[slot] = ch->next;
    else
      pr->next = node->next;
    fd_log_i("removed channel %s node from hashtable\n", node->key);
    //    free(node->buffer);
    // free(node->key);
    free(node);
    
  }
} 

void close_all_channels(){
  fd_channel_node current, prev;
  for(int i = 0; i < channel_hashtable->size; i++){
    current = channel_hashtable->table[i];

    /* if slot in hashtable has channel node, remove all channels */
    if (current != NULL){
      prev = current;
      current = current->next;
      for (current; current != NULL; current = current->next){
	fd_close_channel(prev->key);
	prev = current;
      }
      fd_close_channel(prev->key);
    }    
  }

  /* now free the dictionary itself */
  free(channel_hashtable->table);
  free(channel_hashtable);

}

/* broadcast a message to a channel */
int fd_broadcast(fd_socket_t *socket, char *key, char *buffer, 
		 int msg_type){
  int counter;
  fd_channel_node channel = lookup_channel(key);
  fd_channel_watcher node;

  if(!channel){
    fd_log_e("socket with id %d attempted to broadcast to a non-existant channel\n", socket->id);
    return -1;
  }
  
  /* loop through all the watchers in this channel */
  for(counter = 0, node = channel->watchers; node != NULL; 
      counter++, node = node->next){
    
    /* if it's not ourself, call the callback on the buffer */
    if(node->socket != socket){
      node->cb(node->socket, buffer, msg_type);
    }
  }
  
  return counter;
}

void fd_join_channel(fd_socket_t *socket, char *key, 
		     void (*cb)(fd_socket_t *, char *, int)){
  fd_channel_node channel = lookup_channel(key);
  fd_channel_watcher watcher = malloc(sizeof(struct _fd_channel_watcher));
  
  if(!channel){
    fd_log_e("socket with id %d attempted to broadcast to a non-existant channel\n", socket->id);
    return;
  }
  
  /* initialize the watcher */
  watcher->socket = socket;
  watcher->cb = cb;
  
  /* add the watcher to the channel's list of watchers */
  watcher->next = channel->watchers;
  channel->watchers = watcher;
  
  /* add channel to list of channels for this socket */
  fd_channel_name *new_channel = malloc(sizeof(fd_channel_name));
  new_channel->key = strdup(key);
  new_channel->next = socket->__internal.channel_list;
  socket->__internal.channel_list = new_channel;
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
