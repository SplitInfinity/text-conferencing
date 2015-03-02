#include <stdlib.h>
#include "clientlist.h"

Client * create_client (char* clientID, char* password, char* currentSessionID, char* ipAddress, unsigned int port, int socket){
	Client * newClient = (Client *) malloc (sizeof(Client));
	
}


Client * client_insert_front (Client * head){
	if (head == NULL)
		return NULL;


}


Client * client_remove (Client * head){

}

Client * client_find (char* clientID){

}


