/* utils.c
 * This file contains implementations of utils.h functions
 */

#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

unsigned int create_bytearray (Packet* packet, char* buffer) {
	if (packet == NULL || buffer == NULL) {
		return 0;
	}

	unsigned int packetSize;
	packetSize = snprintf (buffer, BUFFERLEN, "%d:%d:%s:%s\n", packet->type, packet->size, packet->source, packet->data);
	return packetSize;
}

void extract_packet (Packet* packet, const char* bytearray) {
	if (packet == NULL || bytearray == NULL) {
		return;
	}

	sscanf (bytearray, "%d%*[:]%d%*[:]%[^:\n]%*[:]%[^\n]", &(packet->type), &(packet->size), packet->source, packet->data);
	return;
}

ConfigLine* read_config () {
	static char first_time = 0;
	static FILE *conf_file = NULL;
	ConfigLine *next_client = (ConfigLine*) malloc (sizeof(ConfigLine));

	if (first_time == 0) {
		conf_file = fopen ("clients.conf", "r");
		first_time++;
	}

	if (feof (conf_file)) {
		fclose(conf_file);
		return NULL;
	}

	fscanf (conf_file, "%s %s", next_client->clientID, next_client->password);

	return next_client;
}