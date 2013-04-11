#ifndef _FD_H
#define _FD_H

/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include "base64.h"

/* defines */
#define LISTENQ 20 /*maximum number of client connections*/
#define HEADERKEY "Sec-WebSocket-Key"
#define MAGICSTRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define MAX_HEADER_LEN 10
#define MAX_MESSAGE_LEN 100000
#define PAYLOAD_EXT_16 126
#define PAYLOAD_EXT_64 127

/* structs */
typedef struct _fd_socket_t {
	int	tcp_sock;
} fd_socket_t;

/* function definitions */
int handshake(int);
char *base64_encode(const unsigned char *, size_t, size_t *);
unsigned char *base64_decode(const char *, size_t, size_t *);
void build_decoding_table(void);
void base64_cleanup(void);
int fd_send(fd_socket_t *, char *);

#endif

