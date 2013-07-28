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

#define public
#define private static

/* create an empty queue */
public void* qopen(void);        

/* deallocate a queue, assuming every element has been removed and deallocated */
public void qclose(void *qp);   

/* put element at end of queue */
public void qput(void *qp, void *elementp); 

/* get first element from a queue */
public void* qget(void *qp);

/* apply a void function (e.g. a printing fn) to every element of a queue */
public void qapply(void *qp, void (*fn)(void* elementp));

/* search a queue using a supplied boolean function, returns an element */
public void* qsearch(void *qp, 
		     int (*searchfn)(void* elementp,void* keyp),
		     void* skeyp);

/* search a queue using a supplied boolean function, removes an element */
public void* qremove(void *qp,
		     int (*searchfn)(void* elementp,void* keyp),
		     void* skeyp);

/* concatenatenates q2 onto q1, q2 may not be subsequently used */
public void qconcat(void *q1p, void *q2p);


