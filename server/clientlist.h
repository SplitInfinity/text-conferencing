#ifndef CLIENTLIST_H
#define CLIENTLIST_H

typedef struct client {
	char* clientID;
	char* password;
	char* currentSessionID;
	char* ipAddress;	// might end up being an int or a long
	unsigned int port;
	int socket;
	struct client * nxt;
} Client;


Client * create_client (char* clientID, char* password, char* currentSessionID, char* ipAddress, unsigned int port, int socket);

Client * client_insert_front (Client * head);
Client * client_remove (Client * head);
Client * client_find (char* clientID);




#endif