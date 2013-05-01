/* 

   fd_channels.c
   implement channel interface

 */


#include "fd.h"

void add_socket_to_channel(fd_socket *socket, char *channel_name, void *(cb)(int, char *)){
  int channel_id;
  fd_channel_watcher *new_channel, *current;
  
  struct ev_loop *loop = EV_DEFAULT;

  /* set up our channel watcher struct */
  new_channel = malloc(sizeof(fd_channel_watcher));
  memset(new_channel, 0, sizeof(fd_channel_watcher));
  new_channel->fd_socket = socket;
  new_channel->read_channel_cb = cb;
  strcpy(new_channel->channel_key, channel_name);


  memset(new_channel->buffer, 0, MAX_MESSAGE_LEN);


  /* add new_channel to list of channels in fd_socket */
  for (current = socket->channel_list; current = current->next ; current != null);
  current = new_channel;

  /* get id of channel */
  channel_id = lookup_channel(hashtable, channel_name)->fd;

  /* set up watcher to listen on this channel id */
  ev_io_init(&new_channel->w, new_channel->new_channel_cb, channel_id, EV_READ);
  ev_io_start(loop, &new_channel->w);


}

