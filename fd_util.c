#include "fd.h"

fd_socket_t *fd_socket_new(void){
	fd_socket_t *new_sock = (fd_socket_t *) malloc(sizeof(fd_socket_t));
	
	//set defaults
	new_sock->tcp_sock = -1;
	new_sock->last_recv_opcode = -1;
	new_sock->is_open = false;

	return (new_sock);
}

void fd_socket_destroy(fd_socket_t *sock, struct ev_loop *loop){
	ev_io_stop(loop, &sock->read_w);
	fd_socket_close(sock);
	free(sock);
}

int fd_socket_close(fd_socket_t *sock){
  fd_log_i("closing socket with file descriptor: %d\n",sock->tcp_sock);
  
  remove_from_channel("chatroom", sock->tcp_sock);
  sock->is_open = false;
  return ( close(sock->tcp_sock) );
}

void fd_log_write(int level, char *file, int line, char *fmt, ...){
	char output[MAX_LOG_LINE], message[MAX_LOG_LINE];
	va_list args;

	memset(output, 0, MAX_LOG_LINE);
	memset(message, 0, MAX_LOG_LINE);

	/* start the line with the appropriate prefix */
	switch (level) {
	case DEBUG:
		snprintf(output, MAX_LOG_LINE, "[DEBUG] %s:%d: ", file, line);
		break;
	case INFO:
		snprintf(output, MAX_LOG_LINE, "[INFO] %s:%d: ", file, line);
		break;
	case MESSAGE:
		snprintf(output, MAX_LOG_LINE, "[MESSAGE] %s:%d: ", file, line);
		break;
	case WARNING:
		snprintf(output, MAX_LOG_LINE, "[WARNING] %s:%d: ", file, line);
		break;
	case CRITICAL:
		snprintf(output, MAX_LOG_LINE, "[CRITICAL] %s:%d: ", file, line);
		break;
	case ERROR:
		snprintf(output, MAX_LOG_LINE, "[ERROR] %s:%d: ", file, line);
		break;
	}
	
	/* copy the message from the user code */
	va_start(args, fmt);
	vsnprintf(message, MAX_LOG_LINE, fmt, args);
	va_end(args);

	/* assemble the complete line */
	strncat(output, message, MAX_LOG_LINE - strlen(output) - 1);

	/* enqueue this line to be written to the log */
	qput(log_queue, (void *) strdup(output));
}

/* runs in its own thread so the main thread won't block */
void *fd_log(void *data){
	char *line;

	while(1)
		while( (line = (char *) qget(log_queue)) != NULL )
			fprintf(log_file, line);
}
