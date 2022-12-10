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
#include <zookeeper/zookeeper.h>
#include <unistd.h>

#define INIT 101010
struct rtree_t *rtree;
struct rtree_t *head = NULL;
struct rtree_t *tail = NULL;
char *port_address = NULL;
char *linha;
static int is_connected;
static zhandle_t *zh;
int ZDATALEN = 1024 * 1024;
static char *root_path = "/chain";
typedef struct String_vector zoo_string;
static char *watcher_ctx = "ZooKeeper Data Watcher";
char** children_socks;
zoo_string *children_list; 
static char *chain_path = "/chain"; 


/**
* Watcher function for connection state change events
*/
void client_connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) 
		is_connected = (state == ZOO_CONNECTED_STATE) ? 1 : 0;
}

void set_head_tail() {

	int id = 0;
	char* tmp2 = "node";
	int znode_id_max = INT16_MIN;
	int znode_id_min = INT16_MAX;

	char *max_serv_path = malloc(sizeof(char*));
	char *min_serv_path = malloc(sizeof(char*));
	int max_serv_sock;
	int min_serv_sock; 

	printf("%d\n", children_list->count);

	for (int i = 0; i < children_list->count; i++)  {

		char* tmp = malloc(sizeof(char*));

		//Could be cut. Strcmp can do this kind of compare
		
		strncpy(tmp, children_list->data[i] + strlen(tmp2), 
				strlen(children_list->data[i]) + 1 - strlen("node"));

		id = atoi(tmp);

		printf("Id : %d\n",id);

		printf("Port : %s\n",children_socks[i]);

		if(id > znode_id_max) {
			znode_id_max = id;
			//max_serv_path = children_list->data[i];
			max_serv_sock = atoi(children_socks[i]);

		}

		if(id < znode_id_min) {
			znode_id_min = id;
			//min_serv_path = children_list->data[i];
			min_serv_sock = atoi(children_socks[i]);

		}
	}

	char* head_ip = malloc(sizeof(char*));
	char* tail_ip = malloc(sizeof(char*));
	snprintf(head_ip,120,"127.0.0.1:%d",min_serv_sock);
	snprintf(tail_ip,120,"127.0.0.1:%d",max_serv_sock);
	head = rtree_connect(head_ip);
	tail = rtree_connect(tail_ip);

	if(head == NULL || tail == NULL) {
		fprintf(stderr,"Error on connecting to head or tail/n");
	}
	
	//Basta fazer o set e acho que fica feito
	//rtree = rtree_connect(port_address);
}

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {

	printf("Children watching/n");
	int zoo_data_len = ZDATALEN;

	if (state == ZOO_CONNECTED_STATE)	 {
		if (type == ZOO_CHILD_EVENT) {
		/* Get the updated children and reset the watch */ 
			if (ZOK != zoo_wget_children(zh, chain_path, child_watcher, watcher_ctx, children_list)) {
				fprintf(stderr, "Error setting watch at %s!\n", chain_path);
			}

			for (int i = 0; i < children_list->count; i++)  {

				char tmp[120] = "";
				strcat(tmp,chain_path);
				strcat(tmp,"/");
				strcat(tmp,children_list->data[i]);
				printf("\n %s \n",tmp);

				char *zdatabuf = calloc(1, ZDATALEN);
				if(ZOK != zoo_get(zh,tmp ,0, zdatabuf,&ZDATALEN,NULL)) {

					fprintf(stderr, "Error getting data from %s!\n",children_list->data[i]);
				}
				memcpy(children_socks[i],zdatabuf,sizeof(zdatabuf));
				printf("Buffer : %s\n",children_socks[i]);
				free(zdatabuf);

			} 

			set_head_tail();
		}		
	}
	else {
		printf("Entered .. \n");
		for (int i = 0; i < children_list->count; i++)  {

			char tmp[120] = "";
			strcat(tmp,chain_path);
			strcat(tmp,"/");
			strcat(tmp,children_list->data[i]);
			printf("\n %s \n",tmp);

			char *zdatabuf = calloc(1, ZDATALEN);
			if(ZOK != zoo_get(zh,tmp ,0, zdatabuf,&ZDATALEN,NULL)) {

				fprintf(stderr, "Error getting data from %s!\n",children_list->data[i]);
			}
			memcpy(children_socks[i],zdatabuf,sizeof(zdatabuf));
			printf("Buffer : %s\n",children_socks[i]);
			free(zdatabuf);

		} 

		set_head_tail();
	}

}

void sig_handler(int signum) {
	rtree_disconnect(head);
	rtree_disconnect(rtree);
	rtree_disconnect(tail);
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

	children_list =	(zoo_string *) malloc(sizeof(zoo_string));

	children_socks = malloc(ZDATALEN * 10); //10 servers hardcoded
	for(int i = 0; i < 10; i++) {
		children_socks[i] = malloc(ZDATALEN);
	}
	
	zh = zookeeper_init(port_address, client_connection_watcher, 2000, 0, NULL, 0);

    if (zh == NULL) {
        fprintf(stderr, "Error connecting to ZooKeeper server!\n");
		exit(EXIT_FAILURE);
    }

	sleep(3);

	//if connected here

	if (ZOK != zoo_wget_children(zh, chain_path, &child_watcher, watcher_ctx, children_list)) {
			fprintf(stderr, "Error setting watch at %s!\n", chain_path);
	}

	//Soluçao à pedreiro. Isto é para mudar
	child_watcher(zh, INIT, 0, chain_path, watcher_ctx);

	//set_head_tail();

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

							int status = rtree_put(head,entry);

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
						struct data_t *data = rtree_get(tail, entry_key);

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

						int status = rtree_del(rtree,entry_key);

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
					printf("Tree with size: %d\n", rtree_size(rtree));
					invalid_op = 0;
				}  else if(tok != NULL & strcmp(tok, "height") == 0) {
					printf("Tree with height: %d\n", rtree_height(rtree));
					invalid_op = 0;
				}  else if(tok != NULL & strcmp(tok, "getkeys") == 0) {

					char **keys = rtree_get_keys(rtree);

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

					void **values = rtree_get_values(rtree);
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
						int status = rtree_verify(rtree,atoi(tok));

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
