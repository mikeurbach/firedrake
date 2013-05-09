CC = gcc
CFLAGS = -Wall -g -std=gnu99 -pedantic -Ilibev

all: ev

obj_ev: server_ev.c fd_run.c base64.c fd_send_ev.c fd_recv_ev.c fd_channels.c fd_util.c fd.h 
	$(CC) -c $(CFLAGS) -o fd_run.o fd_run.c
	$(CC) -c $(CFLAGS) -o base64.o base64.c
	$(CC) -c $(CFLAGS) -o fd_util.o fd_util.c
	$(CC) -c $(CFLAGS) -o fd_send.o fd_send_ev.c
	$(CC) -c $(CFLAGS) -o fd_recv.o fd_recv_ev.c
	$(CC) -c $(CFLAGS) -o fd_channels.o fd_channels.c
	$(CC) -c $(CFLAGS) -o server.o server_ev.c

obj_legacy: fd.h
	$(CC) -c $(CFLAGS) -o base64.o base64.c
	$(CC) -c $(CFLAGS) -o fd_send.o fd_send_legacy.c
	$(CC) -c $(CFLAGS) -o fd_recv.o fd_recv_legacy.c
	$(CC) -c $(CFLAGS) -o server.o server_legacy.c

obj_client: fd.h client.c
	$(CC) -c $(CFLAGS) -o base64.o base64.c
	$(CC) -c $(CFLAGS) -o client.o client.c

ev: obj_ev
	$(CC) $(CFLAGS) -o server server.o fd_util.o fd_run.o fd_send.o base64.o fd_recv.o fd_channels.o -lpthread -lssl -lcrypto

legacy: obj_legacy
	$(CC) $(CFLAGS) -o server server.o fd_send.o base64.o fd_recv.o -lpthread -lssl -lcrypto

client: obj_client
	$(CC) $(CFLAGS) -o client client.o base64.o  -lcrypto

clean:
	rm -f server client *.o
	rm -f *~ *#
