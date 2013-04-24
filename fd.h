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
#define MAX_MESSAGE_LEN 100000
#define PAYLOAD_EXT_16 126
#define PAYLOAD_EXT_64 127

/* structs */
typedef struct _fd_socket_t {
	ev_io io;
	int	tcp_sock;
	uint64_t bytes_expected;
	uint64_t bytes_received;
	int header_len;
	uint32_t mask_key;
	char buffer[MAX_HEADER_LEN + MAX_MESSAGE_LEN];
	unsigned int last_recv_opcode;
	bool is_open;
	int event;
} fd_socket_t;

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

/* firedrake function definitions */
int fd_run(int);
int fd_send(fd_socket_t *, char *, int opcode);
void fd_strcat(char *, char *, int);
int fd_recv(fd_socket_t *, char *);
fd_socket_t *fd_socket_new(void);
void fd_socket_destroy(fd_socket_t *sock);
int fd_socket_close(fd_socket_t *sock);


#endif

