#include "fd.h"

/* create and return a blank hash table, given a size */
fd_channel_hash init_channels(int size){
	fd_channel_hash hashtable = malloc(sizeof(struct _fd_channel_hash));
	fd_channel_node table = calloc(size, sizeof(struct _fd_channel_node));
	hashtable->size = size;
	hashtable->table = table;
	return hashtable;
}

fd_channel_node lookup_channel(fd_channel_hash hashtable, char *key){
	int slot = hash(key, hashtable->size);
	fd_channel_node node;

	/* look through the nodes at this slot */
	for(node = hashtable->table[slot];
			node != NULL && !strcmp(node->key, key);
			node = node->next)
		;

	return node;
}

fd_channel_node create_channel(fd_channel_hash hashtable, char *key){
	int fd, flags, slot = hash(key, hashtable->size);
	fd_channel_node node = lookup_channel(hashtable, key);
	
	/* only create a channel if it doesn't already exist */
	if(node == NULL){
		/* create a local socket for this channel */
		if((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
			perror(__FILE__);
			return NULL;
		}
		
		/* call fcntl to set it non blocking */
		flags = fcntl(fd, F_GETFL);
		flags |= O_NONBLOCK;
		if(fcntl(fd, F_SETFL, flags)){
			perror(__FILE__);
			return NULL;
		}

		/* set up this channel's node */
		node = malloc(sizeof(struct _fd_channel_node));
		memset(node, 0, sizeof(struct _fd_channel_node));
		node->key = strdup(key);
		node->fd = fd;
		node->addr.sun_family = AF_UNIX;
		strncpy(node->addr.sun_path, key, sizeof(node->addr.sun_path) - 1);

		/* put it in the hash table */
		node->next = hashtable->table[slot];
		hashtable->table[slot] = node;
	}

	return node;
}

void fd_send_to_channel(fd_channel_hash hashtable, char *key, char *buffer){
	struct ev_loop *loop = EV_DEFAULT;
	fd_channel_node node = lookup_channel(hashtable, key);
	fd_channel_watcher *channel = malloc(sizeof(fd_channel_watcher));
 
	strncpy(channel->buffer, buffer, MAX_MESSAGE_LEN);
	channel->fd = node->fd;
	channel->addr = node->addr;

	ev_io_init(&channel->w, fd_send_to_channel_nb, channel->fd, EV_WRITE);
	ev_io_start(loop, &w);
}

void fd_send_to_channel_nb(struct ev_loop *loop, ev_io *w, int revents){
	int status;
	fd_channel_watcher *channel = (fd_channel_watcher *) w;

	/* call sendto once, saving the number of bytes sent */
	if(channel->bytes_completed < channel->bytes_total){
		if((status = 
				sendto(channel->fd, 
							 channel->buffer + channel->bytes_completed, 
							 channel->bytes_total,
							 0,
							 (struct sockaddr *) &channel->addr)) < 0){
			/* since we're nonblocking, these are ok */
			if(errno != EAGAIN && errno != EWOULDBLOCK){
				perror(__FILE__);
				exit(errno);
			}
			printf("fd_send_to_channel_nb invoked, but send returned EAGAIN or EWOULDBLOCK\n");
		}

		channel->bytes_completed += status;
		++channel->calls;
		printf("Bytes sent in call #%d: %d\n"
					 "Total bytes_completed: %d\n"
					 "Total bytes_total: %d\n",
					 channel->calls, status,
					 channel->bytes_completed, channel->bytes_total);
	}

	if(channel->bytes_completed == channel->bytes_total){
		/* unplug from the event loop */
		ev_io_stop(loop, &channel->w);

		free(channel);

		printf("Done sending to channel\n");
	}
}

/* broadcast a message to a channel */
int fd_broadcast(int channel, char *buffer, int msg_type){
	int counter;
	fd_channel_watcher *node;
	struct ev_loop *loop = EV_DEFAULT;

	for(counter = 0, node = channels[channel]; node != NULL; 
			counter++, node = node->next){
		fd_strcat(node->buffer, buffer, 0);
		node->msg_type = msg_type;
		ev_feed_fd_event(loop, node->tcp_sock, EV_CUSTOM);
	}
}

/* callback to invoke when there is a message to the channel */
void fd_channel_listener(ev_loop *loop, ev_io *w, int revents){
	fd_channel_watcher *watcher = (fd_channel_watcher *) w;
	watcher->callback(socket, watcher->buffer, watcher->msg_type);
}

/* Peter Weinberger's hash function, from the dragon book */
static int hash(char *s, int size){
	unsigned h = 0, g;
  char *p;

  for (p = s; *p != '\0'; p++) {
		h = (h << 4) + *p;
		if ((g = (h & 0xf0000000)) != 0)
			h ^= (g >> 24) ^ g;
	}

  return h % size;
}
