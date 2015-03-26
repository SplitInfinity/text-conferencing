#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "clientlist.h"

#define MAX_CLIENTID_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_CURRENTSESSIONID_LEN 50
#define MAX_IPADDRESS_LEN 50


static pthread_mutex_t lock;



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

	return 	newClient;
}


void clientlist_insert_front (ClientNode ** client_list_head, Client* new_client){
	if (client_list_head == NULL || new_client == NULL)
		return;

	pthread_mutex_lock(&lock);
	ClientNode * tmp = (ClientNode*) malloc(sizeof(ClientNode));
	tmp->cn_client = new_client;
	tmp->nxt = (*client_list_head);

	(*client_list_head) = tmp;

	pthread_mutex_unlock(&lock);
}


void clientlist_remove (ClientNode ** client_list_head, char* query_clientID){
	if (client_list_head == NULL || *client_list_head == NULL)
		return;

	pthread_mutex_lock(&lock);
	ClientNode * prev = *client_list_head;
	//First one is the one we need to delete
	if (strcmp(prev->cn_client->clientID, query_clientID) == 0){
		*client_list_head = prev->nxt;
		free(prev);
		pthread_mutex_unlock(&lock);
		return;
	}

	ClientNode * curr = prev->nxt;

	while (curr != NULL){
		if (strcmp(curr->cn_client->clientID, query_clientID) == 0){
			prev->nxt = curr->nxt;
			free(curr);
			pthread_mutex_unlock(&lock);
			return;
		}
		prev = curr;
		curr = curr->nxt;
	}
	pthread_mutex_unlock(&lock);
}


void client_invalidate(Client * client) {
	if (client == NULL)
		return;
	strcpy(client->currentSessionID, "");
	strcpy(client->ipAddress, "");
	client->socket = -1;
	client->port = -1;
}



/*
 *	Find the client based on a given clientID
 *
 */
Client * clientlist_find (ClientNode ** client_list_head, char* query_clientID){
	if (client_list_head == NULL || *client_list_head == NULL)
		return NULL;

	ClientNode * traverser = *client_list_head;


	while (traverser !=NULL) {
		if (strcmp(traverser->cn_client->clientID, query_clientID) == 0)
			return traverser->cn_client;
		traverser = traverser->nxt;
	}
	return NULL;	
}


void clientlist_init(){
	pthread_mutex_init(&lock, NULL);
}

void clientlist_termin(){
	pthread_mutex_destroy(&lock);
}
