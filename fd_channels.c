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

		/* put it in the hash table */
		node->next = hashtable->table[slot];
		hashtable->table[slot] = node;
	}

	return node;
}

/* broadcast a message to a channel */
int fd_broadcast(fd_socket_t *socket, char *key, char *buffer, 
								 int msg_type){
	int counter;
	fd_channel_node channel = lookup_channel(key);
	fd_channel_watcher node;
	struct ev_loop *loop = EV_DEFAULT;

	for(counter = 0, node = channel->watchers; node != NULL; 
			counter++, node = node->next){
		if(node->socket != socket){
			fd_strcat(node->buffer, buffer, 0);
			node->msg_type = msg_type;
			ev_feed_event(loop, &node->w, EV_CUSTOM);
		}
	}

	return counter;
}

/* callback to invoke when there is a message to the channel */
void fd_channel_listener(struct ev_loop *loop, ev_io *w, int revents){
	if(revents & EV_CUSTOM){
		fd_channel_watcher watcher = (fd_channel_watcher) w;
		watcher->cb(watcher->socket, watcher->buffer, watcher->msg_type);
	}
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
	memset(watcher->buffer, 0, MAX_MESSAGE_LEN);
	watcher->cb = cb;
	
	/* add the watcher to the channel's list of watchers */
	watcher->next = channel->watchers;
	channel->watchers = watcher;

	/* bind the watcher to EV_CUSTOM events */
	ev_io_init(&watcher->w, fd_channel_listener, watcher->tcp_sock, EV_READ);
	ev_io_start(loop, &watcher->w);
}

/* void fd_send_to_channel(fd_channel_hash hashtable, char *key, char *buffer){ */
/* 	struct ev_loop *loop = EV_DEFAULT; */
/* 	fd_channel_node node = lookup_channel(hashtable, key); */
/* 	fd_channel_watcher *channel = malloc(sizeof(fd_channel_watcher)); */
 
/* 	strncpy(channel->buffer, buffer, MAX_MESSAGE_LEN); */
/* 	channel->fd = node->fd; */
/* 	channel->addr = node->addr; */

/* 	ev_io_init(&channel->w, fd_send_to_channel_nb, channel->fd, EV_WRITE); */
/* 	ev_io_start(loop, &w); */
/* } */

/* void fd_send_to_channel_nb(struct ev_loop *loop, ev_io *w, int revents){ */
/* 	int status; */
/* 	fd_channel_watcher *channel = (fd_channel_watcher *) w; */

/* 	/\* call sendto once, saving the number of bytes sent *\/ */
/* 	if(channel->bytes_completed < channel->bytes_total){ */
/* 		if((status =  */
/* 				sendto(channel->fd,  */
/* 							 channel->buffer + channel->bytes_completed,  */
/* 							 channel->bytes_total, */
/* 							 0, */
/* 							 (struct sockaddr *) &channel->addr)) < 0){ */
/* 			/\* since we're nonblocking, these are ok *\/ */
/* 			if(errno != EAGAIN && errno != EWOULDBLOCK){ */
/* 				perror(__FILE__); */
/* 				exit(errno); */
/* 			} */
/* 			printf("fd_send_to_channel_nb invoked, but send returned EAGAIN or EWOULDBLOCK\n"); */
/* 		} */

/* 		channel->bytes_completed += status; */
/* 		++channel->calls; */
/* 		printf("Bytes sent in call #%d: %d\n" */
/* 					 "Total bytes_completed: %d\n" */
/* 					 "Total bytes_total: %d\n", */
/* 					 channel->calls, status, */
/* 					 channel->bytes_completed, channel->bytes_total); */
/* 	} */

/* 	if(channel->bytes_completed == channel->bytes_total){ */
/* 		/\* unplug from the event loop *\/ */
/* 		ev_io_stop(loop, &channel->w); */

/* 		free(channel); */

/* 		printf("Done sending to channel\n"); */
/* 	} */
/* } */

/* void add_socket_to_channel(fd_socket *socket, char *channel_name, void *(cb)(int, char *)){ */
/*   int channel_id; */
/*   fd_channel_watcher *new_channel, *current; */
  
/*   struct ev_loop *loop = EV_DEFAULT; */

/*   /\* set up our channel watcher struct *\/ */
/*   new_channel = malloc(sizeof(fd_channel_watcher)); */
/*   memset(new_channel, 0, sizeof(fd_channel_watcher)); */
/*   new_channel->fd_socket = socket; */
/*   new_channel->read_channel_cb = cb; */
/*   strcpy(new_channel->channel_key, channel_name); */


/*   memset(new_channel->buffer, 0, MAX_MESSAGE_LEN); */


/*   /\* add new_channel to list of channels in fd_socket *\/ */
/*   for (current = socket->channel_list; current = current->next ; current != null); */
/*   current = new_channel; */

/*   /\* get id of channel *\/ */
/*   channel_id = lookup_channel(hashtable, channel_name)->fd; */

/*   /\* set up watcher to listen on this channel id *\/ */
/*   ev_io_init(&new_channel->w, new_channel->new_channel_cb, channel_id, EV_READ); */
/*   ev_io_start(loop, &new_channel->w); */


/* } */

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
