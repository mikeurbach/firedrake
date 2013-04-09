/*
 * macros.h
 *
 *  Created on: Apr 15, 2012
 *      Author: Grim
 *
 * Macros file that defines some generally useful macros for use in any c file*/

#ifndef MACROS_H_
#define MACROS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SIGNON 1 /*signon protocol message*/
#define SIGNON_SUCCESS 1 /*signon status signal*/
#define SIGNON_FAILURE 0 /*signong status signal*/
#define SIGNOFF 2 /*signoff protocol message*/
#define STATUS 3 /*status protocol message*/
#define CHAT 4 /*chat protocol message*/
#define ASEC 1 /*one second*/
#define SERVADDR "127.0.0.1"

//Fatal error macro
#define FATAL(s) { fprintf(stderr, "EXITING:  Fatal error at [%s:%d]: %s\n", __FILE__, __LINE__, s); exit(EXIT_FAILURE); }

//Mallocs and error checks automatically for a single 'object' allocation
#define NEW(s, t) if(((s) = (t *) malloc(sizeof(t))) == NULL) { FATAL("malloc() returned NULL"); }

//Similar to NEW() except for multiple 'objects'
#define NEW_N(s, t, n) if(((s) = (t *) malloc(sizeof(t) * n)) == NULL) {FATAL("malloc() returned NULL");}

#endif
