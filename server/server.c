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
#include "../utils.h"


#define BACKLOG 20			// the number of pending connections queue will hold (most systems use 20)



static Client * clientlist = NULL;
static Session * sessionlist = NULL; 


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

void server_transmit_tcp (int client_sock, int packetType, char * src, char * data ){
	//Setup packet
	Packet sendPack; 
	sendPack.type = packetType; //JN_ACK
	sendPack.size = BUFFERLEN;
	strcpy(sendPack.source, src);
	strcpy(sendPack.data, data);

	//pack the bytearray
	char * sendMsg = (char * ) malloc(BUFFERLEN*sizeof(char));
	int sendMsgSize = create_bytearray(&sendPack, sendMsg);

	send (client_sock, sendMsg, sendMsgSize, 0);
	free (sendMsg);
}



/*
 *	IMPORTANT SERVER FUNCTION
 *
 *
 */
 void server_bootstrap (){
 	ConfigLine * confline;
 	while ( (confline = read_config()) != NULL){
 		Client * newClient = create_client(confline->clientID, confline->password, "", "",  1, -1);
 		clientlist_insert_front(&clientlist, newClient);
 	} 
 }



int sever_list_sessions(int client_sock){
	/* Code for listing */
	char msg[BUFFERLEN];
	sprintf (msg ,"Users:");
	Client * client_traverse = clientlist;
	while (client_traverse != NULL && client_traverse->socket != -1){
		strcat(msg, client_traverse->clientID);
		strcat(msg, ",");
		client_traverse = client_traverse->nxt;
	}
	strcat(msg, "\n");
	Session * session_traverse = sessionlist;
	while(session_traverse != NULL){
		strcat(msg, session_traverse->sessionID);
		strcat(msg, ",");
		session_traverse = session_traverse->nxt;
	}

	//Setup packet
	Packet sendPack; 
	sendPack.type = QU_ACK;
	sendPack.size = BUFFERLEN;
	strcpy(sendPack.source, "SERVER");
	strcpy(sendPack.data, msg);

	//pack the bytearray
	char * sendMsg = (char * ) malloc(BUFFERLEN*sizeof(char));
	int sendMsgSize = create_bytearray(&sendPack, sendMsg);


	send (client_sock, sendMsg, sendMsgSize, 0);
	free (sendMsg);

	printf("sent this: %s\n", sendMsg);
	return SERVER_SUCCESS;
}


void server_broadcast (char * message, char * clientID){
	/* Code to broadcast messages */
	if (message == NULL || clientID == NULL)
		return;

	Client * the_client = clientlist_find(&clientlist, clientID);

	Session * queriedsession = sessionlist_find(&sessionlist, the_client->currentSessionID);
	if (queriedsession == NULL)
		return;

	if (queriedsession->clientsInSession == NULL)
		return;

	Client * sessionClient = *queriedsession->clientsInSession;

	while(sessionClient != NULL){
		//We dont want to send to ourself
		if (strcmp(the_client->clientID, sessionClient->clientID) !=0){
			int msg_len = strlen(message);
			send (sessionClient->socket, message, msg_len, 0);
			send (sessionClient->socket,"\n", 1,0);
		}
		sessionClient = sessionClient->nxt;
	}
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

int server_client_leave_session(Client * newClient, char * sessionID) {

	return 0;
}

//LATER ON ADD IPADDRESS AND PORT SAVING
void server_login_client(char * clientID, char * passw, int sock) {
	/* Check to determine if client is kosher */
	(void) passw;

	if (clientID == NULL || passw == NULL)
		return; 
	Client * client = clientlist_find(&clientlist, clientID);
	if (client == NULL) {
		//SEND A NACK
		server_transmit_tcp(sock, LO_NAK, "SERVER", "CLIENT DOES NOT EXIST");
		return;
	} else if (strcmp(passw, client->password) != 0){
		//SEND A NACK
		server_transmit_tcp(sock, LO_NAK, "SERVER", "WRONG PASSWORD");
		return;
	}

	//Check to see if pass is correct

	client->socket = sock;
	server_transmit_tcp(client->socket, LO_ACK, "SERVER", client->clientID);

	//So far so good!
}

void server_client_exit(char * clientID, int client_sock) {
	clientlist_remove(&clientlist, clientID);
	close(client_sock);
	printf("%s has disconnected\n", clientID);
	pthread_exit(NULL);
}

/*
 * Client Handler
 *
 */
void * server_client_handler(void * conn_sock){		//DOes this HAVE to be a function pointer?
	int client_sock = *((int*) conn_sock); 	// Get the client socket
	char buffer[BUFFERLEN];
	int bytes_received =0; 
	while ((bytes_received = recv (client_sock, buffer, BUFFERLEN, 0)) ) {
		buffer[bytes_received] = '\0';
		printf("INCOMING PACK-ET: %s\n", buffer);
		//send (client_sock, buffer, bytes_received, 0);
		/*
		char * temp = (char*)malloc(10000*sizeof(char));
		sprintf(temp, "Client %d", client_sock);
		Client * newClient = create_client(temp, "", "", "",  1, client_sock);
		clientlist_insert_front(&clientlist, newClient);
		*/
		//server_listClients(client_sock);
		Packet incomingPack; 
		extract_packet(&incomingPack, buffer);

		switch(incomingPack.type) {
			case LOGIN:
				server_login_client(incomingPack.source,incomingPack.data,client_sock);
				break;
			case EXIT:
				server_client_exit(incomingPack.source, client_sock);
				break;
			/*case JOIN:
			//	int status = server_client_join_session();

				break;
			case NEW_SESS:
				//int status = server_add_new_session();

				break;
			case LEAVE_SESS:
				//int status = server_client_leave_session();
				
				break;*/
			case MESSAGE:
				server_broadcast(incomingPack.data, incomingPack.source);
				break;
			case QUERY:
				sever_list_sessions(client_sock);
				break;
		//	default:

		}



	}

	close(client_sock);
	
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
	
	//bootstrap the clients
	server_bootstrap();

	while (1) {
		new_sockfd = accept(sockfd, (struct sockaddr *) &connector_addr, &sockaddr_size);

		pthread_create(&connection, NULL, server_client_handler, (void*) &new_sockfd);


	}

}