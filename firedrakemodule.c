
#include "fd.h"
#include <Python.h>
#include <structmember.h>

/* 
	 Macros
 */
#define DATA  1
#define END   2
#define CLOSE 4

/* 
	 Helper functions 
*/
int event_type(const char *event){
	if(strcmp(event,"data") == 0)									
		return DATA;																				
	else if(strcmp(event,"end") == 0)							
		return END;																				
	else if(strcmp(event,"close") == 0)						
		return CLOSE;																			
	else																					
		return -1;
}

/* 
	 Socket attribute declarations
 */

/* the socket struct */
typedef struct {
	PyObject_HEAD
	int id;         							/* unique id used in lib */
	PyObject *ondata;							/* data event callback */
	PyObject *onend;							/* end event callback */
	PyObject *onclose;						/* close event callback */
	PyObject *data;								/* private user data */
} Socket;

/* expose its members */
static PyMemberDef Socket_members[] = {
	{"id", T_INT, offsetof(Socket, id), 0, "unique id in libfd"},
	{"ondata", T_OBJECT_EX, offsetof(Socket, ondata), 0, "data cb"},
	{"onend", T_OBJECT_EX, offsetof(Socket, onend), 0, "end cb"},
	{"onclose", T_OBJECT_EX, offsetof(Socket, onclose), 0, "close cb"},
	{"data", T_OBJECT_EX, offsetof(Socket, data), 0, "user data"},
	{NULL, 0, 0, NULL}
};

/* 
	 Socket method declarations and definitions
 */

/* initialize the socket */
static int Socket_init(Socket *self, PyObject *args, PyObject *kwds){
	/* int, optionally private user data */
	if(!PyArg_ParseTuple(args, "i", &self->id)){
		PyErr_Print();
		return -1;
	}
	
	return 0;
}

/* free the socket */
static void Socket_dealloc(Socket *self){
	Py_XDECREF(self->ondata);
	Py_XDECREF(self->onend);
	Py_XDECREF(self->onclose);
	Py_XDECREF(self->data);
	self->ob_type->tp_free((PyObject *) self);
}

/* bind a callback to a socket */
static PyObject *Socket_on(Socket *self, PyObject *args){
	const char *event;
	PyObject *callback;

	if(!PyArg_ParseTuple(args, "sO:on", &event, &callback))
		return NULL;

	if(!PyCallable_Check(callback)){
		PyErr_SetString(PyExc_TypeError, "second parameter must be callable");
		return NULL;
	}

	Py_XINCREF(callback);

	/* assign the appropriate callback */
	switch(event_type(event)){
	case DATA:
		Py_XDECREF(self->ondata);
		self->ondata = callback;
		break;
	case END:
		Py_XDECREF(self->onend);
		self->onend = callback;
		break;
	case CLOSE:
		Py_XDECREF(self->onclose);
		self->onclose = callback;
		break;
	default:
		Py_XDECREF(callback);
		PyErr_SetString(PyExc_ValueError, "unknown event name");
		NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

/* send a message to the socket's client */
static PyObject *Socket_send(Socket *self, PyObject *args){
	fd_socket_t *socket;
	const char *buffer;
	int ret;
	
	if(!PyArg_ParseTuple(args, "s", &buffer))
		return NULL;
	
	/* find our socket */
	socket = fd_lookup_socket(self->id);
	if(!socket){
		PyErr_SetString(PyExc_ValueError, "socket lookup failed");
		return NULL;
	}

	/* call fd_send */
	if((ret = fd_send(socket, buffer, TEXT))){
		PyErr_SetString(PyExc_ValueError, "fd_send failed");
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


/* expose its methods */
static PyMethodDef Socket_methods[] = {
	{"on", (PyCFunction) Socket_on, METH_VARARGS, "bind a callback"},
	{"send", (PyCFunction) Socket_send, METH_VARARGS, "send a message"},
	{NULL, NULL, 0, NULL}
};

/* the socket type */
static PyTypeObject SocketType = {
	PyObject_HEAD_INIT(NULL)
	0,
	"firedrake.Socket",
	sizeof(Socket),
	0,                         
	(destructor) Socket_dealloc,                         
	0,                         
	0,                         
	0,                         
	0,                         
	0,                         
	0,                         
	0,                         
	0,                         
	0,                         
	0,                         
	0,                         
	0,                         
	0,                         
	0,                         
	Py_TPFLAGS_DEFAULT,        
	"Socket object",           
	0,               
	0,               
	0,               
	0,               
	0,               
	0,               
	Socket_methods,             
	Socket_members,             
	0,                         
	0,                         
	0,                         
	0,                         
	0,                         
	0,                         
	(initproc) Socket_init,      
	0,                         
	0,                 
};

/* setup the accept callback for python */
int py_init_socket(void(*cb)(fd_socket_t *), int id){
	PyObject *callback = (PyObject *) cb;														
	PyObject *py_socket;																						
	PyObject *args = Py_BuildValue("(i)", id);
  fd_socket_t *socket;																						
																																	
	/* create the Socket object */																	
	py_socket = PyObject_CallObject((PyObject *) &SocketType, args);
	if(!py_socket){
		PyErr_Print();
		return -1;
	}
	Py_DECREF(args);
																																	
	/* call the accept cb */																				
	args = Py_BuildValue("(O)", py_socket);															
	if(!PyObject_CallObject(callback, args)){												
		PyErr_Print();
		return -1;																										
	}																																
	Py_DECREF(args);																								
																																	
  /* attach the Socket object to the fd_socket_t struct */				
	socket = fd_lookup_socket(id);
	socket->py_socket = py_socket;

	return 0;
}																			

/* invoke a user function on a fd_socket_t and a buffer */
int py_call_fs(fd_socket_t *socket, char *buffer, int type){
	Socket *py_socket = (Socket *) socket->py_socket;
	PyObject *args;

	/* set up the python args */
	args = Py_BuildValue("Os", (PyObject *) py_socket, buffer);

	/* call the user function on said args */
	switch(type){
	case FD_DATA:
		if(!PyCallable_Check(py_socket->ondata)){
			PyErr_Print();
			return -1;
		}
		if(!PyObject_CallObject(py_socket->ondata, args)){
			PyErr_Print();
			return -1;
		}
		break;
	case FD_END:
		if(!PyCallable_Check(py_socket->onend)){
			PyErr_Print();
			return -1;
		}
		if(!PyObject_CallObject(py_socket->onend, args)){
			PyErr_Print();
			return -1;
		}
		break;
	case FD_CLOSE:
		if(!PyCallable_Check(py_socket->onclose)){
			PyErr_Print();
			return -1;
		}
		if(!PyObject_CallObject(py_socket->onclose, args)){
			PyErr_Print();
			return -1;
		}
		break;
	default:
		break;
	}

	return 0;
}

/* 
	 Module level definitions
 */
static PyObject *firedrake_run(PyObject *self, PyObject *args){
	int port;
	PyObject *accept_cb;

	if(!PyArg_ParseTuple(args, "iO", &port, &accept_cb))
		return NULL;
	
	if(!PyCallable_Check(accept_cb))
		return NULL;

	Py_XINCREF(accept_cb);

	if(fd_run(port, (void *) accept_cb)){
		PyErr_SetString(PyExc_ValueError, "fd_run failed");
		return NULL;
	}
	
	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef FiredrakeMethods[] = {
	{"run", firedrake_run, METH_VARARGS, "start the websocket server"},
	{NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC initfiredrake(void){
	PyObject *m;

	SocketType.tp_new = PyType_GenericNew;
	if(PyType_Ready(&SocketType) < 0)
		return;

	m = Py_InitModule3("firedrake", FiredrakeMethods,
										 "fast websocket server library");

	Py_INCREF(&SocketType);
	PyModule_AddObject(m, "Socket", (PyObject *) &SocketType);
}

