/*
 * chat_server.h
 *
 *  Created on: Apr 15, 2012
 *      Author: Grim
 */

#ifndef SERVER_H_
#define SERVER_H_


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "macros.h"

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 8080 /*port*/
#define NAMELEN 15 /*max username length*/
#define LISTENQ 20 /*maximum number of client connections*/
#define HEADERKEY "Sec-WebSocket-Key"
#define MAGICSTRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

typedef struct client Client;
struct client{
	int		id,	sockfd;
	char	uname[NAMELEN];
};

#endif /* CHAT_SERVER_H_ */
