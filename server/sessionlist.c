#include <stdlib.h>
#include <string.h>
#include "sessionlist.h"

#define MAX_SESSIONID_LEN 50



Session * create_session(char * sessionID){
	Session * new_session = (Session *) malloc (sizeof(Session));

	new_session->sessionID = (char *) malloc(MAX_SESSIONID_LEN*sizeof(char));
	strcpy(new_session->sessionID, sessionID);
	new_session->clientsInSession = NULL;
	new_session->nxt = NULL;
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
	while (traverser->nxt != NULL){
		if (strcmp(traverser->sessionID, query_sessionID) == 0)
			return traverser;
		traverser = traverser->nxt;
	}
	return NULL;
}

void sessionlist_addclient(Session * specific_session, Client * new_client){


}