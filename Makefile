
CC = gcc
CFLAGS = -Wall -g -std="c99" -pedantic 

all: server

chat_server.o: server.c base64.c fd_send2.c fd.h
	$(CC) -c $(CFLAGS) -o server.o server.c
	$(CC) -c $(CFLAGS) -o base64.o base64.c
	$(CC) -c $(CFLAGS) -o fd_send.o fd_send2.c
	$(CC) -c $(CFLAGS) -o fd_recv.o fd_recv.c
	$(CC) -c $(CFLAGS) -o fd_util.o fd_util.c

server: chat_server.o 
	$(CC) $(CFLAGS) -o server server.o fd_send.o base64.o fd_recv.o fd_util.o -lpthread -lssl -lcrypto

clean:
	rm -f server $(OBJS) server.ob
	rm *.o
	rm *~ *#