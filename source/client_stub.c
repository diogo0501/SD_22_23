/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
*/

#include "message-private.h"
#include "network_client.h"
#include "sdmessage.pb-c.h"
#include "client_stub-private.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <zookeeper/zookeeper.h> 


/* Remote tree. A definir pelo grupo em client_stub-private.h
 */
struct rtree_t;

#define INIT 101010
static int is_connected;
static zhandle_t *zh;
int ZDATALENGTH = 1024 * 1024;
static char *root_path = "/chain";
typedef struct String_vector zoo_string;
static char *watcher_ctx = "ZooKeeper Data Watcher";
char** children_socks;
zoo_string *children_list; 
static char *chain_path = "/chain";
struct rtree_t *rtree;

/**
* Watcher function for connection state change events
*/
void client_connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) 
		is_connected = (state == ZOO_CONNECTED_STATE) ? 1 : 0;
}

// void set_head_tail() {

// 	int id = 0;
// 	char* tmp2 = "node";
// 	int znode_id_max = INT16_MIN;
// 	int znode_id_min = INT16_MAX;

// 	char *max_serv_path = malloc(sizeof(char*));
// 	char *min_serv_path = malloc(sizeof(char*));
// 	int max_serv_sock;
// 	int min_serv_sock; 

// 	printf("%d\n", children_list->count);

// 	for (int i = 0; i < children_list->count; i++)  {

// 		char* tmp = malloc(sizeof(char*));

// 		//Could be cut. Strcmp can do this kind of compare
		
// 		strncpy(tmp, children_list->data[i] + strlen(tmp2), 
// 				strlen(children_list->data[i]) + 1 - strlen("node"));

// 		id = atoi(tmp);

// 		printf("Id : %d\n",id);

// 		printf("Port : %s\n",children_socks[i]);

// 		if(id > znode_id_max) {
// 			znode_id_max = id;
// 			max_serv_path = children_list->data[i];
// 			max_serv_sock = atoi(children_socks[i]);

// 		}

// 		if(id < znode_id_min) {
// 			znode_id_min = id;
// 			//min_serv_path = children_list->data[i];
// 			min_serv_sock = atoi(children_socks[i]);

// 		}
// 	}

// 	char* head_ip = malloc(sizeof(char*));
// 	char* tail_ip = malloc(sizeof(char*));
// 	snprintf(head_ip,120,"127.0.0.1:%d",min_serv_sock);
// 	snprintf(tail_ip,120,"127.0.0.1:%d",max_serv_sock);
// 	rtree->head = rtree_connect(head_ip);
// 	rtree->tail = rtree_connect(tail_ip);

// 	if(rtree->head == NULL || rtree->tail == NULL) {
// 		fprintf(stderr,"Error on connecting to head or tail/n");
// 	}
	
// 	//Basta fazer o set e acho que fica feito
// 	//rtree = rtree_connect(port_address);
// }

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {

	printf("Children watching \n");
	int zoo_data_len = ZDATALENGTH;

    if (state == ZOO_CONNECTED_STATE) {
        if (type == ZOO_CHILD_EVENT) {

            if (ZOK != zoo_wget_children(zh, chain_path, NULL, watcher_ctx, children_list)) {
                        fprintf(stderr, "Error setting watch at %s!\n", chain_path);
                }

            int id = 0;
            char* tmp2 = "node";
            int znode_id_max = INT16_MIN;
            int znode_id_min = INT16_MAX;

            char *max_serv_path = malloc(sizeof(char*));
            char *min_serv_path = malloc(sizeof(char*));
            int max_serv_sock;
            int min_serv_sock; 

            printf("Numero de filhos de chain %d\n", children_list->count);

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
                    max_serv_path = children_list->data[i];
                    max_serv_sock = atoi(children_socks[i]);

                }

                if(id < znode_id_min) {
                    znode_id_min = id;
                    min_serv_path = children_list->data[i];
                    min_serv_sock = atoi(children_socks[i]);

                }
            }
            
            //Soluçao à pedreiro. Isto é para mudar
            //child_watcher(zh, INIT, 0, chain_path, watcher_ctx);

            //set_head_tail();

            rtree = malloc(sizeof(struct rtree_t));

            rtree->head_path = min_serv_path;
            rtree->tail_path = max_serv_path;
            
            if (ZOK != zoo_wget_children(zh, chain_path, &child_watcher, watcher_ctx, NULL)) {
                printf("Couldnt set watch at /chain!\n");
            }
        }
    }
}

char **connect_to_head_and_tail() {

	char *cat1 = malloc(sizeof(char) * 120);
	strcat(cat1,"/chain/");
	strcat(cat1,rtree->head_path);
	char *cat2 = malloc(sizeof(char) * 120);
	strcat(cat2,"/chain/");
	strcat(cat2,rtree->tail_path);
    int headExists = zoo_exists(zh, cat1, 0, NULL);
    int tailExists = zoo_exists(zh, cat2, 0, NULL);
	printf("Print de teste: rtree->head_path %s \n", rtree->head_path);
	printf("Print de teste: rtree->tail_path %s \n", rtree->tail_path);
	printf("Print de teste: headExists %d \n", headExists);
	printf("Print de teste: tailExists %d \n", tailExists);
    char **buffer = malloc(2 * sizeof(char *));
    buffer[0] = malloc((ZDATALENGTH * sizeof(char)) + 1);
    buffer[1] = malloc((ZDATALENGTH * sizeof(char)) + 1);
    char *zdata_buf = (char *)calloc(1, ZDATALENGTH * sizeof(char));
    char *zdata_buf2 = (char *)calloc(1, ZDATALENGTH * sizeof(char));

    if (headExists == ZOK) {
	    printf("Print de teste: headExists is true \n");
        if (ZOK == zoo_get(zh, cat1, 0, zdata_buf, &ZDATALENGTH, NULL)) {
            buffer[0] = zdata_buf;
        } else {
            printf("Couldnt get head server data\n");
            free(zdata_buf);

        }
    }

    if (tailExists == ZOK) {
	    printf("Print de teste: tailExists is true \n");
        if (ZOK == zoo_get(zh, cat2, 0, zdata_buf2, &ZDATALENGTH, NULL)) {
            buffer[1] = zdata_buf2;
        } else {
            printf("Couldnt get tail server data\n");
            free(zdata_buf2);
        }
    }

	printf("Print de teste: saiu do connect_to_head_and_tail \n");
    return buffer;
}

/* Função para estabelecer uma associação entre o cliente e o servidor, 
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtree_t *rtree_connect(const char *address_port){

	children_list =	(zoo_string *) malloc(sizeof(zoo_string));

	children_socks = malloc(ZDATALENGTH * 10); //10 servers hardcoded
	for(int i = 0; i < 10; i++) {
		children_socks[i] = malloc(ZDATALENGTH);
	}
	
	zh = zookeeper_init(address_port, client_connection_watcher, 2000, 0, NULL, 0);

    if (zh == NULL) {
        fprintf(stderr, "Error connecting to ZooKeeper server!\n");
		exit(EXIT_FAILURE);
    }

	sleep(3);

	//if connected here

	if (ZOK != zoo_wget_children(zh, chain_path, &child_watcher, watcher_ctx, children_list)) {
			fprintf(stderr, "Error setting watch at %s!\n", chain_path);
	}

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
			max_serv_path = children_list->data[i];
			max_serv_sock = atoi(children_socks[i]);

		}

		if(id < znode_id_min) {
			znode_id_min = id;
			min_serv_path = children_list->data[i];
			min_serv_sock = atoi(children_socks[i]);

		}
	}
    
	//Soluçao à pedreiro. Isto é para mudar
	//child_watcher(zh, INIT, 0, chain_path, watcher_ctx);

	//set_head_tail();

    rtree = malloc(sizeof(struct rtree_t));
    rtree->head = malloc(sizeof(struct rtree_t));
    rtree->tail = malloc(sizeof(struct rtree_t));

    rtree->head_path = min_serv_path;
    rtree->tail_path = max_serv_path;
    // rtree->head = NULL;
    // rtree->tail = NULL;

    char **server_infos = connect_to_head_and_tail();
	printf("Print de teste: server addr %s \n", server_infos[0]);
    // char *tok = strtok(server_infos[0], ":");
    char *a = server_infos[0];
	printf("Print de teste: server ip %s \n", a);
    if(a == NULL) {
        return NULL;
    }
	printf("Antes do ip\n");
    rtree->head->ip = "10.0.2.15";      //tive de meter o ip hardcoded pq ja nao eh passado nenhum ip como argumento nesta fase
	printf("depois do ip\n");
    // tok = strtok(NULL, ":");

    if(a == NULL) {
        return NULL;
    }
    rtree->head->port = atoi(a);
	printf("Print de teste: server port %d\n", rtree->head->port);

    int conectionStatus = network_connect(rtree->head);

    if(conectionStatus == -1) {
        rtree_disconnect(rtree->head);
        return NULL;
    }

    // char *tok2 = strtok(server_infos[1], ":");
    char *b = server_infos[1];
    if(b == NULL) {
        return NULL;
    }

    rtree->tail->ip = "10.0.2.15"; //tive de meter o ip hardcoded pq ja nao eh passado nenhum ip como argumentonesta fase

    // tok2 = strtok(NULL, ":");

    if(b == NULL) {
        return NULL;
    }

    rtree->tail->port = atoi(b);

    int conectionStatus2 = network_connect(rtree->tail);

    if(conectionStatus2 == -1) {
        rtree_disconnect(rtree->tail);
        return NULL;
    }

    return rtree; 

}

/* Termina a associação entre o cliente e o servidor, fechando a 
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(struct rtree_t *rtree) {
    
    if(rtree == NULL) {
        return -1;
    }

    int a = network_close(rtree);
    free(rtree->ip);
    free(rtree);
    return a;
}

/* Função para adicionar um elemento na árvore.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtree_put(struct rtree_t *rtree, struct entry_t *entry) {

    int last_assigned;
    
    if(rtree == NULL || entry == NULL) {
        return -1;
    }

    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;

    msg->entry = malloc(sizeof(MessageT__EntryT));
    message_t__entry_t__init(msg->entry);
    msg->entry->key = entry->key;
    msg->entry->data.data = (uint8_t *)entry->value->data;
    msg->entry->data.len = entry->value->datasize;

    struct message_t *msg_wrapper = malloc(sizeof(struct message_t));
	msg_wrapper->recv_msg = msg;

    struct message_t *resp = network_send_receive(rtree, msg_wrapper);
    if(resp->recv_msg == NULL) {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg_wrapper);
        free(msg->entry);
        free(msg);
        return -1;
    }

    if(resp->recv_msg->c_type == MESSAGE_T__C_TYPE__CT_RESULT || resp->recv_msg->opcode == MESSAGE_T__OPCODE__OP_PUT + 1) {
        last_assigned = resp->recv_msg->datalength;
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg_wrapper);
        free(msg->entry);
        free(msg);
        return last_assigned;
    } else {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg_wrapper);
        free(msg->entry);
        free(msg);
        return -1;
    }
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtree_get(struct rtree_t *rtree, char *key) {

    if(rtree == NULL || key == NULL) {
        return NULL;
    }

    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_GET;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg->data.data = key;
    msg->data.len = strlen(key) + 1;
    msg->datalength = strlen(key) + 1;

    struct message_t *msg_wrapper = malloc(sizeof(struct message_t));
	msg_wrapper->recv_msg = msg;
    struct message_t *resp = network_send_receive(rtree, msg_wrapper);

    if(resp->recv_msg == NULL) {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg);
        free(msg_wrapper);
        return NULL;
    }

    if(resp->recv_msg->c_type == MESSAGE_T__C_TYPE__CT_VALUE || resp->recv_msg->opcode == MESSAGE_T__OPCODE__OP_GET + 1) {
        
        int data_size = resp->recv_msg->data.len;

        if (data_size == 0){
            message_t__free_unpacked(resp->recv_msg, NULL);
            free(resp);
            free(msg);
            free(msg_wrapper);
            return NULL;
        }

        char *value = malloc(data_size + 1);
        memcpy(value, resp->recv_msg->data.data, data_size);
        struct data_t *data = data_create2(data_size, value);

        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg);
        free(msg_wrapper);

        return data;
    } else {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg);
        free(msg_wrapper);
        return NULL;
    }
}

/* Função para remover um elemento da árvore. Vai libertar 
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtree_del(struct rtree_t *rtree, char *key) {

    int last_assigned;
    if(rtree == NULL || key == NULL) {
        return -1;
    }

    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg->data.data = key;
    msg->data.len = strlen(key) + 1;

    struct message_t *msg_wrapper = malloc(sizeof(struct message_t));
	msg_wrapper->recv_msg = msg;

    struct message_t *resp = network_send_receive(rtree, msg_wrapper);

    if(resp == NULL) {
    	return -1;
    }

    if(resp->recv_msg == NULL) {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg);
        free(msg_wrapper);
        return -1;
    }

    if(resp->recv_msg->c_type == MESSAGE_T__C_TYPE__CT_RESULT || resp->recv_msg->opcode == MESSAGE_T__OPCODE__OP_DEL + 1) {
        last_assigned = resp->recv_msg->datalength;
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg);
        free(msg_wrapper);
        return last_assigned;
    } else {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg);
        free(msg_wrapper);
        return -1;
    }
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t *rtree) {

    if(rtree == NULL) {
        return -1;
    }

    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg->datalength = 0;
    
    struct message_t *msg_wrapper = malloc(sizeof(struct message_t)); 
	msg_wrapper->recv_msg = msg;
    struct message_t *resp = network_send_receive(rtree, msg_wrapper);

    if(resp->recv_msg == NULL) {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg);
        return -1;
    }

    if(resp->recv_msg->c_type == MESSAGE_T__C_TYPE__CT_RESULT || resp->recv_msg->opcode == MESSAGE_T__OPCODE__OP_SIZE + 1) {
        int data_size = resp->recv_msg->datalength;
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(msg_wrapper);
        free(resp);
        free(msg);
        return data_size;  
    } else {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(msg_wrapper);
        free(resp);
        free(msg);
        return -1;
    }
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t *rtree) {

    if(rtree == NULL) {
        return -1;
    }

    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg->datalength = 0;
    
    struct message_t *msg_wrapper = malloc(sizeof(struct message_t));
	msg_wrapper->recv_msg = msg;
    struct message_t *resp = network_send_receive(rtree, msg_wrapper);

    if(resp == NULL || resp->recv_msg == NULL) {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg_wrapper);
        free(msg);
        return -1;
    }

    if(resp->recv_msg->c_type == MESSAGE_T__C_TYPE__CT_RESULT || resp->recv_msg->opcode == MESSAGE_T__OPCODE__OP_HEIGHT + 1) {
        int data_size = resp->recv_msg->datalength;
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg_wrapper);
        free(msg);
        return data_size;  
    } else {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg_wrapper);
        free(msg);
        return -1;
    }
}

/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */
char **rtree_get_keys(struct rtree_t *rtree) {

    if(rtree == NULL) {
        return NULL;
    }

    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct message_t *msg_wrapper = malloc(sizeof(struct message_t));
	msg_wrapper->recv_msg = msg;
    struct message_t *resp = network_send_receive(rtree, msg_wrapper);

    if(resp == NULL) {
        free(msg_wrapper);
        free(resp);
        free(msg);
        return NULL;
    }
    if(resp->recv_msg == NULL) {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(msg_wrapper);
        free(msg);
        return NULL;
    }

    if(resp->recv_msg->c_type == MESSAGE_T__C_TYPE__CT_KEYS || resp->recv_msg->opcode == MESSAGE_T__OPCODE__OP_GETKEYS + 1) {
        int nrKeys = resp->recv_msg->n_keys;
        char **keys = malloc((nrKeys + 1) * sizeof(char *));

        for (int i = 0; i < nrKeys; i++) {
            keys[i] = malloc(strlen(resp->recv_msg->keys[i]) + 1);
            strcpy(keys[i], resp->recv_msg->keys[i]);
        }

        keys[nrKeys] = NULL;
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg_wrapper);
        free(msg);

        return keys;
    } else {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg_wrapper);
        free(msg);
        return NULL;
    }
}

/* Devolve um array de void* com a cópia de todas os values da árvore,
 * colocando um último elemento a NULL.
 */
void **rtree_get_values(struct rtree_t *rtree) {

    if(rtree == NULL) {
        return NULL;
    }

    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct message_t *msg_wrapper = malloc(sizeof(struct message_t));
	msg_wrapper->recv_msg = msg;
    struct message_t *resp = network_send_receive(rtree, msg_wrapper);

    if(resp->recv_msg == NULL) {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(msg);
        free(msg_wrapper);
        return NULL;
    }

    if(resp->recv_msg->c_type == MESSAGE_T__C_TYPE__CT_VALUES || resp->recv_msg->opcode == MESSAGE_T__OPCODE__OP_GETVALUES + 1) {
        int nrValues = resp->recv_msg->n_values;
        void **values = (void**)malloc((nrValues + 1) * sizeof(void *));

        for (int i = 0; i < nrValues; i++) {
            values[i] = (void*)malloc(resp->recv_msg->values[i].len);
            memcpy(values[i],(void*)(resp->recv_msg->values[i].data),resp->recv_msg->values[i].len);
        }

        values[nrValues] = NULL;
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(msg);
        free(msg_wrapper);
        free(resp);

        return values;
    } else {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(msg);
        free(msg_wrapper);
        free(resp);
        return NULL;
    }
}

/* Verifica se a operação identificada por op_n foi executada na tree dada.
*/
int rtree_verify(struct rtree_t *rtree, int op_n){
    
    if(rtree == NULL) {
        return -1;
    }

    MessageT *msg = malloc(sizeof(MessageT));
    message_t__init(msg);
    msg->opcode = MESSAGE_T__OPCODE__OP_VERIFY;
    msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
    msg->datalength = op_n;
    
    struct message_t *msg_wrapper = malloc(sizeof(struct message_t));
    msg_wrapper->recv_msg = msg;
    struct message_t *resp = network_send_receive(rtree, msg_wrapper);

    if(resp == NULL || resp->recv_msg == NULL) {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg_wrapper);
        free(msg);
        return -1;
    }

    if(resp->recv_msg->c_type == MESSAGE_T__C_TYPE__CT_RESULT || resp->recv_msg->opcode == MESSAGE_T__OPCODE__OP_VERIFY + 1) {
        int data_size = resp->recv_msg->datalength;
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg_wrapper);
        free(msg);
        return data_size;  
    } else {
        message_t__free_unpacked(resp->recv_msg, NULL);
        free(resp);
        free(msg_wrapper);
        free(msg);
        return -1;
    }
}

