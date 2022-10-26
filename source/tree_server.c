/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
*/

#include "network_server.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

int main(int argc, char **argv) {

	int short socket_num;
	int socket,connsocket;
	char *endptr;

	if(argc != 2) {
		perror("Too many or too few arguments passed. Please, only pass the port number\n");
		exit(-1);
	}

	socket_num = (short) strtol(argv[1],&endptr,10);

	if(strlen(endptr) > 0) {
		perror("Argument passed do not match with any integer. Please confirm"
		"that what you are passing is a valid port number\n");
		exit(-1);
	}

	socket = network_server_init((short) socket_num);

	if(socket == -1) {
		exit(-1);
	}

	connsocket = 0;

	while(connsocket != -1) {

		connsocket = network_main_loop(socket);

	}

	network_server_close();

	return 0;

}
