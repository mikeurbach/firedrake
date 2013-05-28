/* 
    firedrake: an efficient library for writing websocket servers

    Copyright (C) 2013 Mike Urbach, Matt Diephui, Drew Jankowski

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

		Report bugs to <mikeurbach@gmail.com>
*/

#include <stdlib.h>

#include "./queue.h"

/* each node in the queue will simply */
/* point to the next node */
typedef struct {
  void *item;
  void *next;
} qitem;

/* create an empty queue */
public void* qopen(void) {
  qitem *qp;
  qp = malloc(sizeof(qitem));
  qp->item = NULL;
  qp->next = NULL;
  return (void *) qp;
}

/* deallocate a queue, assuming every element has been removed and deallocated */
public void qclose(void *qp) {
  free(qp);
}

/* put element at end of queue */
public void qput(void *qp, void *elementp) {
  qitem *ip; /* get a pointer for this item */

  /* create a new qitem */
  ip = malloc(sizeof(qitem));
  ip->item = elementp;
  ip->next = NULL;

  /* seek to the end of the queue 
     qp will point to the last added item */
  while(((qitem *) qp)->next != NULL) qp = ((qitem *) qp)->next;
  
  /* add the new item */
  ((qitem *) qp)->next = ip;
}

/* get first element from a queue */
public void* qget(void *qp) {
  /* cut the first qitem out of the queue, if not empty */
  if(((qitem *) qp)->next == NULL){
    /* empty queue */
    return NULL;
  } else {
    /* not empty queue */
    qitem *ip; 
    void *item;

    /* cut out the first item */
    ip = ((qitem *) qp)->next;
    ((qitem *) qp)->next = ip->next;

    /* return the first qitem's item pointer */
    item = ip->item;
    free(ip);
    return item;
  }
}

/* apply a void function (e.g. a printing fn) to every element of a queue */
public void qapply(void *qp, void (*fn)(void* elementp)) {
  /* run down the queue, if we have one */
  while(((qitem *) qp)->next != NULL) {
    qp = ((qitem *) qp)->next;
    (*fn)((void *) (((qitem *) qp)->item));
  }
}

/* /\* search a queue using a supplied boolean function, returns an element *\/ */
/* public void* qsearch(void *qp,  */
/* 		     int (*searchfn)(void* elementp,void* keyp), */
/* 		     void* skeyp); */

/* /\* search a queue using a supplied boolean function, removes an element *\/ */
/* public void* qremove(void *qp, */
/* 		     int (*searchfn)(void* elementp,void* keyp), */
/* 		     void* skeyp); */

/* /\* concatenatenates q2 onto q1, q2 may not be subsequently used *\/ */
/* public void qconcat(void *q1p, void *q2p); */


