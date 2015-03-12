#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>//What is the types for?
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include "clientlist.h"
#include "sessionlist.h"
#include "err.h"


#define BACKLOG 20			// the number of pending connections queue will hold (most systems use 20)



static Client * clientlist = NULL;
static Session * sessionlist = NULL; 
int n = 0;


/*
 *	HELPER/TRIVIAL SERVER FUNCTION
 *
 *
 */




void waitFor (unsigned int secs) {
    int retTime = time(0) + secs;     // Get finishing time.
    while (time(0) < retTime);    // Loop until it arrives.
}


void server_listClients(int client_sock){
	if (clientlist == NULL)
		return;

	char msg[] = "List of all clients on the server:\n";
	int msg_len = strlen(msg);
	send (client_sock, msg, msg_len, 0);

	Client * traverse = clientlist;
	while(traverse != NULL){
		msg_len = strlen(traverse->clientID);
		send (client_sock, traverse->clientID, msg_len, 0);
		send(client_sock,"\n", 1,0);
		traverse = traverse->nxt;
	}
}


/*
 *	IMPORTANT SERVER FUNCTION
 *
 *
 */



int sever_list_sessions(int client_sock){
	/* Code for listing */
	if (sessionlist == NULL)
		return SERVER_NO_SESSIONS;
	char msg[] = "List of all sessions on the server:\n";
	int msg_len = strlen(msg);
	send (client_sock, msg, msg_len, 0);

	Session * traverse = sessionlist;
	while(traverse != NULL){
		msg_len = strlen(traverse->sessionID);
		send (client_sock, traverse->sessionID, msg_len, 0);
		send(client_sock,"\n", 1,0);
		traverse = traverse->nxt;
	}
	return SERVER_SUCCESS;
}


int server_broadcast (char * message, char * messageSenderID, char * sessionID){
	/* Code to broadcast messages */
	if (message == NULL)
		return SERVER_PARAM_ERROR;
	if (sessionID == NULL)
		return BAD_SESSION;

	Session * queriedsession = sessionlist_find(&sessionlist, sessionID);
	if (queriedsession == NULL)
		return SESSION_NOT_EXIST;

	if (queriedsession->clientsInSession == NULL)
		return SERVER_BROADCAST_ERROR;

	Client * sessionClient = *queriedsession->clientsInSession;

	while(sessionClient != NULL){
		//We dont want to send to ourself
		if (strcmp(messageSenderID, sessionClient->clientID) !=0){
			int msg_len = strlen(message);
			send (sessionClient->socket, message, msg_len, 0);
			send (sessionClient->socket,"\n", 1,0);
		}
		sessionClient = sessionClient->nxt;
	}

	return SERVER_SUCCESS;
}

int server_add_new_session(char * sessionID){
	/* Code to add session */
	if (sessionlist_find(&sessionlist, sessionID) != NULL)
		return SESSION_EXISTS;
	Session * newSession = create_session(sessionID);
	if (newSession == NULL)
		return BAD_SESSION;
	sessionlist_insert_front(&sessionlist, newSession);
	return SERVER_SUCCESS;
}

int server_client_join_session(Client * newClient, char * sessionID){
	/* Code to add session */
	if (newClient == NULL)
		return BAD_CLIENT;
	if (sessionID == NULL)
		return BAD_SESSION;

	if (sessionlist_find(&sessionlist, sessionID) == NULL)
		return SESSION_NOT_EXIST;

	sessionlist_addclient(&sessionlist,sessionID, newClient);
	return SERVER_SUCCESS;
}

int server_accept_client(Client * newClient) {
	/* Check to determine if client is kosher */
	if (newClient == NULL)
		return -1; 
	if (newClient->clientID == NULL)
		return -1;
	if (clientlist_find (&clientlist, newClient->clientID) == NULL)
		return -1;

	//So far so good!
	return SERVER_SUCCESS;
}

/*
 * Client Handler
 *
 */
void * server_client_handler(void * conn_sock){		//DOes this HAVE to be a function pointer?
	int client_sock = *((int*) conn_sock); 	// Get the client socket

	int msg_len = strlen("Hi from Arash");
	send (client_sock, "Hi from Arash\n", msg_len+1, 0);
	n++;
	char * temp = (char*)malloc(10000*sizeof(char));
	sprintf(temp, "Client %d", n);
	Client * newClient = create_client(temp, "", "", "",  1, client_sock);
	free(temp);
	clientlist_insert_front(&clientlist, newClient);

	/*int n = 0;
	while(1 ){
		waitFor(1);
		
		char * temp = (char*)malloc(10000*sizeof(char));
		sprintf(temp, "ping!%d\n",n);
		msg_len = strlen(temp);
		send (client_sock, temp, msg_len+1, 0);
		n++;
	}*/



	server_listClients(client_sock);
	close(client_sock);
	clientlist_remove(&clientlist, newClient->clientID);
	return 0;
}








int main (int argc, char ** arv){
	//unsigned int server_port;
	int sockfd, new_sockfd;
	struct addrinfo hints, *serverinfo;
	struct sockaddr_storage connector_addr;
	socklen_t sockaddr_size; 
	//struct sigaction sa;

	//Ensure proper usage
	if (argc != 2){
		printf("Usage: server <TCP port to listen on>\n");
		return 1;
	}
	printf("Establishing teleconference server...\n");

	//Set up the clientlist


	//Extract server port#
	//server_port = atoi(arv[1]);

	//Establish the connection
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM; 		//To specify TCP
	hints.ai_flags = AI_PASSIVE; 			// It will fill up our IP adress for us

	int status;
	if ((status = getaddrinfo(NULL, arv[1], &hints, &serverinfo)) != 0){
		fprintf(stderr, "Server Error: getaddrinfo error %s\n", gai_strerror(status));
		return 1; 							//getaddrinfo() failed; we must close the server
	}


	//Create our socket
	sockfd = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
	if (sockfd == -1){
		fprintf(stderr, "Server Error: socket creation error \n");
		return 1;
	}

	//We want to listen on our server port, so we bind
	if (bind(sockfd, serverinfo->ai_addr, serverinfo->ai_addrlen) == -1){
		fprintf(stderr, "Server Error: binding socket error\n");
		return 1;
	}



	//Lets listen for incoming connections
	if (listen(sockfd, BACKLOG) == -1){
		//Change this part later on so it is correct but for now
		fprintf(stderr, "Server: listen error \n");
		return 1;
	}

	pthread_t connection;
	sockaddr_size = sizeof(connector_addr);
	
	while (1) {
		new_sockfd = accept(sockfd, (struct sockaddr *) &connector_addr, &sockaddr_size);

		pthread_create(&connection, NULL, server_client_handler, (void*) &new_sockfd);


	}

}