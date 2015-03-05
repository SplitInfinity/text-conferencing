#ifndef SESSIONLIST_H
#define SESSIONLIST_H

#include "clientlist.h"


typedef struct session {
	char * sessionID;
	Client ** clientsInSession;
	struct session * nxt;
} Session;

Session * create_session (char * sessionID);

void sessionlist_insert_front(Session ** session_list_head, Session * new_session);

void sessionlist_remove(Session ** session_list_head, char * query_sessionID);

Session * sessionlist_find(Session ** session_list_head, char * query_sessionID);

void sessionlist_addclient(Session * specific_session, Client * new_client);



#endif