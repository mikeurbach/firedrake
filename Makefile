CC = gcc
CFLAGS = -Wall -g -std=gnu99 -pedantic -Ilibev

all: fd_run

obj: server2.c fd_run.c base64.c fd_send2.c fd_recv.c fd.h 
	$(CC) -c $(CFLAGS) -o fd_run.o fd_run.c
	$(CC) -c $(CFLAGS) -o base64.o base64.c
	$(CC) -c $(CFLAGS) -o fd_send.o fd_send2.c
	$(CC) -c $(CFLAGS) -o fd_recv.o fd_recv.c
	$(CC) -c $(CFLAGS) -o server.o server2.c

fd_run: obj
	$(CC) $(CFLAGS) -o server server.o fd_run.o fd_send.o base64.o fd_recv.o -lpthread -lssl -lcrypto

clean:
	rm -f fd_run *.o
	rm -f *~ *#
