#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>//What is the types for?
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>

#define BACKLOG 20			// the number of pending connections queue will hold (most systems use 20)





void waitFor (unsigned int secs) {
    int retTime = time(0) + secs;     // Get finishing time.
    while (time(0) < retTime);    // Loop until it arrives.
}

/*
 * Client Handler
 *
 */
void * client_handler(void * conn_sock){		//DOes this HAVE to be a function pointer?
	int client_sock = *((int*) conn_sock); 	// Get the client socket

	int msg_len = strlen("Hi from Arash");
	send (client_sock, "Hi from Arash\n", msg_len+1, 0);
	
	int n = 0;
	while(1){
		waitFor(1);
		
		char * temp = (char*)malloc(10000*sizeof(char));
		sprintf(temp, "ping!%d\n",n);
		msg_len = strlen(temp);
		send (client_sock, temp, msg_len+1, 0);
		n++;
	}
	close(client_sock);
	return 0;
}



int main (int argc, char ** arv){
	//unsigned int server_port;
	int sockfd, new_sockfd;
	struct addrinfo hints, *serverinfo;
	struct sockaddr_storage connector_addr;
	socklen_t sockaddr_size; 
	//struct sigaction sa;

	//Ensure proper usage
	if (argc != 2){
		printf("Usage: server <TCP port to listen on>\n");
		return 1;
	}
	printf("Establishing teleconference server...\n");

	//Extract server port#
	//server_port = atoi(arv[1]);

	//Establish the connection
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM; 		//To specify TCP
	hints.ai_flags = AI_PASSIVE; 			// It will fill up our IP adress for us

	int status;
	if ((status = getaddrinfo(NULL, arv[1], &hints, &serverinfo)) != 0){
		fprintf(stderr, "Server Error: getaddrinfo error %s\n", gai_strerror(status));
		return 1; 							//getaddrinfo() failed; we must close the server
	}


	//Create our socket
	sockfd = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
	if (sockfd == -1){
		fprintf(stderr, "Server Error: socket creation error \n");
		return 1;
	}

	//We want to listen on our server port, so we bind
	if (bind(sockfd, serverinfo->ai_addr, serverinfo->ai_addrlen) == -1){
		fprintf(stderr, "Server Error: binding socket error\n");
		return 1;
	}

	//Lets listen for incoming connections
	if (listen(sockfd, BACKLOG) == -1){
		//Change this part later on so it is correct but for now
		fprintf(stderr, "Server: listen error \n");
		return 1;
	}

	pthread_t connection;
	sockaddr_size = sizeof(connector_addr);
	
	while (1) {
		new_sockfd = accept(sockfd, (struct sockaddr *) &connector_addr, &sockaddr_size);

		pthread_create(&connection, NULL, client_handler, (void*) &new_sockfd);


	}

}