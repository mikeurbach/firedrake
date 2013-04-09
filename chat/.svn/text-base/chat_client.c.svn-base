/*
 * chat_client.c
 *
 *  Created on: Apr 15, 2012
 *      Author: Grim
 *
 *      Initial code taken from notes provided at
 *      http://www.cs.dartmouth.edu/~campbell/cs60/socketprogramming.html
 */



#include "chat_client.h"

//#define DEBUG0

static int IMconnect(char * addr, char * uname);
static void get_username(char *uname);
void* listen_thread(void *args);
static void parse_msg(char *msg);
static int parse_status(char *status);
static void interface(int serv_sock);
static void chat_menu(int serv_sock);
static void get_status(int serv_sock);
static void get_chat_msg(char *msg);
static void signon(char *addr);
static void send_msg(int serv_sock, int uid);
static void signoff(int serv_sock);
static void clrscrn();
static void place_cursor(int x, int y);
static void signon_user(char *msg);
static void signoff_user(char *msg);

bool connected;

int main(int argc, char **argv){

	connected = false;

	//clear the screen and display a welcome message
	clrscrn();
	fprintf(stdout, "\nWelcome to Drew's IM Client!\n");
	signon(SERVADDR);

	exit(EXIT_SUCCESS);
}

//contains the main interface loop for signing onto the IM server
static void signon(char *addr){

	int		n, serv_sock, protmsg, protstatus, response;
	char 	uname[15],
			recvline[MAXLINE];

	pthread_t	listener;

	fpurge(stdin);

	fprintf(stdout, "\nWhat would you like to do?\n\n[0] Exit\n[1] Sign on\n\nResponse: ");
	n = fscanf(stdin, "%d", &response);

	if(response == 0 && n == 1){
		signoff(serv_sock);
	}
	else if (response == 1 && n == 1){
		//get the username
		get_username(uname);

#ifdef DEBUG0
fprintf(stdout, "The user specified name is: %s.\n", uname);
#endif

		//connect to the server and get the socket file descriptor back
		serv_sock = IMconnect(addr, uname);

		//receive the server's response to the signon request
		n = recv(serv_sock, recvline, MAXLINE,0);
		if (n < 0){
			printf("%s\n", "Read error in signon()");
		}

#ifdef DEBUG0
printf("String received from the server on fd %d: %s.\n", serv_sock, recvline);
#endif

		//parse the server's response
		sscanf(recvline, "%d %d", &protmsg, &protstatus);

		if (protmsg == SIGNON && protstatus == SIGNON_SUCCESS){
			fprintf(stdout, "Successfully signed onto the chat server!\n");
			connected = true;

			//spawn the listening thread, then call the interface
			if ( pthread_create(&listener, NULL, listen_thread, (void *) &serv_sock) != 0 ) {
				perror("Problem in creating client thread.");
				exit(EXIT_FAILURE);
			}
			interface(serv_sock);
		}
		else{
			clrscrn();
			fprintf(stdout, "%s is not a valid username...try again.\n", uname);
			fprintf(stdout, "Make sure your username contains only alphanumeric characters and underscores.\n");
			signon(addr);
		}
	}
	else{
		clrscrn();
		fprintf(stdout, "Invalid choice. Please pick a number.\n");
		fpurge(stdin);
		signon(addr);
	}

	pthread_cancel(listener);
}

//connects the client to the IM Server, and sends a SIGNON protocol message with the username
static int IMconnect(char * addr, char * uname){
	int sockfd;
	struct sockaddr_in servaddr;
	char sendline[MAXLINE];

	//Create a socket for the client
	//If sockfd<0 there was an error in the creation of the socket
	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		perror("Problem in creating the socket, exiting the program.");
		exit(2);
	}

	//Creation of the socket
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr= inet_addr(addr);
	servaddr.sin_port =  htons(SERV_PORT); //convert to big-endian order

	//Connection of the client to the socket
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
		perror("Problem in connecting to the server, exiting the program.");
		exit(3);
	}

	//Send the username with the appropriate tag to the server
	sprintf(sendline, "%d ", SIGNON);
	strcat(sendline, uname);
	send(sockfd, sendline, strlen(sendline), 0);

#ifdef DEBUG0
	fprintf(stderr, "String sent to server: %s.\n", sendline);
#endif

	return (sockfd);
}

//Gets the username from the user
static void get_username(char *uname){
	fprintf(stdout, "What username would you like to use to connect to the IM Network?\nUser Name: ");
	fscanf(stdin, "%s", uname);
}

//listen for messages from the server and display them to the user
void* listen_thread(void *args){
	int serv_sock = *( (int *) args);
	int n;
	char recvline[MAXLINE];

	memset(recvline, 0, MAXLINE);

#ifdef DEBUG0
	fprintf(stdout, "Started a new thread for listening to the server on fd %d.\n", serv_sock);
#endif
	for(;;){
		n = recv(serv_sock, recvline, MAXLINE,0);
		if (n < 0){
			//printf("%s\n", "Read error in listen_thread()");
		}

		//fprintf(stdout, "%s", recvline);
		parse_msg(recvline);

	}

	return(NULL);
}

//parses the protocol tag and sends the leftovers to the appropriate function
static void parse_msg(char *msg){
	int tag, n;

	if(connected){
		n = sscanf(msg, "%d", &tag);

		if( (n == 1) && (tag == STATUS) ){
			parse_status(msg + 2);
		}
		else if( (n == 1) && (tag == SIGNON) ){
			signon_user(msg + 2);
		}
		else if( (n == 1) && (tag == SIGNOFF) ){
			signoff_user(msg + 2);
		}
		else if( (n == 1) && (tag == CHAT) ){
			get_chat_msg(msg + 2);
		}
		else{
			fprintf(stdout, "Unknown message received from the server...ignoring it.\n");
		}
	}
}

//show simple command line interface for the user and enable interaction
static void interface(int serv_sock){

	int n, response;

	clrscrn();

	for(;;){
		//display a simple menu
		fprintf(stdout, "\n\nWhat would you like to do?\n\n[0] Exit\n[1] Chat\nResponse: ");
		n = fscanf(stdin, "%d", &response);

		if(response == 0 && n == 1){
			signoff(serv_sock);
		}
		else if (response == 1 && n == 1){
			chat_menu(serv_sock);
		}
		else{
			clrscrn();
			fprintf(stdout, "Invalid choice. Please pick a number.\n");
			fpurge(stdin);
			interface(serv_sock);
		}
	}
}

//performs the chat menu functions for enabling chat
static void chat_menu(int serv_sock){
	int n, response;

	clrscrn();
	get_status(serv_sock);
	sleep(ASEC);

	//display a simple prompt
	fprintf(stdout, "Who would you like to chat with?\nResponse: ");
	n = fscanf(stdin, "%d", &response);

	if(response > 0 && n == 1){
		fpurge(stdin);
		send_msg(serv_sock, response);
	}
	else{
		clrscrn();
		fprintf(stdout, "Invalid choice.\n");
		fpurge(stdin);
		interface(serv_sock);
	}
}

//sends a request to get the status from the server
static void get_status(int serv_sock){

	char	sendline[MAXLINE];

	//send the server the status tag
	sprintf(sendline, "%d", STATUS);
	send(serv_sock, sendline, strlen(sendline), 0);
	fprintf(stdout, "Status request sent to the server.\n");

}

//parses the status message from the server
static int parse_status(char *status){
	int n = 0;
	int id, num_users;

	char uname[NAMELEN];

	fprintf(stdout, "\n\nUSERS ONLINE:\n");

	while(n < strlen(status) ){
		//scan in the names and display them to the user
		sscanf(status + n, "%d %s", &id, uname);
		fprintf(stdout, "[%d] %s\n", id, uname);
		n += (3 + strlen(uname) );

		num_users++;
	}

	return(num_users);
}

static void get_chat_msg(char *msg){
	fprintf(stdout, "%s\n", msg);
}

//gets the message from the user and formats it so the server knows what to do with it
static void send_msg(int serv_sock, int uid){
	char 	sendline[MAXLINE], temp, temp2[2];
	int		n;

	memset(sendline, 0, MAXLINE);
	sprintf(sendline, "%d %d ", CHAT, uid);

	fprintf(stdout, "What would you like to say?\n");

	//get the chat string
	do{
		fscanf(stdin, "%c", &temp);
		sprintf(temp2, "%c", temp);
		strcat(sendline, temp2);
	}while( temp != '\n' );

#ifdef DEBUG0
	fprintf(stdout, "Sending message: ""%s"" to the server.\n", sendline);
#endif

	//send the formatted chat string
	n = send(serv_sock, sendline, strlen(sendline), 0);

#ifdef DEBUG0
	fprintf(stdout, "Sent message: ""%s"" to the server.\n", sendline);
#endif

}

//informs the server that the client is signing off, and does some cleanup
static void signoff(int serv_sock){
	char sendline[MAXLINE];

	connected = false;

	memset(sendline, 0, MAXLINE);

	//send the server the signoff tag
	sprintf(sendline, "%d", SIGNOFF);
	send(serv_sock, sendline, strlen(sendline), 0);

	close(serv_sock);
	fprintf(stdout, "Thanks for chatting!\n");
	exit(EXIT_SUCCESS);
}

//clear terminal screen and place cursor at top left
static void clrscrn(){
	fprintf(stdout, "\033[2J");
	place_cursor(0,0);
}

//places cursor at x,y
static void place_cursor(int x, int y){
	fprintf(stdout, "\033[<%d>;<%d>H", x, y);
}

//informs user of user's new available status
static void signon_user(char *msg){
	fprintf(stdout, "%s has just signed on.\n", msg);
}

//informs user of user's new unavailable status
static void signoff_user(char *msg){
	fprintf(stdout, "%s has just signed off.\n", msg);
}
