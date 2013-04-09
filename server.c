/*
 * chat_server.c
 *
 *  Created on: Apr 15, 2012
 *      Author: Grim
 *
 */

#include "server.h"

//#define DEBUG0

static void get_connections();
static int parse_logon(char *args, char *uname);
static bool valid_uname(char *uname);
static void send_active_users(Client *client);
static void listen_to_client(Client *client);
static void remove_user(Client *client);
static void parse_chat(Client *client, char *msg);
static void client_signed_on(Client *client);
void* setup_client(void *args);

Client  *clients;
int   num_clients;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main (int argc, char **argv){

  //set the initial state
  NEW_N(clients, Client, LISTENQ);
  num_clients = 0;
  for(int i = 0; i < LISTENQ; i++){
    clients[i].id = 0;
  }

  get_connections();

  free(clients);
}

//runs the main loop that gets connections from clients and spawns new threads
static void get_connections(){
  int         listenfd,
            connfd;

  socklen_t       clilen;

  struct sockaddr_in  cliaddr,
            servaddr;

  pthread_t     threads[LISTENQ];

  //Create a socket
  //If sockfd<0 there was an error in the creation of the socket
  if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
    perror("Problem in creating the socket");
    exit(2);
  }


  //preparation of the socket address
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(SERV_PORT);

  //bind the socket
  bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

  //listen to the socket by creating a connection queue, then wait for clients
  listen (listenfd, LISTENQ);

  printf("%s\n","Server running...waiting for connections.");

  for ( ; ; ) {

    clilen = sizeof(cliaddr);
    //accept a connection
    connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

    printf("%s\n","Received request...");

    //if itÕs 0, the thread was created properly
    //make sure num_clients doesn't change during this!'
    if(num_clients < LISTENQ){
      pthread_mutex_lock(&mutex);
      if ( pthread_create(threads + num_clients, NULL, setup_client, (void *) &connfd) != 0 ) {
        perror("Problem in creating client thread.");
        close(connfd);
      }
      pthread_mutex_unlock(&mutex);
    }
  }
}

void* setup_client(void *args){

  int   n, connfd;
  char  recvline[MAXLINE], sendline[MAXLINE], uname[NAMELEN];
  Client  client;

  //make sure this has exclusive access to num_clients and clients until it's all set up
  pthread_mutex_lock(&mutex);

  printf ("%s\n","Thread created for dealing with client requests");

  connfd = *( (int *) args);

  n = recv(connfd, recvline, MAXLINE,0);
  printf("String received from the client: %s.\n", recvline);

  if (n < 0){
    printf("%s\n", "Read error");
  }

  //if the message follows the protocol then add the client to the list
  if (parse_logon(recvline, uname)){

    //set up the client struct and add it to the global list
    client.sockfd = connfd;
    num_clients++;
    strcpy(client.uname, uname);

    //assign the client to the first id = 0 spot in the global list
    for(int i = 0; i < LISTENQ; i++){
      if( clients[i].id == 0 ){
        client.id = i + 1;
        clients[i] = client;
        break;
      }
    }

    //tell the client it successfully connected
    sprintf(sendline, "%d %d", SIGNON, SIGNON_SUCCESS);
    send(connfd, sendline, strlen(sendline), 0);
    client_signed_on(&client);
  }
  //otherwise exit the thread and tell the client that the username was invalid
  else{

    //tell the client it failed to successfully connect
    sprintf(sendline, "%d %d", SIGNON, SIGNON_FAILURE);
    send(connfd, sendline, strlen(sendline), 0);

    //close the thread and the socket and release the mutex
    pthread_mutex_unlock(&mutex);
    close(connfd);
    pthread_exit(EXIT_SUCCESS);
  }

  //unlock the mutex
  pthread_mutex_unlock(&mutex);

#ifdef DEBUG0
  fprintf(stderr, "Username accepted!\n");
#endif

  listen_to_client(&client);

  return(NULL);
}

//listens for messages from the given client
static void listen_to_client(Client *client){
  int n, connfd, arg;
  char recvline[MAXLINE];

  connfd = client->sockfd;
  memset(recvline, 0, MAXLINE);

  for(;;){

    strcpy(recvline, "");
    n = recv(connfd, recvline, MAXLINE,0);

#ifdef DEBUG0
  fprintf(stdout, "String received from client on fd %d: %s.\n", connfd, recvline);
#endif

    sscanf(recvline, "%d", &arg);

    if(arg == STATUS){
      send_active_users(client);
    }
    else if(arg == SIGNOFF){
      remove_user(client);
      pthread_exit(EXIT_SUCCESS);
    }
    else if(arg == CHAT){
      parse_chat(client, recvline + 2);
    }
    else{

    }
  }
}

//Parses a string and returns 1 if the leading int corresponds to the logon code from the protocol (SIGNON)
static int parse_logon(char *args, char *uname){
  int argid;

  sscanf(args, "%d %s", &argid, uname);

  //parse the correct signal and verify the username
  if(argid == SIGNON && valid_uname(uname)){
    return(true);
  }
  else{
    return(false);
  }
}

static void client_signed_on(Client *client){

  char  sendline[MAXLINE];

  sleep(ASEC);

  sprintf(sendline, "%d %s", SIGNON, client->uname);

  for(int i = 0; i < LISTENQ; i++){
    if( (clients + i)->id != 0){
      send( (clients + i)->sockfd, sendline, strlen(sendline), 0);
    }
  }

}

//sends the message to the client specified in msg and inserts the sending client's username
static void parse_chat(Client *client, char *msg){
  int receiver;
  char sendline[MAXLINE];

  //format the string to be sent to the specified client
  sscanf(msg, "%d ", &receiver);
  sprintf(sendline, "%d ", CHAT);
  strcat(sendline, client->uname);
  strcat(sendline, ": ");
  strcat(sendline, msg + 2);

  //send the string
  send( (clients[receiver -1]).sockfd, sendline, strlen(sendline), 0);

#ifdef DEBUG0
  fprintf(stdout, "Sending message to user: %s.\n", sendline);
#endif

}

//removes the client from the list
static void remove_user(Client *client){

  char  sendline[MAXLINE];

#ifdef DEBUG0
  fprintf(stdout, "Removing user: %s.\n", (clients[client->id - 1]).uname);
#endif

  sprintf(sendline, "%d %s", SIGNOFF, client->uname);

  for(int i = 0; i < LISTENQ; i++){
    if( (clients + i)->id != 0){
      send( (clients + i)->sockfd, sendline, strlen(sendline), 0);
    }
  }

  (clients[client->id - 1]).id = 0;
  close( (clients[client->id - 1]).sockfd);
  strcpy( (clients[client->id - 1]).uname, "");
  num_clients--;

}

static bool valid_uname(char *uname){
  char  c;
  int   a;

  //check to see that all characters are valid (alphanumeric + underscore)
  for(int i = 0; i < strlen(uname); i++){

    c = uname[i];
    a = (int) c;
    fprintf(stdout, "Uname character: %c, ASCII: %d.\n", c, a);

    if( (a > 47) && (a < 58) ){

    }
    else if(a == 95){

    }
    else if( (a > 64) && (a < 90) ){

    }
    else if( (a > 96) && (a < 123) ){

    }
    else{
      fprintf(stdout, "Invalid character detected in username: %c.\n", c);
      return(false);
    }
  }

#ifdef DEBUG0
  fprintf(stdout, "num_clients: %d\n", num_clients);
#endif

  //check to see that the uname isn't already connected to the server
  for(int i = 0; i <= num_clients; i++){

#ifdef DEBUG0
  fprintf(stdout, "Comparing requested name %s with client name %s.\n", uname, clients[i].uname);
#endif

    if( !strcmp(uname, clients[i].uname) ){
      fprintf(stdout, "Username already connected to server: %s.\n", uname);
      return(false);
    }
  }

  //if it's gotten this far, then it must be true
  return(true);
}

//sends the list of active users
static void send_active_users(Client *client){
  int   connfd = client->sockfd;
  int   n;
  char  sendline[MAXLINE], intbuf[NAMELEN + 2];

  sprintf(sendline, "%d ", STATUS);

  //concatenate the string for sending
  for(int i = 0; i < LISTENQ; i++){
    memset(intbuf, 0, NAMELEN + 2);

    //make sure the client exists
    if( (clients[i].id) != 0){
      sprintf(intbuf, "%d %s ", clients[i].id, clients[i].uname);
      strcat(sendline, intbuf);
    }
  }

  n = send(connfd, sendline, strlen(sendline), 0);
  fprintf(stdout, "Sending: %s on fd %d.\nSent %d bytes.\n", sendline, connfd, n);

}
