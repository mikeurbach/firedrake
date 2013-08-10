CC = gcc
CFLAGS = -g -std=gnu99 -Ilibev -Wall -pedantic -fPIC

all: obj
	$(CC) -shared $(CFLAGS) -o libfiredrake.so fd_run.o fd_send.o fd_recv.o fd_channels.o fd_util.o base64.o queue.o -lpthread -lssl -lcrypto

obj: fd_run.c fd_send.c fd_recv.c fd_channels.c fd_util.c base64.c queue.c fd.h 
	$(CC) -c $(CFLAGS) -o fd_run.o fd_run.c
	$(CC) -c $(CFLAGS) -o fd_send.o fd_send.c
	$(CC) -c $(CFLAGS) -o fd_recv.o fd_recv.c
	$(CC) -c $(CFLAGS) -o fd_channels.o fd_channels.c
	$(CC) -c $(CFLAGS) -o fd_util.o fd_util.c
	$(CC) -c $(CFLAGS) -o base64.o base64.c
	$(CC) -c $(CFLAGS) -o queue.o queue.c

clean:
	rm -f *.o *.so
	rm -f *~ *#
	rm -f log.txt
	rm -f echo chat
