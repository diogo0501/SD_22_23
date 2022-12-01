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
#include <zookeeper/zookeeper.h>
#include <errno.h>
#include <unistd.h> 

/* Remote tree. A definir pelo grupo em client_stub-private.h
 */
struct rtree_t;

static int is_connected;
static zhandle_t *zh;
int ZDATALEN = 1024 * 1024;
static char *root_path = "/node";
typedef struct String_vector zoo_string;
static char *watcher_ctx = "ZooKeeper Data Watcher";

/**
* Watcher function for connection state change events
*/
void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			is_connected = 1; 
		} else {
			is_connected = 0; 
		}
	}
}

/**
* Data Watcher function for /chain node
*/
static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
	zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	int zoo_data_len = ZDATALEN;

	if (state == ZOO_CONNECTED_STATE)	 {
		if (type == ZOO_CHILD_EVENT) {
		/* Get the updated children and reset the watch */ 
			if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list)) {
				fprintf(stderr, "Error setting watch at %s!\n", root_path);
			}
			fprintf(stderr, "\n=== znode listing === [ %s ]", root_path); 
			for (int i = 0; i < children_list->count; i++)  {
				fprintf(stderr, "\n(%d): %s", i+1, children_list->data[i]);
			}
			fprintf(stderr, "\n=== done ===\n");
		} 
	}
}

/* Função para estabelecer uma associação entre o cliente e o servidor, 
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtree_t *rtree_connect(const char *address_port){

    zh = zookeeper_init(address_port, connection_watcher, 2000, 0, NULL, 0);

    if (zh == NULL) {
        fprintf(stderr, "Error connecting to ZooKeeper server!\n");
        return NULL;
    }

	zoo_string *children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	while (1) {
		if (is_connected) {
			if (ZNONODE == zoo_exists(zh, root_path, 0, NULL)) {
				if (ZOK == zoo_create( zh, root_path, NULL, -1, & ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0)) {
					fprintf(stderr, "%s created!\n", root_path);
				} else {
					fprintf(stderr,"Error Creating %s!\n", root_path);
					exit(EXIT_FAILURE);
				} 
			}	
			if (ZOK != zoo_wget_children(zh, root_path, &child_watcher, watcher_ctx, children_list)) {
				fprintf(stderr, "Error setting watch at %s!\n", root_path);
			}
			fprintf(stderr, "\n=== znode listing === [ %s ]", root_path); 
			for (int i = 0; i < children_list->count; i++)  {
				fprintf(stderr, "\n(%d): %s", i+1, children_list->data[i]);
			}
			fprintf(stderr, "\n=== done ===\n");
			pause(); 
		}
	}

    //ns onde ir buscar os ips dos filhos c id maior e menor de chain 
    struct rtree_t *rtree = malloc(sizeof(struct rtree_t));

    char *tok = strtok((char *)address_port, ":");
    if(tok == NULL) {
        return NULL;
    }

    rtree->ip = tok;

    tok = strtok(NULL, ":");

    if(tok == NULL) {

        return NULL;
    }
    rtree->port = atoi(tok);

    int conectionStatus = network_connect(rtree);

    if(conectionStatus == -1) {
        rtree_disconnect(rtree);
        return NULL;
    }

	free(children_list);
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

