/*
 * chat_client.h
 *
 *  Created on: Apr 15, 2012
 *      Author: Grim
 */

#ifndef CHAT_CLIENT_H_
#define CHAT_CLIENT_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ncurses.h>
#include "macros.h"

#define MAXLINE 4096 /*max text line length*/
#define NAMELEN 15 /*max username length*/
#define SERV_PORT 8080 /*port*/

#endif /* CHAT_CLIENT_H_ */
