
CC = gcc
CFLAGS = -Wall -g -std="c99" -pedantic -lpthread
OBJS = chat_client.o
all: mychat server

mychat: $(OBJS) chat_client.h
	$(CC) $(CFLAGS) -o mychat $(OBJS)
	
chat_server.o: chat_server.c chat_server.h
	$(CC) -c $(CFLAGS) -o chat_server.o chat_server.c

server: chat_server.o
	$(CC) $(CFLAGS) -o server chat_server.o
	
clean:
	rm -f mychat server $(OBJS) chat_server.o