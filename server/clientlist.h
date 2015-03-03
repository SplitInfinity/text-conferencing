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

void clientlist_insert_front (Client ** client_list_head, Client* new_client);

void clientlist_remove (Client ** client_list_head, char* query_clientID);

Client * clientlist_find (Client ** client_list_head, char* query_clientID);




#endif