#ifndef SESSIONLIST_H
#define SESSIONLIST_H

#include "clientlist.h"


typedef struct session {
	char * sessionID;
	ClientNode * clientsInSession;
	struct session * nxt;
} Session;

Session * create_session (char * sessionID);

void sessionlist_insert_front(Session ** session_list_head, Session * new_session);

void sessionlist_remove(Session ** session_list_head, char * query_sessionID);

Session * sessionlist_find(Session ** session_list_head, char * query_sessionID);

Session * sessionlist_addclient(Session ** session_list_head, char * sessionID, Client * new_client);

void sessionlist_removeclient (Session **session_list_head, char * sessionID, char * query_clientID);

void sessionlist_init ();

void sessionlist_termin ();

#endif