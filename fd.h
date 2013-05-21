#ifndef _FD_H
#define _FD_H

/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include "base64.h"

/* libev */
#ifndef EV_STANDALONE
#define EV_STANDALONE 1
#include "ev.h"
#endif

/* defines */
#define LISTENQ 20 /*maximum number of client connections*/
#define HEADERKEY "Sec-WebSocket-Key"
#define MAGICSTRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define MIN_HEADER_LEN 6
#define MAX_HEADER_LEN 14
#define MAX_MESSAGE_LEN 4096
#define PAYLOAD_EXT_16 126
#define PAYLOAD_EXT_64 127
#define HASH_SIZE 256

/* structs */
typedef struct _fd_socket_t fd_socket_t;
struct _fd_socket_t {
	ev_io read_w;
	ev_io write_w;
	int	tcp_sock;
	uint64_t bytes_expected;
	uint64_t bytes_received;
	uint64_t bytes_outgoing;
	uint64_t bytes_sent;
	int header_len;
	int fin;
	int opcode;
	uint32_t mask_key;
	int mask_start;
	char buffer[MAX_HEADER_LEN + MAX_MESSAGE_LEN];
	char out_buffer[MAX_HEADER_LEN + MAX_MESSAGE_LEN];
	unsigned int last_recv_opcode;
	bool is_open;
  int just_opened;
  int recvs;
	int sends;
	int event;
	void *data;
	void (*accept_cb)(fd_socket_t *socket);
	void (*data_cb)(fd_socket_t *socket, char *buffer);
	void (*end_cb)(fd_socket_t *socket);
};

typedef struct _fd_channel_watcher *fd_channel_watcher;
struct _fd_channel_watcher {
	ev_io w;
	fd_socket_t *socket;
	int tcp_sock;
	int msg_type;
	char *buffer;
	void (*cb)(fd_socket_t *socket, char *buffer, int msg_type);
	fd_channel_watcher next;
};

typedef struct _fd_channel_node *fd_channel_node;
struct _fd_channel_node {
	fd_channel_watcher watchers;
	char *key;
	char buffer[MAX_MESSAGE_LEN];
	fd_channel_node next;
};

typedef struct _fd_channel_hash *fd_channel_hash;
struct _fd_channel_hash {
	int size;
	fd_channel_node *table;
};

/* enum our own custom event types */
enum EVENT {
	FD_READ,
	FD_WRITE
};

/* enum the opcodes for the data framing */
enum OPCODE { 
	CONTINUATION,
	TEXT,
	BINARY,
	NC1,
	NC2,
	NC3,
	NC4,
	NC5,
	CONNECTION_CLOSE,
	PING,
	PONG,
	CF1,
	CF2,
	CF3,
	CF4,
	CF5,
	OPEN,
};

/* macros */
#define wtos(w,m)																						\
	(fd_socket_t *) (((char *) w) - offsetof(fd_socket_t, m))
#define assert_event(e)												\
	if(!(revents & e))													\
		return


/* global variables */
fd_channel_hash hashtable;

/* handshaking function definitions */
int handshake(int);
char *base64_encode(const unsigned char *, size_t, size_t *);
unsigned char *base64_decode(const char *, size_t, size_t *);
void build_decoding_table(void);
void base64_cleanup(void);

/* callback functions */
void accept_callback(struct ev_loop *, ev_io *, int);
void handshake_callback_r(struct ev_loop *, ev_io *, int);
void handshake_callback_w(struct ev_loop *, ev_io *, int);
void client_callback_r(struct ev_loop *, ev_io *, int);
void client_callback_w(struct ev_loop *, ev_io *, int);
void fd_recv_nb(struct ev_loop *, ev_io *, int);
void fd_send_nb(struct ev_loop *, ev_io *, int);

/* channel function definitions */
fd_channel_hash init_channels(int );
fd_channel_node lookup_channel(char *);
fd_channel_node create_channel(char *);
int fd_broadcast(fd_socket_t *, char *, char *, int);
void fd_channel_listener(struct ev_loop *, ev_io *, int);
void fd_join_channel(fd_socket_t *, char *, 
										 void (*cb)(fd_socket_t *, char *, int));
int hash(char *s, int size);

/* firedrake function definitions */
int fd_ondata(fd_socket_t *, void(*)(char *));
int fd_run(int, void(*)(fd_socket_t *));
int fd_send(fd_socket_t *, char *, int);
void fd_strcat(char *, char *, int);
int fd_recv(fd_socket_t *, char *);
fd_socket_t *fd_socket_new(void);
void fd_socket_destroy(fd_socket_t *, struct ev_loop *);
int fd_socket_close(fd_socket_t *);
void fd_close(struct ev_loop *, ev_signal *, int);

#endif

