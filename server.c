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

#define BACKLOG 1

void* handleClient (void *fd) {
	long socket = (long)fd;
	char buf[1000];
	int bytesReceived;

	while ((bytesReceived = recv (socket, buf, 1000, 0))) {
		printf ("Received: %s\n", buf);
		send (socket, buf, bytesReceived, 0);
	}

	printf ("Client logged out.\n");
	pthread_exit(NULL);
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
			pthread_t newThread;

			listen (listenSocket, BACKLOG);
			long newFd = accept (listenSocket, (struct sockaddr *)&their_addr, &their_addrlen);

			pthread_create(&newThread, NULL, handleClient, (void *)newFd);
		}
	}

	return 0;
}