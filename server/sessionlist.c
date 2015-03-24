#include <stdlib.h>
#include <string.h>
#include "sessionlist.h"

#define MAX_SESSIONID_LEN 50



Session * create_session(char * sessionID){
	//Do i need to return NULL on failure?
	Session * new_session = (Session *) malloc (sizeof(Session));
	new_session->sessionID = (char *) malloc(MAX_SESSIONID_LEN*sizeof(char));
	strcpy(new_session->sessionID, sessionID);
	new_session->clientsInSession = NULL;
	new_session->nxt = NULL;
	return new_session;
}


void sessionlist_insert_front(Session ** session_list_head, Session * new_session){
	if (session_list_head == NULL || new_session == NULL)
		return;

	new_session->nxt = *session_list_head;

	*session_list_head = new_session;
}

void sessionlist_remove(Session ** session_list_head, char * query_sessionID){
	if (session_list_head == NULL || *session_list_head == NULL || query_sessionID == NULL)
		return;

	Session * prev = *session_list_head;

	if (strcmp(prev->sessionID, query_sessionID) == 0){
		*session_list_head = prev->nxt;
		free (prev);
		return;//Assuming that only a unqique sessionID is available
	}

	Session * curr = prev->nxt;
	while (curr != NULL){
		if (strcmp(curr->sessionID, query_sessionID) == 0){
			prev->nxt = curr->nxt;
			free (curr);
			return; //Assuming that only a unique sessioniD is available
		}
		prev = curr;
		curr = curr->nxt;
	}
}

Session * sessionlist_find(Session ** session_list_head, char * query_sessionID){
	if (session_list_head == NULL || *session_list_head == NULL || query_sessionID == NULL)
		return NULL;

	Session * traverser = *session_list_head;
	while (traverser != NULL){
		if (strcmp(traverser->sessionID, query_sessionID) == 0)
			return traverser;
		traverser = traverser->nxt;
	}
	return NULL;
}

Client * sessionlist_find_client (Session * specific_session, Client * queryClient){
	if (specific_session == NULL || queryClient == NULL)
		return NULL;

	if (specific_session->clientsInSession == NULL)
		return NULL;

	return clientlist_find(&(specific_session->clientsInSession), queryClient->clientID);
}



Session * sessionlist_addclient(Session ** session_list_head, char * sessionID, Client * new_client){
	if (session_list_head == NULL || sessionID == NULL || new_client == NULL)
		return NULL;

	Session * specific_session = sessionlist_find(session_list_head, sessionID);

	if (specific_session == NULL)
		return NULL;

	clientlist_insert_front(&(specific_session->clientsInSession),new_client);

	return specific_session;
}

void sessionlist_removeclient (Session **session_list_head, char * sessionID, char * query_clientID){
	if (session_list_head == NULL || sessionID == NULL || query_clientID == NULL)
		return;

	Session * specific_session = sessionlist_find(session_list_head, sessionID);

	if (specific_session == NULL)
		return;

	clientlist_remove( &(specific_session->clientsInSession), query_clientID);
}