#include <stdlib.h>
#include <string.h>
#include "clientlist.h"

#define MAX_CLIENTID_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_CURRENTSESSIONID_LEN 50
#define MAX_IPADDRESS_LEN 50



Client * create_client (char* clientID, char* password, char* currentSessionID, char* ipAddress, unsigned int port, int socket){
	Client * newClient = (Client *) malloc (sizeof(Client));

	newClient->clientID = (char*) malloc(MAX_CLIENTID_LEN *sizeof(char));
	newClient->password = (char*) malloc(MAX_PASSWORD_LEN*sizeof(char));
	newClient->currentSessionID = (char*) malloc(MAX_CURRENTSESSIONID_LEN*sizeof(char));
	newClient->ipAddress = (char*) malloc(MAX_IPADDRESS_LEN*sizeof(char));
	strcpy(newClient->clientID, clientID);
	strcpy(newClient->password, password);
	strcpy(newClient->currentSessionID, currentSessionID);
	strcpy(newClient->ipAddress, currentSessionID);
	newClient->port = port;
	newClient->socket = socket;
	newClient->nxt = NULL;

	return 	newClient;
}


void clientlist_insert_front (Client ** client_list_head, Client* new_client){
	if (client_list_head == NULL)
		return;

	Client* tmp = (*client_list_head);


	//implement primative
	new_client->nxt = tmp;

	*client_list_head =  new_client;
	//implement primative
}


void clientlist_remove (Client ** client_list_head, char* query_clientID){
	if (client_list_head == NULL || *client_list_head == NULL)
		return;

	Client * prev = *client_list_head;

	//First one is the one we need to delete
	if (strcmp(prev->clientID, query_clientID) == 0){
		client_list_head = &(prev->nxt);
		free(prev);
		return;
	}

	Client * curr = prev->nxt;

	while (curr != NULL){
		if (strcmp(curr->clientID, query_clientID) == 0){
			prev->nxt = curr->nxt;
			free(curr);
			return;
		}
		prev = curr;
		curr = curr->nxt;
	}
}


/*
 *	Find the client based on a given clientID
 *
 */
Client * clientlist_find (Client ** client_list_head, char* query_clientID){
	if (client_list_head == NULL || *client_list_head == NULL)
		return NULL;

	Client * traverser = *client_list_head;

	//I dont think this is necissary!!! (delete)
	if (strcmp(traverser->clientID, query_clientID) == 0)
		return traverser;

	while (traverser->nxt !=NULL) {
		if (strcmp(traverser->clientID, query_clientID) == 0)
			return traverser;
		traverser = traverser->nxt;
	}
	return NULL;	
}

