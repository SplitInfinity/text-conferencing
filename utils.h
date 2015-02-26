/* utils.h
 * This file contains definitions for utility functions, structs and packet-related macros that are used by both the client and server.
 */

 #ifndef UTILS_H
 #define UTILS_H

#define BUFFERLEN 200
#define MAX_CLIENT_ID_SIZE 20
#define MAX_DATA_SIZE 100

// Macros for packet type
#define LOGIN 1 		// Login
#define LO_ACK 2 		// Acknowledge login	
#define LO_NAK 3        // Negative acknowledgement of login
#define EXIT 4          // Exit from the server
#define JOIN 5          // Join a conference session
#define JN_ACK 6   		// Acknowledge session join
#define JN_NAK 7        // Negative acknowledgment of session join
#define LEAVE_SESS 8 	// Leave session
#define NEW_SESS 9      // Create session
#define NS_ACK 10   	// Acknowledge new conference session
#define MESSAGE 11		// Send message/Display message
#define QUERY 12        // Get list of users and sessions
#define QU_ACK 13       // Acknowledge QUERY and send data

/*
 * Struct that represents a packet sent by the client and server
 * 	type - packet type (see macros above)
 *	size - length of data in packet
 *  source - client ID of sender
 *  data - actual data
 */

typedef struct {
	unsigned int type;
	unsigned int size; 
	unsigned char source[MAX_CLIENT_ID_SIZE];
	unsigned char data[MAX_DATA_SIZE];
} Packet;

#endif