/* server.c
 * This file implements the text conferencing server.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include "utils.h"

#define BACKLOG 1

void handleClient (int fd) {
	int socket = fd;
	char buffer[BUFFERLEN];
	int bytesReceived;

	while ((bytesReceived = recv (socket, buffer, BUFFERLEN, 0))) {
		buffer[bytesReceived] = '\0';
		printf ("Received: %s\n", buffer);
		send (socket, buffer, bytesReceived, 0);
	}

	printf ("Client logged out.\n");
}

int main (int argc, char **argv) {
	
	if (argc != 2) {
		printf ("Usage: server <listen port>\n");
		return 1;
	}

	struct addrinfo hints, *serverInfo;

	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo (NULL, argv[1], &hints, &serverInfo);

	if (serverInfo != NULL) {
		int listenSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
		bind (listenSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);

		freeaddrinfo (serverInfo);

		while (1) {
			struct sockaddr_storage their_addr;
			socklen_t their_addrlen = sizeof(their_addr);

			listen (listenSocket, BACKLOG);
			long newFd = accept (listenSocket, (struct sockaddr *)&their_addr, &their_addrlen);

			handleClient (newFd);
		}
	}

	return 0;
}