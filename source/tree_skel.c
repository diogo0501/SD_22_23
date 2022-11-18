/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
*/

#include "entry.h"
#include "sdmessage.pb-c.h"
#include "message-private.h"
#include "server_structs-private.h"
#include "tree_skel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct tree_t *server_side_tree;
struct request_t *queue_head = NULL;
struct op_proc *ops_info;
int n_threads;
int last_assigned;
int counter = 1;											//nr de threads que acede a zona critica
pthread_mutex_t process_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ops_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t process_cond = PTHREAD_COND_INITIALIZER;

/* Inicio da guarda para garantir acesso exclusivo a zona critica
 */
void guard_start(int thread_number) {

	pthread_mutex_lock(&process_lock);
	
	while(counter == 0) {
		pthread_cond_wait(&process_cond,&process_lock);
	}

	counter--;
	pthread_mutex_unlock(&process_lock);
}

/* Fim da guarda para garantir acesso exclusivo a zona critica
 */
void guard_end() {

	pthread_mutex_lock(&process_lock);

	counter++;
	
	pthread_cond_broadcast(&process_cond);
	pthread_mutex_unlock(&process_lock);
}

/* Cria um request e processa o dependendo do opcode
 */
struct request_t *create_request(int op, MessageT *message) {

	struct request_t *new_req = malloc(sizeof(struct request_t));
	new_req->op_n = last_assigned;
	new_req->op = op;

	if(op == 1) {
		struct data_t *data = data_create2(message->entry->data.len, message->entry->data.data);
		new_req->key = message->entry->key;
		new_req->data = data;
	}
	else {
		new_req->key = (char*)(message->data.data);
		new_req->data = NULL;
	}

	return new_req;
}

/* Inicia os locks e as condicoes de lock
 */
void locks_init(){

	int queue_lock_init = pthread_mutex_init(&queue_lock,NULL);
	int proc_lock_init = pthread_mutex_init(&process_lock,NULL);
	int ops_lock_init = pthread_mutex_init(&ops_lock,NULL);

	int lock_status = queue_lock_init + proc_lock_init	
						+ ops_lock_init;

	if( lock_status != 0) {
		perror("Some mutex could not be initialized\n");
		exit(EXIT_FAILURE);
	}

	int queue_cond_init = pthread_cond_init(&queue_not_empty,NULL);
	int proc_cond_init = pthread_cond_init(&process_cond,NULL);

	int cond_status = queue_cond_init + proc_cond_init;

	if(cond_status != 0) {
		perror("Some cond could not be initiaized\n");
		exit(EXIT_FAILURE);
	}
}

/* Adiciona um novo pedido a lista de pedidos do servidor
 */
void queue_add_request(struct request_t *new_req) {

	pthread_mutex_lock(&queue_lock);

	if(queue_head == NULL) { /* Adiciona na cabeça da fila */
		queue_head = new_req; 
		new_req->succ = NULL;
	} 
	else { /* Adiciona no fim da fila */
		struct request_t *tpreq = queue_head;
		while(tpreq->succ != NULL) {
			tpreq = tpreq->succ;
		}
		tpreq->succ = new_req;
		new_req->succ = NULL;
	}

	pthread_cond_signal(&queue_not_empty); /* Avisa um bloqueado nessa condição */
	pthread_mutex_unlock(&queue_lock);
}

/* Devolve um pedido da lista
 */
struct request_t *queue_get_request() {

	pthread_mutex_lock(&queue_lock);

	while(queue_head == NULL)
		pthread_cond_wait(&queue_not_empty, &queue_lock); /* Espera haver algo */
	
	struct request_t *req = queue_head; 
	queue_head = req->succ;

	pthread_mutex_unlock(&queue_lock);

	return req;
}

/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). 
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int tree_skel_init(int N) {

	n_threads = N;

	ops_info = malloc(sizeof(struct op_proc));

	pthread_mutex_lock(&ops_lock);

	ops_info->max_proc = 0;
	ops_info->in_progress = malloc(sizeof(int) * n_threads);

	for(int i = 0; i < n_threads; i++) {
		ops_info->in_progress[i] = 0;
	}

	pthread_mutex_unlock(&ops_lock);

	last_assigned = 1;

	locks_init();

	pthread_t thread[n_threads];
	int thread_param[n_threads];

	for(int i = 0; i < n_threads; i++) {
		thread_param[i] = i+1;
		if(pthread_create(&thread[i],NULL, &process_request,(void*) &thread_param[i]) != 0){
			printf("Thread %d was not created sucessfully.\n",i);
			exit(EXIT_FAILURE);
		}
		if(pthread_detach(thread[i]) != 0) {
			printf("Thread %d couldn't be detached properly.\n",i);
			exit(EXIT_FAILURE);
		}
	}

	server_side_tree = tree_create();

	return server_side_tree == NULL ? -1 : 0;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy() {

	free(ops_info->in_progress);
	free(ops_info);
	 
	pthread_mutex_destroy(&process_lock);
	pthread_mutex_destroy(&ops_lock);
	pthread_mutex_destroy(&queue_lock);
		
	tree_destroy(server_side_tree);
}

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
*/
int invoke(struct message_t *msg) {

	pthread_mutex_lock(&ops_lock);

	for(int i = 0; i < n_threads; i++) {
        if(verify(ops_info->in_progress[i]) == 1) {
            ops_info->in_progress[i] = 0;
        }
    }

	pthread_mutex_unlock(&ops_lock);

	MessageT *message = msg->recv_msg;

	if(server_side_tree == NULL || msg == NULL) {
		return -1;
	}

	if(message->opcode == MESSAGE_T__OPCODE__OP_PUT && message->c_type == MESSAGE_T__C_TYPE__CT_ENTRY) {
		
		if(message->entry == NULL) {
			return -1;
		}

		struct request_t* new_req = create_request(1,message);

		queue_add_request(new_req);	

		pthread_mutex_lock(&ops_lock);	

		for(int i = 0; i < n_threads; i++) {
			if(ops_info->in_progress[i] == 0) {
				ops_info->in_progress[i] = last_assigned;
				break;
			}
		}

		pthread_mutex_unlock(&ops_lock);
		
		for(int i = 0; i < n_threads; i++) {
				printf("%d \n", ops_info->in_progress[i]);
		}

		message->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
		message->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
		message->datalength = last_assigned;
		last_assigned++;

		return 0;

	}

	if(message->opcode == MESSAGE_T__OPCODE__OP_GET && message->c_type == MESSAGE_T__C_TYPE__CT_KEY) {

		message->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
		message->c_type = MESSAGE_T__C_TYPE__CT_VALUE;

		struct data_t *data = tree_get(server_side_tree, (char *)message->data.data);
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

		struct request_t *new_req = create_request(0,message);

		queue_add_request(new_req);

		pthread_mutex_lock(&ops_lock);

		for(int i = 0; i < n_threads; i++) {
			if(ops_info->in_progress[i] == 0) {
				ops_info->in_progress[i] = last_assigned;
				break;
			}
		}

		pthread_mutex_unlock(&ops_lock);

		message->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
		message->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
		message->datalength = last_assigned;
		last_assigned++;

		return 0;
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

	if(message->opcode == MESSAGE_T__OPCODE__OP_VERIFY && message->c_type == MESSAGE_T__C_TYPE__CT_RESULT) {

		message->opcode = MESSAGE_T__OPCODE__OP_VERIFY + 1;
		message->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
		int v = verify((int)message->datalength);
		message->datalength = v;
		return 0;
	}

	message->datalength = 0;
	message->opcode = MESSAGE_T__OPCODE__OP_BAD;
	message->c_type = MESSAGE_T__C_TYPE__CT_BAD;

	return 0;
}

/* Verifica se a operação identificada por op_n foi executada.
*/
int verify(int op_n) {
	if(op_n < 0)
		return 2;
	return (ops_info->max_proc >= op_n) ? 1 : 0;
}

/* Função da thread secundária que vai processar pedidos de escrita.
*/
void *process_request(void *params) {

	if(params == NULL) {
		return NULL;
	}

	int *myid = (int *)params;

	while(1) {

		struct request_t *req = queue_get_request();

		guard_start(*myid);

		if(req->op == 0) {
			int status = tree_del(server_side_tree, req->key);
		} else {
			int status = tree_put(server_side_tree, req->key, req->data);
		}

		pthread_mutex_lock(&ops_lock);
		if(ops_info->max_proc < req->op_n)
				ops_info->max_proc = req->op_n;
		pthread_mutex_unlock(&ops_lock);
		//falta dar free na request
		guard_end();
	}
	return NULL;
}

