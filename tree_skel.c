#include "sdmessage.pb-c.h"
#include "tree_skel.h"
#include "tree-private.h"
#include "message-private.h"

struct tree_t *tree;

int tree_skel_init() {
	tree = tree_create();
	if (tree==NULL){
		return -1;
	}
	return 0;
}

void tree_skel_destroy(){
	tree_destroy(tree);
}

int invoke (struct message_t *msg){
	if(msg==NULL){		//verifica se NULL
		return -1;
	}

	//verificar integridade da mensagem
	if ((msg->recv_msg->opcode<10 || msg->recv_msg->opcode>MESSAGE_T__OPCODE__OP_GETVALUES)
	|| (msg->recv_msg->c_type<0 || msg->recv_msg->c_type>MESSAGE_T__C_TYPE__CT_NONE) ){
		return -1;
	}


	switch(msg->recv_msg->opcode){
		case(MESSAGE_T__OPCODE__OP_BAD):
			return -1;
		case(MESSAGE_T__OPCODE__OP_SIZE):
			int size = tree_size(tree);
			if(size<0){
				msg->recv_msg->opcode=MESSAGE_T__OPCODE__OP_ERROR;
				msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_NONE;
			}
			else{
				msg->recv_msg->opcode++;
				msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_RESULT;
				msg->recv_msg->data_size=size;
			}
		case(MESSAGE_T__OPCODE__OP_HEIGHT):
			int height = tree_height(tree);
			if (height<0){
				msg->recv_msg->opcode=MESSAGE_T__OPCODE__OP_ERROR;
				msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_NONE;
			}
			else{
				msg->recv_msg->opcode++;
				msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_RESULT;
				msg->recv_msg->data_size=height;
			}
		case(MESSAGE_T__OPCODE__OP_DEL):
			int result = tree_del(tree,(char*) msg->recv_msg->data.data);
			if(result==-1){
				msg->recv_msg->opcode=MESSAGE_T__OPCODE__OP_ERROR;
				msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_NONE;
				return -1;
			}
			else{
				msg->recv_msg->opcode++;
				msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_NONE;
			}
		case(MESSAGE_T__OPCODE__OP_GET):
			struct data_t * result;
			if(result=tree_get(tree,(char*) msg->recv_msg->data.data==NULL)){
				msg->recv_msg->opcode=MESSAGE_T__OPCODE__OP_ERROR;
				msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_NONE;
			}
			else{
				msg->recv_msg->opcode++;
				msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_VALUE;
				msg->recv_msg->data.data=result;
				msg->recv_msg->data.len=sizeof(result);
			}
		case(MESSAGE_T__OPCODE__OP_PUT):
			struct data_t *to_put;
			to_put=data_create2(msg->recv_msg->entry->value.len,msg->recv_msg->entry->value.data);
			if(to_put==NULL){
			msg->recv_msg->opcode=MESSAGE_T__OPCODE__OP_ERROR;
			msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_NONE;
				return -1;
			}
			else{
				int result = tree_put(tree,msg->recv_msg->entry->key,to_put);
				if(result==-1){
					msg->recv_msg->opcode=MESSAGE_T__OPCODE__OP_ERROR;
					msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_NONE;
					return -1;
				}
				else{
					msg->recv_msg->opcode++;
					msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_NONE;
				}
			}
		case(MESSAGE_T__OPCODE__OP_GETKEYS):
			char **keys=tree_get_keys(tree);
			if (keys==NULL){
				msg->recv_msg->opcode=MESSAGE_T__OPCODE__OP_ERROR;
				msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_NONE;
				return -1;
			}
			else{
				msg->recv_msg->opcode++;
				msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_KEYS;
				msg->recv_msg->keys->keys;
			}
		case(MESSAGE_T__OPCODE__OP_GETVALUES):
			void **values = tree_get_values(tree);
			if(values==NULL){
				msg->recv_msg->opcode=MESSAGE_T__OPCODE__OP_ERROR;
				msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_NONE;
				return -1;
			}
			else{
				msg->recv_msg->opcode++;
				msg->recv_msg->c_type=MESSAGE_T__C_TYPE__CT_VALUES;
				msg->recv_msg->values->values=values;
			}
	
	return 0;

			
	}
}