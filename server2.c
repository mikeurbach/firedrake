#include "fd.h"

int main(int argc, char *argv[]){
	int port;

	/* read the port from the command line */
	if(argc != 2){
		printf("usage: server <port>\n");
		exit(1);
	}
	port = atoi(argv[1]);

	/* start firedrake */
	fd_run(port);
	
	return 0;
}
