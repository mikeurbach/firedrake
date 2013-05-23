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
#define LOG_FILE "log.fd"
#define LISTENQ 20 /*maximum number of client connections*/
#define HEADERKEY "Sec-WebSocket-Key"
#define MAGICSTRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define MIN_HEADER_LEN 6
#define MAX_HEADER_LEN 14
#define MAX_MESSAGE_LEN 4096
#define PAYLOAD_EXT_16 126
#define PAYLOAD_EXT_64 127
#define HASH_SIZE 256

/* logging macros */

/* open logging file */
#define fd_log_setup {log_file = fopen(LOG_FILE,"a");}

/*close logging file*/
#define fd_log_close {fclose(log_file);}

/* debug log */
#define fd_log_d(...) {fprintf(log_file, "[DEBUG] in [%s:%d]: ", __FILE__, __LINE__); fprintf(log_file, __VA_ARGS__);}

/* info log */
#define fd_log_i(...) {fprintf(log_file, "[INFO] in [%s:%d]: ", __FILE__, __LINE__); fprintf(log_file, __VA_ARGS__);}

/* message log */
#define fd_log_m(...) {fprintf(log_file, "[MESSAGE] in [%s:%d]: ", __FILE__, __LINE__); fprintf(log_file, __VA_ARGS__);}	

/* warning log */  	  	  	  	  	  
#define fd_log_w(...) {fprintf(log_file, "[WARNING] in [%s:%d]: ", __FILE__, __LINE__); fprintf(log_file, __VA_ARGS__);}

/* critical log */  	  	  	  	  	  
#define fd_log_c(...) {fprintf(log_file, "[CRITICAL] in [%s:%d]: ", __FILE__, __LINE__); fprintf(log_file, __VA_ARGS__);}
  	  	  	  	  	  
/* error log */
#define fd_log_e(...) {fprintf(log_file, "[ERROR] in [%s:%d]: ", __FILE__, __LINE__); fprintf(log_file, __VA_ARGS__);}	  	  	  	  	  

/* structs */
typedef struct _fd_channel_name fd_channel_name;
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
	char *buffer;
	char *out_buffer;
	unsigned int last_recv_opcode;
	bool is_open;
  int just_opened;
  int recvs;
	int sends;
	int event;
	void *data;
	void (*accept_cb)(fd_socket_t *socket);
	void (*data_cb)(fd_socket_t *socket, char *buffer);
  fd_socket_t *next;
  fd_channel_name *channel_list;
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
	char *buffer;
	fd_channel_node next;
};


struct _fd_channel_name {
  char *key;
  fd_channel_name *next;
};

typedef struct _fd_channel_hash *fd_channel_hash;
struct _fd_channel_hash {
	int size;
	fd_channel_node *table;
};

typedef struct _fd_socket_hash *fd_socket_hash;
struct _fd_socket_hash {
	int size;
	fd_socket_t **table;
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

fd_channel_hash channel_hashtable;
fd_socket_hash socket_hashtable;

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
void remove_from_channel(char *, int);
int fd_broadcast(fd_socket_t *, char *, char *, int);
void fd_channel_listener(struct ev_loop *, ev_io *, int);
void remove_from_all_channels(int);
void fd_join_channel(fd_socket_t *, char *, void (*cb)(fd_socket_t *, char *, int));
int hash(char *, int);
void remove_channel_from_sock_list(fd_socket_t *, char *);
void fd_close_channel(char *);
void close_all_channels();
int hash(char *s, int size);


/* firedrake function definitions */
int fd_ondata(fd_socket_t *, void(*)(char *));
int fd_run(int, void(*)(fd_socket_t *));
int fd_send(fd_socket_t *, char *, int);
void fd_strcat(char *, char *, int);
int fd_recv(fd_socket_t *, char *);

/* util function definitions */
fd_socket_t *fd_socket_new(void);
void fd_socket_destroy(fd_socket_t *, struct ev_loop *);
int fd_socket_close(fd_socket_t *);
void fd_close(struct ev_loop *, ev_signal *, int);
void add_sock_to_hashtable(fd_socket_t *);
void remove_sock_from_hashtable(fd_socket_t *);
fd_socket_t *fd_lookup_socket(int);
fd_socket_hash init_socket_hashtable(int);


#endif

