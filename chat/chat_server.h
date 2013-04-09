/*
 * chat_server.h
 *
 *  Created on: Apr 15, 2012
 *      Author: Grim
 */

#ifndef CHAT_SERVER_H_
#define CHAT_SERVER_H_


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "macros.h"

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 8080 /*port*/
#define NAMELEN 15 /*max username length*/
#define LISTENQ 20 /*maximum number of client connections*/


typedef struct client Client;
struct client{
	int		id,
			sockfd;
	char	uname[NAMELEN];

};

#endif /* CHAT_SERVER_H_ */
