#ifndef SESSIONLIST_H
#define SESSIONLIST_H

#include "clientlist.h"


typedef struct session {
	char * sessionID;
	Client ** clientsInSession;
	struct session * nxt;
} Session;



#endif