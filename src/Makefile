CC = gcc
CFLAGS = -g -std=gnu99 -Ilibev -fPIC

all: ev

obj_ev: server_ev.c fd_run.c base64.c fd_send_ev.c fd_recv_ev.c fd_channels.c fd_util.c fd.h 
	$(CC) -c $(CFLAGS) -o fd_run.o fd_run.c
	$(CC) -c $(CFLAGS) -o base64.o base64.c
	$(CC) -c $(CFLAGS) -Wall -pedantic -o queue.o queue.c
	$(CC) -c $(CFLAGS) -Wall -pedantic -o fd_util.o fd_util.c
	$(CC) -c $(CFLAGS) -Wall -pedantic -o fd_send.o fd_send_ev.c
	$(CC) -c $(CFLAGS) -Wall -pedantic -o fd_recv.o fd_recv_ev.c
	$(CC) -c $(CFLAGS) -Wall -pedantic -o fd_channels.o fd_channels.c
	$(CC) -c $(CFLAGS) -Wall -pedantic -o server.o server_ev.c

obj_legacy: fd.h
	$(CC) -c $(CFLAGS) -o base64.o base64.c
	$(CC) -c $(CFLAGS) -Wall -pedantic -o fd_send.o fd_send_legacy.c
	$(CC) -c $(CFLAGS) -Wall -pedantic -o fd_recv.o fd_recv_legacy.c
	$(CC) -c $(CFLAGS) -Wall -pedantic -o server.o server_legacy.c

obj_client: fd.h client.c
	$(CC) -c $(CFLAGS) -o base64.o base64.c
	$(CC) -c $(CFLAGS) -Wall -pedantic -o client.o client.c

ev: obj_ev
	$(CC) $(CFLAGS) -o server server.o fd_util.o fd_run.o fd_send.o base64.o queue.o fd_recv.o fd_channels.o -lpthread -lssl -lcrypto

lib: obj_ev
	$(CC) -shared $(CFLAGS) -o libfiredrake.so fd_util.o fd_run.o fd_send.o base64.o queue.o fd_recv.o fd_channels.o -lpthread -lssl -lcrypto
	PYTHONPATH=$$PYTHONPATH./pybindgen python firedrake.py > firedrake_binding.c
	$(CC) $(CFLAGS) -I/usr/include/python2.7 -c -o firedrake_binding.o firedrake_binding.c
	gcc -shared -L. -o firedrake.so firedrake_binding.o -lpython2.7 -lfiredrake

legacy: obj_legacy
	$(CC) $(CFLAGS) -o server server.o fd_send.o base64.o fd_recv.o -lpthread -lssl -lcrypto

client: obj_client
	$(CC) $(CFLAGS) -o client client.o base64.o  -lcrypto

test: 
	$(CC) -c $(CFLAGS) -o fd_run.o fd_run.c
	$(CC) -c $(CFLAGS) -o base64.o base64.c
	$(CC) -c $(CFLAGS) -o fd_util.o fd_util.c
	$(CC) -c $(CFLAGS) -o fd_send.o fd_send_ev.c
	$(CC) -c $(CFLAGS) -o fd_recv.o fd_recv_ev.c
	$(CC) -c $(CFLAGS) -o fd_channels.o fd_channels.c
	$(CC) -c $(CFLAGS) -o server.o server_test.c
	$(CC) $(CFLAGS) -o server server.o fd_util.o fd_run.o fd_send.o base64.o fd_recv.o fd_channels.o -lpthread -lssl -lcrypto

clean:
	rm -f server client *.o *.so
	rm -f *~ *#
	rm -f *.fd
	rm -f firedrake_binding.c
