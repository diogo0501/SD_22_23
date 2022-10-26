/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
 */

#include "sdmessage.pb-c.h"
#include "message-private.h"
#include "tree_skel.h"
#include "entry.h"
#include <stdlib.h>
#include <stdio.h>

struct tree_t *tree;

int tree_skel_init() {

	tree = tree_create();

	return tree == NULL ? -1 : 0;
}

void tree_skel_destroy() {
	tree_destroy(tree);
};

int invoke(struct message_t *msg) {

	MessageT *netmsg = msg->recv_msg;

	if (tree == NULL || msg == NULL || netmsg == NULL) {
		return -1;
	}

	if (netmsg->opcode == MESSAGE_T__OPCODE__OP_SIZE && netmsg->c_type == MESSAGE_T__C_TYPE__CT_NONE) {

		int size = tree_size(tree);

		netmsg->datalength = size;
		netmsg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
		netmsg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;

		return 0;

	}

	if (netmsg->opcode == MESSAGE_T__OPCODE__OP_HEIGHT && netmsg->c_type == MESSAGE_T__C_TYPE__CT_NONE) {

		int height = tree_height(tree);

		netmsg->datalength = height;
		netmsg->opcode = MESSAGE_T__OPCODE__OP_HEIGHT + 1;
		netmsg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;

		return 0;

	}

	if (netmsg->opcode == MESSAGE_T__OPCODE__OP_DEL && netmsg->c_type == MESSAGE_T__C_TYPE__CT_KEY) {

		int size = tree_size(tree);

		int del = tree_del(tree, (char*) netmsg->data.data);

		if(del == -1) {
			return -1;
		}
		netmsg->datalength = 0;
		netmsg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
		netmsg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

		return 0;
	}


	if (netmsg->opcode == MESSAGE_T__OPCODE__OP_GET && netmsg->c_type == MESSAGE_T__C_TYPE__CT_KEY) {

		netmsg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
		netmsg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;

		struct data_t *data = tree_get(tree, (char *)netmsg->data.data);

		if (data == NULL) {

			free(netmsg->data.data);

			netmsg->data.data = NULL;
			netmsg->data.len = 0;

			return 0;
		}

		free(netmsg->data.data);

		netmsg->data.data = (uint8_t *)data->data;
		netmsg->data.len = data->datasize;

		free(data);

		return 0;
	}

	if (netmsg->opcode == MESSAGE_T__OPCODE__OP_PUT && netmsg->c_type == MESSAGE_T__C_TYPE__CT_ENTRY)
	{
		if (netmsg->entry == NULL || netmsg->entry == NULL) {
			return -1;
		}

		struct data_t *temp = data_create2(netmsg->entry->data.len, (void*)netmsg->entry->data.data);

		struct data_t *temp1 = data_create2(netmsg->entry->data.len, (void*)netmsg->entry->data.data);

		struct entry_t *entry = entry_create(netmsg->entry->key, temp1);


		free(temp1);
		free(entry);

		int put = tree_put(tree, netmsg->entry->key, temp);
		if (put == -1)
		{
			return -1;
		}

		free(temp);
		netmsg->datalength = 0;

		netmsg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
		netmsg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
		return 0;

	}


	if (netmsg->opcode == MESSAGE_T__OPCODE__OP_GETKEYS && netmsg->c_type == MESSAGE_T__C_TYPE__CT_NONE) {

		int treesize = tree_size(tree);

		netmsg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
		netmsg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;

		if(treesize == 0) {
			netmsg->keys = NULL;
			netmsg->n_keys = treesize;
			return 0;
		}
		char **keys = tree_get_keys(tree);

		if (keys == NULL) {
			return -1;

		}

		netmsg->keys = keys;
		netmsg->n_keys = treesize;

		return 0;
	}

	if (netmsg->opcode == MESSAGE_T__OPCODE__OP_GETVALUES && netmsg->c_type == MESSAGE_T__C_TYPE__CT_NONE) {

		int treesize = tree_size(tree);

		netmsg->opcode = MESSAGE_T__OPCODE__OP_GETVALUES + 1;
		netmsg->c_type = MESSAGE_T__C_TYPE__CT_VALUES;

		void **values = tree_get_values(tree);

		if(treesize == 0) {
			netmsg->values = NULL;
			netmsg->n_values = treesize;
			return 0;
		}

		if (values == NULL) {
			return -1;

		}

		netmsg->values = malloc(sizeof(ProtobufCBinaryData) * treesize);
		for(int i = 0; i < treesize; i++) {
			char* tmp = (char*)(((struct data_t*)values[i])->data);
			netmsg->values[i].data = (uint8_t *)((void*)tmp);
			netmsg->values[i].len = sizeof(void*);
		}
		netmsg->n_values = treesize;

		return 0;
	}

	netmsg->datalength = 0;
	netmsg->opcode = MESSAGE_T__OPCODE__OP_BAD;
	netmsg->c_type = MESSAGE_T__C_TYPE__CT_BAD;

	return 0;
}
