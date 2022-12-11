/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
 */

#include "data.h"
#include "entry.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <zookeeper/zookeeper.h> 

struct rtree_t *rtree = NULL;
char *port_address = NULL;
char *linha;

void sig_handler(int signum) {
	rtree_disconnect(rtree);
	free(linha);
	exit(-1);
}

int main(int argc, char **argv) {

	signal(SIGINT, sig_handler);	
	signal(SIGPIPE,SIG_IGN);

	if(argc != 2) {
		printf("Invalid number of arguments, try: ./tree_client <address:port>");
	}

	port_address = malloc(50);
	strcpy(port_address, argv[1]);
	linha = malloc(1000);

	rtree = rtree_connect(port_address);
	printf("Print de teste: Depois do connect \n");
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

							struct data_t *data = data_create2(strlen(str) + 1, (void*)str);
							struct entry_t *entry = entry_create(entry_key, data);

							int status = rtree_put(rtree->head,entry);

							if(status == -1) {
								printf("Error trying to execute operation 'put'\n");
							}
							else {
								printf("Last assigned: %d\n",status);
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
						struct data_t *data = rtree_get(rtree->tail, entry_key);

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

						int status = rtree_del(rtree->head,entry_key);

						if(status == -1) {
							printf("Error trying to execute operation 'del'\n");
						} else {
							printf("Last assigned: %d\n",status);
						}
						free(entry_key);
						invalid_op = 0;
					} else {
						printf("Invalid input format required to execute operation 'del'\n");
					}
				}  else if(tok != NULL & strcmp(tok, "size") == 0) {
					printf("Tree with size: %d\n", rtree_size(rtree->tail));
					invalid_op = 0;
				}  else if(tok != NULL & strcmp(tok, "height") == 0) {
					printf("Tree with height: %d\n", rtree_height(rtree->tail));
					invalid_op = 0;
				}  else if(tok != NULL & strcmp(tok, "getkeys") == 0) {

					char **keys = rtree_get_keys(rtree->tail);

					if (keys[0] == NULL) {
						printf("There is currently no nodes in the tree\n");
					}
					else {
						int i = 0;

						while(keys[i] != NULL) {
							printf("Key : %s\n", keys[i]);
							i++;
						}
					}
					
					for(int i = 0; keys[i] != NULL; i++) {
						free(keys[i]);
					}
					invalid_op = 0;
					free(keys);
				}  else if(tok != NULL & strcmp(tok, "getvalues") == 0) {

					void **values = rtree_get_values(rtree->tail);
					int i = 0;

					if(values[0] == NULL) {
						printf("There is currently no nodes in the tree\n");
					}else {
						while(values[i] != NULL) {
							printf("Value : %s\n",(char*) values[i]);
							i++;
						}
					}
					invalid_op = 0;

					for(int i = 0; values[i] != NULL; i++) {
							free(values[i]);
					}
					free(values);

				}
				else if(tok != NULL & strcmp(tok,"verify") == 0) {
					tok = strtok(NULL, " ");

					if(tok != NULL) {
						int status = rtree_verify(rtree->tail,atoi(tok));

						if(status == -1) {
							printf("There has been an error on verifying the operation\n");
						}
						else if(status == 0) {
							printf("Writting operation was not performed yet\n");
						}
						else if (status == 1) {
							printf("Writting operation was performed\n");
						} else {
							printf("Invalid number of operation, must be greater than 0\n");
						}

						invalid_op = 0;
					}
					else{
						printf("Invalid input format required to execute operation 'verify'\n");
					}
				}
				else if(tok != NULL & strcmp(tok, "quit") == 0) {

					free(op_args);
					network_close(rtree);
					free(linha);
					rtree_disconnect(rtree);
					return 0;

				}
				else {
					printf("Invalid operation\n");
					free(op_args);
				}

			}

		}
		free(op_args);
	}
	free(linha);
	free(port_address);
}