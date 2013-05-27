lib_LTLIBRARIES = libfd.la
libfd_la_SOURCES = 
	src/base64.c, src/base64.h, src/queue.c, src/queue.h \
	src/fd_channels.c, src/fd_recv.c, src/fd_run.c, src/fd_send.c, \
	src/fd_util.c
libfd_la_LIBADD = 
	src/base64.o, src/queue.o, src/fd_channels.o, \
	src/fd_recv.o, src/fd_run.o, src/fd_send.o, src/fd_util.o
libfd_la_LDFLAGS = -version-info 1:0:0
include_HEADERS = src/fd.h
ACLOCAL_AMFLAGS= -I m4
