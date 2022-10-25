/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
 */

#include "data.h"
#include "entry.h"
#include "client_stub.h"
#include "network_client.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct rtree_t *rtree = NULL;
char *port_address = NULL;

void sig_handler(int signum) {
	rtree_disconnect(rtree);
	free(port_address);
	exit(-1);
}

int main(int argc, char **argv) {

	signal(SIGPIPE, sig_handler);                                                       // o gnu do windows n esta a reconhecer o sig SIGPIPE
	signal(SIGINT, sig_handler);

	if(argc != 2) {
		printf("Invalid number of arguments, try: ./tree_client <address:port>");
	}

	port_address = malloc(50);
	strcpy(port_address, argv[1]);
	char linha[1000];

	rtree = rtree_connect(port_address);

	while(1) {
		int invalid_op = 1;
		char* op_args;
		while(invalid_op == 1) {
			printf("> ");
			if(fgets(linha, 1000, stdin) != NULL) {

				op_args = malloc(strlen(linha) + 1);
				strcpy(op_args, linha);

				if(strlen(op_args) != 0) {
					op_args[strlen(op_args) - 1] = '\0';
				}

				char *tok = strtok(op_args, " ");

				if(tok != NULL & strcmp(tok, "put") == 0) {

					tok = strtok(NULL, " ");

					if(tok != NULL) {

						char *entry_key = malloc(strlen(tok) + 1);
						strcpy(entry_key, tok);
						tok = strtok(NULL, "");

						if(tok != NULL) {

							char *str = malloc(strlen(tok) + 1);
							strcpy(str, tok);
							//printf("%s\n",str);

							struct data_t *data = data_create2(strlen(str) + 1, (void*)str);
							struct entry_t *entry = entry_create(entry_key, data);

							if(rtree_put(rtree, entry) != 0) {
								printf("Error trying to execute operation 'put'\n");
							}

							entry_destroy(entry);
							invalid_op = 0;

						}
						else{
							printf("Invalid input format required to execute operation 'put'\n");
						}
					}
					else {

						printf("Invalid input format required to execute operation 'put'\n");
					}

				} else if(tok != NULL & strcmp(tok, "get") == 0) {

					tok = strtok(NULL, " ");

					if(tok != NULL) {

						char *entry_key = malloc(strlen(tok) + 1);
						strcpy(entry_key, tok);
						struct data_t *data = rtree_get(rtree, entry_key);

						if(data == NULL) {
							printf("Couldnt get data with given key\n");
						} else {
							printf("Data : %s\n", (char*) data->data);
							data_destroy(data);
						}

						free(entry_key);
						invalid_op = 0;
					} else {
						printf("Invalid input format required to execute operation 'get'\n");
					}
				} else if(tok != NULL & strcmp(tok, "del") == 0) {

					tok = strtok(NULL, " ");

					if(tok != NULL) {

						char *entry_key = malloc(strlen(tok) + 1);
						strcpy(entry_key, tok);

						if(rtree_del(rtree, entry_key) == 0) {
							printf("Data with given key has been removed from the tree\n");
						} else {
							printf("Error / Couldnt delete data with given key from the tree\n");
						}
						free(entry_key);
						invalid_op = 0;
					} else {
						printf("Invalid input format required to execute operation 'del'\n");
					}
				}  else if(tok != NULL & strcmp(tok, "size") == 0) {
					printf("Tree with size: %d\n", rtree_size(rtree));
					invalid_op = 0;
				}  else if(tok != NULL & strcmp(tok, "height") == 0) {
					printf("Tree with height: %d\n", rtree_height(rtree));
					invalid_op = 0;
				}  else if(tok != NULL & strcmp(tok, "getkeys") == 0) {

					char **keys = rtree_get_keys(rtree);

					if (keys == NULL) {
						printf("There is currently no nodes in the tree\n");
					}
					else {
						int i = 0;

						while(keys[i] != NULL) {
							printf("Key : %s\n", keys[i]);
							i++;
						}
					}
					invalid_op = 0;
					free(keys);
				}  else if(tok != NULL & strcmp(tok, "getvalues") == 0) {

					void **values = rtree_get_values(rtree);
					int i = 0;

					if (values == NULL) {
						printf("There is currently no nodes in the tree\n");
					}
					else {
						while(values[i] != NULL) {
							printf("Value : %s\n",(char*) values[i]);
							i++;
						}
					}
					invalid_op = 0;
					free(values);

				}
				else if(tok != NULL & strcmp(tok, "quit") == 0) {

					free(op_args);
					invalid_op = 0;

				}
				else {
					printf("Invalid operation\n");
				}

			}

		}
		free(op_args);
		network_close(rtree);
		network_connect(rtree);


	}
	free(port_address);
}
