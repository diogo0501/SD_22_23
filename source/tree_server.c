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

	//Existe mais um novo arg - tree <port> <N> onde N é o numero de threads secundarias a ser lançadas pela main thread

	int short socket_num;
	int socket,connsocket;
	char *endptr;
	int n_threads;
	char *endptr2;

	if(argc != 3) {
		perror("Invalid number of arguments, try: ./tree-server <port> <N>\n");
		exit(-1);
	}

	socket_num = (short) strtol(argv[1],&endptr,10);

	if(strlen(endptr) > 0) {
		perror("Socket port passed do not match with any integer\n");
		exit(-1);
	}

	n_threads = strtol(argv[2],&endptr2,10);

	if(strlen(endptr2) > 0) {
		perror("Number of threads do not match with any integer\n");
		exit(-1);
	}

	tree_skel_init(n_threads);
	socket = network_server_init((short) socket_num);

	if(socket == -1) {
		exit(-1);
	}

	connsocket = 0;

	while(connsocket != -1) {

		connsocket = network_main_loop(socket);

	}

	network_server_close();
	tree_skel_destroy();

	return 0;

}
