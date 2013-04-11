
CC = gcc
CFLAGS = -Wall -g -std="c99" -pedantic 

all: server

chat_server.o: server.c base64.c fd_send2.c fd.h
	$(CC) -c $(CFLAGS) -o server.o server.c
	$(CC) -c $(CFLAGS) -o base64.o base64.c
<<<<<<< HEAD
	$(CC) -c $(CFLAGS) -o fd_send.o fd_send2.c
=======
	$(CC) -c $(CFLAGS) -o fd_send.o fd_send.c
	$(CC) -c $(CFLAGS) -o fd_recv.o fd_recv.c
>>>>>>> 85b8dd41a9034dc69dc5c59ee4b0b11833fc914c

server: chat_server.o 
	$(CC) $(CFLAGS) -o server server.o fd_send.o base64.o fd_recv.o -lpthread -lssl -lcrypto

clean:
	rm -f server $(OBJS) server.ob
	rm *~ *#