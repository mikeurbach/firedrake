lib_LTLIBRARIES = libfd.la
libfd_la_SOURCES = src/base64.c src/base64.h src/queue.c src/queue.h src/fd_channels.c src/fd_recv.c src/fd_run.c src/fd_send.c	src/fd_util.c
libfd_la_LDFLAGS = -version-info 1:0:0
include_HEADERS = src/fd.h src/base64.h src/queue.h
ACLOCAL_AMFLAGS= -I m4
AM_CFLAGS= -g -std=gnu99 -Wall -pedantic -lpthread -lssl -lcrypto -I../../src/libev -I../src/libev -I./src/libev -Ilibev -fPIC
MOSTLYCLEANFILES= src/*.o src/*.lo
include $(top_srcdir)/src/libev