/*	client.c
 *	This file contains the text conferencing client. 
 */

#include <stdbool.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <ctype.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "utils.h"

#define STDIN 0

int socketFd = -1;					// Socket for connection with conferencing server
char *currentClientID = NULL;
char *currentSessionID = NULL;
bool userWantsToQuit = false;
char buffer[BUFFERLEN] = {0};
unsigned int packetSize = 0;

Packet packet;

/* This struct is used to represent each command that the client understands.
 * 		name - the name of the command
 *		handler - the function that is called to handle that command
 *		info - short explanation of what the command does
 *		args - short explanation of what arguments the command needs
 */

typedef struct {
	const char* name;
	void (*handler)();
	const char* info;
	const char* args;
} Command;

// Forward declarations of all command handler functions
void cmd_login();
void cmd_logout();
void cmd_joinsession();
void cmd_leavesession();
void cmd_createsession();
void cmd_list();
void cmd_quit();
void cmd_help();

// Array of all commands that the client can understand - NULL command marks end of list
const Command commands[] = {
	{"/login", cmd_login, "Log into the server at the given address and port.", "<client ID> <password> <server IP> <server port>"}, 
	{"/logout", cmd_logout, "Exit the server.", ""},
	{"/joinsession", cmd_joinsession, "Join the conference session with the given session ID.", "<session ID>"},
	{"/leavesession", cmd_leavesession, "Leave the currently established session.", ""},
	{"/createsession", cmd_createsession, "Create a new conference session and join it.", "<session ID>"},
	{"/list", cmd_list, "Get a list of connected clients and available sessions.", ""},
	{"/quit", cmd_quit, "Terminate the program.", ""},
	{"/help", cmd_help, "See all available client commands.", ""},
	{NULL, NULL, NULL, NULL}
};

// Handles /login
void cmd_login() {
	const char* clientID = strtok (NULL, " ");
	const char* password = strtok (NULL, " ");
	const char* serverIPAddress = strtok (NULL, " ");
	const char* serverPort = strtok (NULL, " ");

	if (!clientID || !password || !serverIPAddress || !serverPort || strtok (NULL, " ") != NULL) {
		printf ("Usage: /login <client ID> <password> <server IP address> <server port>\n");
	} else {
		// printf("LOGIN command detected.\nClientID: %s, Password: %s, Server IP: %s, Server Port: %s\n", clientID, password, serverIPAddress, serverPort);

		struct addrinfo hints, *serverInfo = NULL;

		memset (&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;

		getaddrinfo (serverIPAddress, serverPort, &hints, &serverInfo);

		if (serverInfo != NULL) {
			socketFd = socket (serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

			if (connect (socketFd, serverInfo->ai_addr, serverInfo->ai_addrlen)) {
				printf ("Could not connect to server.\n");
				socketFd = -1;
				return;
			}

			memset (&packet, 0, sizeof(packet));
			packet.type = LOGIN;
			packet.size = strlen (password);
			snprintf (packet.source, sizeof(packet.source), "%s", clientID);
			snprintf (packet.data, sizeof(packet.data), "%s", password);
			packetSize = create_bytearray (&packet, buffer);
			send (socketFd, buffer, packetSize, 0);
			recv (socketFd, buffer, BUFFERLEN, 0);
			extract_packet (&packet, buffer);
			
			switch (packet.type) {
				case LO_ACK: {
					printf ("Logged in successfully.\n");

					if (currentClientID != NULL) {
						free (currentClientID);
					}

					currentClientID = strdup (clientID);
					break;
				}

				case LO_NAK: {
					printf ("Login unsuccessful. Reason: %s\n", packet.data);
					break;
				}
			}
		} else {
			printf ("No server found at the given address and port.\n");
		}

		freeaddrinfo (serverInfo);
	}

	return;
}

// Handles /logout
void cmd_logout() {
	if (strtok (NULL, " ") != NULL) {
		printf ("Usage: /logout\n");
	} else {
		// printf("LOGOUT command detected.\n");

		if (socketFd != -1) {
			memset (&packet, 0, sizeof(packet));
			packet.type = EXIT;
			packet.size = 0;
			snprintf (packet.source, sizeof(packet.source), "%s", currentClientID);
			packetSize = create_bytearray (&packet, buffer);
			send (socketFd, buffer, packetSize, 0);
			free (currentClientID);
			currentClientID = NULL;
			close (socketFd);
			socketFd = -1;
		} else {
			printf ("Cannot logout - you are not connected to a server.\n");
		}
	}
	
	return;
}

// Handles /joinsession
void cmd_joinsession() {
	const char* sessionID = strtok (NULL, " ");

	if (!sessionID || strtok(NULL, " ") != NULL) {
		printf ("Usage: /joinsession <session ID>\n");
	} else {
		// printf("JOIN SESSION command detected.\nSession ID: %s\n", sessionID);

		memset (&packet, 0, sizeof(packet));
		packet.type = JOIN;
		packet.size = strlen(sessionID);
		snprintf (packet.source, sizeof(packet.source), "%s", currentClientID);
		snprintf (packet.data, sizeof(packet.data), "%s", sessionID);
		packetSize = create_bytearray (&packet, buffer);
		send (socketFd, buffer, packetSize, 0);
	}
	
	return;
}

// Handles /leavesession
void cmd_leavesession() {
	if (strtok (NULL, " ") != NULL) {
		printf ("Usage: /leavesession\n");
	} else {
		// printf("LEAVE SESSION command detected.\n");
		if (currentSessionID != NULL) {
			free(currentSessionID);
			currentSessionID = NULL;

			memset (&packet, 0, sizeof(packet));
			packet.type = LEAVE_SESS;
			packet.size = 0;
			snprintf (packet.source, sizeof(packet.source), "%s", currentClientID);
			packetSize = create_bytearray (&packet, buffer);
			send (socketFd, buffer, packetSize, 0);
		} else {
			printf ("Cannot leave session - you are not part of any session.\n");
		}
		
	}

	return;
}

// Handles /createsession
void cmd_createsession() {
	const char* sessionID = strtok (NULL, " ");

	if (!sessionID || strtok(NULL, " ") != NULL) {
		printf ("Usage: /createsession <session ID>\n");
	} else {
		// printf("CREATE SESSION command detected.\nSession ID: %s\n", sessionID);

		memset (&packet, 0, sizeof(packet));
		packet.type = NEW_SESS;
		packet.size = strlen(sessionID);
		snprintf (packet.source, sizeof(packet.source), "%s", currentClientID);
		snprintf (packet.data, sizeof(packet.data), "%s", sessionID);
		packetSize = create_bytearray (&packet, buffer);
		send (socketFd, buffer, packetSize, 0);
	}
	
	return;
}

// Handles /list
void cmd_list() {
	if (strtok (NULL, " ") != NULL) {
		printf ("Usage: /list\n");
	} else {
		// printf("LIST command detected.\n");

		memset (&packet, 0, sizeof(packet));
		packet.type = QUERY;
		packet.size = 0;
		snprintf (packet.source, sizeof(packet.source), "%s", currentClientID);
		packetSize = create_bytearray (&packet, buffer);
		send (socketFd, buffer, packetSize, 0);
	}

	return;
}

// Handles /quit
void cmd_quit() {
	if (strtok (NULL, " ") != NULL) {
		printf ("Usage: /quit\n");
	} else {
		// printf("QUIT command detected.\n");
		userWantsToQuit = true;
	}

	return;
}

// Handles /help
void cmd_help() {
	if (strtok (NULL, " ") != NULL) {
		printf ("Usage: /help\n");
	} else {
		const Command* currentCommand = commands;

		while (currentCommand->name != NULL) {
			printf("%s %s\n%s\n", currentCommand->name, currentCommand->args, currentCommand->info);
			currentCommand++;
		}
	}

	return;
}

/* Strips leading and trailing whitespace from a string. Replaces all trailing
 * whitespace with null-terminating characters and returns a pointer to the first
 * non-whitespace character in the string. DON'T FREE THE GIVEN STRING UNTIL YOU ARE
 * DONE WITH THE STRIPPED STRING!
 *
 * @param[in]	line 	The line to strip
 * @return		The whitespace-stripped string.
 */

char* strip_whitespace (char *line) {
	char *start = NULL, *end = NULL;

	if (line != NULL) {
		start = line;

		while (isspace(*start)) {
			start++;
		}

		if (*start == '\0') {
			return start;
		}

		end = start + strlen(start) - 1;

		while (end > start && isspace(*end)) {
			*end = '\0';
			end++;
		}
	}

	return start;
}

/* Executes the command in the given string. If no command is found, the client
 * assumes the string is a message to broadcast to users in the session the 
 * client is currently connected to.
 *
 * @param[in]	strippedLine	(Whitespace-stripped) string containing a command
 */

void execute_line (char *strippedLine) {
	if (strippedLine == NULL || *strippedLine == '\0') {
		return;
	}

	if (strippedLine[0] == '/') {
		char* token = strtok (strippedLine, " ");
		const Command* currentCommand = commands;
		bool commandExecuted = false;

		while (currentCommand->name != NULL && commandExecuted == false) {
			if (strcmp(currentCommand->name, token) == 0) {
				commandExecuted = true;
				currentCommand->handler();
			}
			currentCommand++;
		}

		if (commandExecuted == false) {
			printf("Invalid command :(\n");
		}
	} else {
		if (currentSessionID != NULL) {
			printf("Sending the following message to everyone in session ID %s:\n%s\n", currentSessionID, strippedLine);

			memset (&packet, 0, sizeof(packet));
			packet.type = MESSAGE;
			packet.size = strlen (strippedLine);
			snprintf (packet.source, sizeof(packet.source), "%s", currentClientID);
			snprintf (packet.data, sizeof(packet.data), "%s", strippedLine);
			packetSize = create_bytearray (&packet, buffer);
			send (socketFd, buffer, packetSize, 0);
		} else {
			printf("Can't send message - you are not part of any session!\n");
		}
	}

	return;
}

/* Generator function for GNU Readline custom tab completer interface. It generates
 * a list of potential command matches for the given string.
 *
 * @param[in] 	text		The text to complete
 * @param[in] 	state		Used by Readline to indicate how many times
 *							function has been called on the given text
 * @return 		A string containing a potential match
 */

char *tab_generator (const char* text, int state) {
	static const Command *currentCommand;
	static int len;
	const char *commandName;

	if (state == 0) {
		currentCommand = commands;
		len = strlen(text);
	}

	while ((commandName = currentCommand->name)) {
		currentCommand++;

		if (strncmp (commandName, text, len) == 0) {
			return (strdup (commandName));
		}
	}

	return NULL;
}

/* Completion function for GNU Readline custom tab completer interface. It
 * decides when to attempt to complete the command (only if its the first
 * word, etc.).
 *
 * @param	text[in]		Contains the text to complete
 * @param 	start[in]		The start index of the text to complete
 * @param 	end[in]			The end index of the text to complete
 * @return 	An array of potential matches
 */

char **tab_completion (char* text, int start, int end) {
	char **matches = NULL;

	matches = rl_completion_matches (text, &tab_generator);
	return matches;
}

/* Callback function for GNU Readline asynchronous input interface.
 * Gets callled when Readline has one full line of raw input. Line
 * is stripped, added to command history, and executed.
 *
 * @param	rawLine[in]		The raw line of input
 */

void rl_handler (char* rawLine) {
	if (rawLine == NULL) {
		userWantsToQuit = true;
		return;
	}

	char *strippedLine = strip_whitespace(rawLine);

	if (strippedLine != NULL && *strippedLine != '\0') {
		add_history(strippedLine);
		execute_line(strippedLine);
	}

	free (rawLine);
	return;
}

/* Creates a set of file descriptors to pass to the select()
 * system call. The client will have to monitor at most two
 * file descriptors at any time - stdin and the socket for
 * the connection to the text conferencing server if there is one.
 *
 * @param[out]	set 	The file descriptor set to insert the FDs into
 * @return 		One greater than the highest number FD placed in set
 */

int createReadFdSet (fd_set *set) {
	int nfds;
	FD_ZERO (set);
	FD_SET (STDIN, set);
	nfds = STDIN + 1;

	if (socketFd != -1) {
		FD_SET (socketFd, set);
		nfds = (socketFd > STDIN ? socketFd : STDIN) + 1;
	}

	return nfds;
}

// Handles the most recent server response contained in the global buffer.
void handle_server_response () {
	Packet serverResponse;
	
	extract_packet (&serverResponse, buffer);
		
	switch (serverResponse.type) {
		case JN_ACK: {
			printf ("Joined session %s.\n", serverResponse.data);

			if (currentSessionID != NULL) {
				free(currentSessionID);
			}

			currentSessionID = strdup((char *)serverResponse.data);
			break;
		}

		case JN_NAK: {
			char sessionID[MAX_DATA_SIZE] = {0};
			char reasonForFailure[MAX_DATA_SIZE] = {0};

			sscanf ((char *)serverResponse.data, "%[^,]%*[,]%[^\n]", sessionID, reasonForFailure);
			printf ("Failed to join session %s. Reason: %s\n", sessionID, reasonForFailure);
			break;
		}

		case NS_ACK: {
			printf ("Created session %s.\n", serverResponse.data);
			break;
		}

		case QU_ACK: {
			printf ("Here's a list of users and sessions: %s\n", serverResponse.data);
			break;
		}

		case MESSAGE: {
			printf ("%s: %s\n", serverResponse.source, serverResponse.data);
			break;
		}
	}

	return;
}

int main () {
	rl_attempted_completion_function = (rl_completion_func_t *)tab_completion;
	fd_set readFdSet;
	FD_ZERO (&readFdSet);

	printf ("Text Conferencing Client. Type /help to see a list of available commands.\n");
	rl_callback_handler_install ("> ", (rl_vcpfunc_t*)&rl_handler);

	while (userWantsToQuit == false) {
		int nfds = createReadFdSet(&readFdSet);
		select (nfds, &readFdSet, NULL, NULL, NULL);

		if (FD_ISSET (STDIN, &readFdSet)) {						
			// Keyboard input is waiting - read a character from the terminal
			rl_callback_read_char();
		}

		if (FD_ISSET (socketFd, &readFdSet)) {			
			// Server sent something
			unsigned int bytesReceived;
			if ((bytesReceived = recv (socketFd, buffer, BUFFERLEN, 0))) {
				buffer[bytesReceived] = '\0';
				// printf ("Received: %s\n", buffer);
				handle_server_response ();
			} else {
				socketFd = -1;
			}
		}
	}

	// Uninstall callback_handler so Readline doesn't keep calling rl_handler
	// after the program has terminated
	rl_callback_handler_remove();
	return 0;
}