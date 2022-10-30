/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
 */

#include "entry.h"
#include "sdmessage.pb-c.h"
#include "message-private.h"
#include "tree_skel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tree_t *server_side_tree;

/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). 
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int tree_skel_init() {

	server_side_tree = tree_create();

	return server_side_tree == NULL ? -1 : 0;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy() {
	tree_destroy(server_side_tree);
}

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
*/
int invoke(struct message_t *msg) {

	MessageT *message = msg->recv_msg;

	if(server_side_tree == NULL || msg == NULL) {
		return -1;
	}

	if(message->opcode == MESSAGE_T__OPCODE__OP_PUT && message->c_type == MESSAGE_T__C_TYPE__CT_ENTRY) {
		
		if(message->entry == NULL) {
			return -1;
		}

		message->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
		message->c_type = MESSAGE_T__C_TYPE__CT_NONE;
		message->datalength = 0;

		struct data_t *data = data_create2(message->entry->data.len, (void*)message->entry->data.data);

		struct entry_t *entry = entry_create(message->entry->key, data);

		free(entry);

		int status = tree_put(server_side_tree, message->entry->key, data);
		
		if(status == -1) {
			return -1;
		} else {
			free(data);
			return 0;
		}
	}

	if(message->opcode == MESSAGE_T__OPCODE__OP_GET && message->c_type == MESSAGE_T__C_TYPE__CT_KEY) {

		message->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
		message->c_type = MESSAGE_T__C_TYPE__CT_VALUE;

		struct data_t *data = tree_get(server_side_tree, (char *)message->data.data);
		//Nao encontra a data com given key
		if (data == NULL) {

			free(message->data.data);

			message->data.data = NULL;
			message->data.len = 0;

			return 0;
		}

		free(message->data.data);

		message->data.data = (uint8_t *)data->data;
		message->data.len = data->datasize;

		free(data);

		return 0;
	}

	if(message->opcode == MESSAGE_T__OPCODE__OP_DEL && message->c_type == MESSAGE_T__C_TYPE__CT_KEY) {

		int status = tree_del(server_side_tree, (char*) message->data.data);

		if(status == -1) {
			return -1;
		} else{
			message->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
			message->c_type = MESSAGE_T__C_TYPE__CT_NONE;
			message->datalength = 0;

			return 0;
		}
	}

	if(message->opcode == MESSAGE_T__OPCODE__OP_SIZE && message->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
		
		message->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
		message->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
		int s = tree_size(server_side_tree);
		message->datalength = s;
		return 0;
	}

	if(message->opcode == MESSAGE_T__OPCODE__OP_HEIGHT && message->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
		
		message->opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
		message->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
		int h = tree_height(server_side_tree);
		message->datalength = h;
		return 0;
	}

	if (message->opcode == MESSAGE_T__OPCODE__OP_GETKEYS && message->c_type == MESSAGE_T__C_TYPE__CT_NONE) {

		int s = tree_size(server_side_tree);

		message->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
		message->c_type = MESSAGE_T__C_TYPE__CT_KEYS;

		if(s == 0) {
			message->keys = NULL;
			message->n_keys = s;
			return 0;
		}
		char **keys = tree_get_keys(server_side_tree);

		if (keys == NULL) {
			return -1;

		}

		message->keys = keys;
		message->n_keys = s;

		return 0;
	}

	if (message->opcode == MESSAGE_T__OPCODE__OP_GETVALUES && message->c_type == MESSAGE_T__C_TYPE__CT_NONE) {

		int s = tree_size(server_side_tree);

		message->opcode = MESSAGE_T__OPCODE__OP_GETVALUES + 1;
		message->c_type = MESSAGE_T__C_TYPE__CT_VALUES;

		void **values = tree_get_values(server_side_tree);

		if(s == 0) {
			message->values = NULL;
			message->n_values = s;
			return 0;
		}

		if (values == NULL) {
			return -1;
		}

		message->values = malloc(sizeof(ProtobufCBinaryData) * s);
		for(int i = 0; i < s; i++) {
			char* tmp = (char*)(((struct data_t*)values[i])->data);
			message->values[i].data = malloc(sizeof(uint8_t*));
			memcpy(message->values[i].data,(uint8_t *)((void*)tmp),sizeof(uint8_t*));
			message->values[i].len = ((struct data_t*)values[i])->datasize;
		}
		message->n_values = s;

		for(int i = 0; i < s+1; i++) {
			free(values[i]);
		}
		
		free(values);

		return 0;
	}

	message->datalength = 0;
	message->opcode = MESSAGE_T__OPCODE__OP_BAD;
	message->c_type = MESSAGE_T__C_TYPE__CT_BAD;

	return 0;
}
