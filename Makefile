
CC = gcc
CFLAGS = -Wall -g -std="c99" -pedantic -lpthread

all: server
	
chat_server.o: server.c server.h
	$(CC) -c $(CFLAGS) -o server.o server.c

server: chat_server.o
	$(CC) $(CFLAGS) -o server server.o
	
clean:
	rm -f server $(OBJS) server.o