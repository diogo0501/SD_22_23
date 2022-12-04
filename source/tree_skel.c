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
#include "client_stub.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <zookeeper/zookeeper.h>
#include <arpa/inet.h>

int ZDATALEN = 1024 * 1024;

struct rtree_t *nex_serv_conn;
static char *watcher_ctx = "ZooKeeper Data Watcher";
typedef struct String_vector zoo_string; 
zoo_string* children_list; 
char** children_socks;
int new_path_len = 1024;
char* new_path;
static zhandle_t *zh;
static char *chain_path = "/chain"; 
static char *new_chain_path = "/chain"; 
struct tree_t *server_side_tree;
struct request_t *queue_head = NULL;
struct op_proc *ops_info;
int last_assigned;
int is_connected;
int counter = 1;
int znode_id;
int next_serv_sock;
int next_serv_port;
int next_server_id = INT16_MAX; 
struct sockaddr_in next_server_addr;


pthread_mutex_t process_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ops_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t process_cond = PTHREAD_COND_INITIALIZER;

static void set_next_server() {
	
	next_server_id = INT16_MAX;
	int highest_znode_id = INT16_MIN;
	int id = 0;
	char* tmp2 = "node";
	char* next_serv_path = malloc(sizeof(char*));
	char* zdata_buf = malloc(ZDATALEN * sizeof(char*));

	if((next_serv_sock = socket(AF_INET,SOCK_STREAM,0)) < 0 ) {
		perror("Error on creating socket\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < children_list->count; i++)  {

		char* tmp = malloc(sizeof(char*));

		//Could be cut. Strcmp can do this kind of compare
		
		strncpy(tmp, children_list->data[i] + strlen(tmp2), 
				strlen(children_list->data[i]) + 1 - strlen("node"));

		id = atoi(tmp);

		printf("Id : %d\n",id);

		printf("Port : %s\n",children_socks[i]);

		if(id > znode_id && id < next_server_id) {
			next_server_id = id;
			next_serv_path = children_list->data[i];
			next_serv_port = atoi(children_socks[i]);
		}

	}

	if(next_server_id == INT16_MAX) {
		next_serv_path = new_path;
		next_server_id = znode_id;
		next_serv_port = -1;
		nex_serv_conn = NULL;
		printf("Socket n : %d\n",next_serv_port);

	}
	else {
		char *tmp3 = malloc(sizeof(char*) * 120);
		strcat(tmp3,"/chain/");
		strcat(tmp3,next_serv_path);

		printf("Next Server Path : %s\n",tmp3);
		if (ZNONODE == zoo_exists(zh, tmp3, 0, NULL)) {
				fprintf(stderr, "%s doesn't exist!\n", next_serv_path);
				exit(EXIT_FAILURE);
		}

		printf("Socket n : %d\n",next_serv_port);

		char *next_ip;
		next_ip = malloc(sizeof(char) * 120);
		snprintf(next_ip,120,"127.0.0.1:%d",next_serv_port); //IP hardcoded. TODO : Chanhe this

		printf("%s\n",next_ip);

		// int port = 0;
		nex_serv_conn = rtree_connect(next_ip);

		if(nex_serv_conn == NULL) {
			fprintf(stderr, "Error on connecting to next server\n");
			exit(EXIT_FAILURE);
		}
		printf("Next server connected\n");
	}

}

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {

	int zoo_data_len = ZDATALEN;

	if (state == ZOO_CONNECTED_STATE)	 {
		if (type == ZOO_CHILD_EVENT) {
		/* Get the updated children and reset the watch */ 
			if (ZOK != zoo_wget_children(zh, chain_path, child_watcher, watcher_ctx, children_list)) {
				fprintf(stderr, "Error setting watch at %s!\n", chain_path);
			}
			fprintf(stderr, "\n=== znode listing === [ %s ]", chain_path); 
			for (int i = 0; i < children_list->count; i++)  {
				//fprintf(stderr, "\n(%d): %s", i+1, children_list->data[i]);

				char tmp[120] = "";
				strcat(tmp,chain_path);
				strcat(tmp,"/");
				strcat(tmp,children_list->data[i]);
				printf("\n %s \n",tmp);

				char *zdatabuf = calloc(1, ZDATALEN);
				if(ZOK != zoo_get(zh,tmp ,0, zdatabuf,&ZDATALEN,NULL)) {

					fprintf(stderr, "Error getting data from %s!\n",children_list->data[i]);
				}
				printf("Buffer : %s\n",zdatabuf);
				memcpy(children_socks[i],zdatabuf,sizeof(zdatabuf));
				free(zdatabuf);

			}
			fprintf(stderr, "\n=== done ===\n");

			printf("Children count : %d\n", children_list->count);

			set_next_server();
			
		} 
	}

}


/* Inicio da guarda para garantir acesso exclusivo a zona critica
 */
void guard_start() {

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

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) 
		is_connected = (state == ZOO_CONNECTED_STATE) ? 1 : 0;
}

/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). 
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int tree_skel_init(char* zoo_ip, char* port) {

	server_side_tree = tree_create();

	zh = zookeeper_init(zoo_ip, connection_watcher, 2000, 0, 0, 0);

	if (zh == NULL) {
		fprintf(stderr,"Error connecting to ZooKeeper server[%d]!\n", errno);
		exit(EXIT_FAILURE);
	}

	//printf("Hostname : %s\n",zh->hostname);

	sleep(3);

	if (is_connected) {
		if (ZNONODE == zoo_exists(zh, chain_path, 0, NULL)) {

			int new_chain_len = 1024;

			new_chain_path = malloc(new_chain_len);



			if (ZOK != zoo_create(zh, chain_path, NULL, -1, & ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0)) {
				fprintf(stderr, "Error creating znode from path %s!\n", chain_path);
			    exit(EXIT_FAILURE);
			}
		}

		char node_path[120] = "";
		strcat(node_path,new_chain_path); 
		strcat(node_path,"/node"); 
		new_path = malloc (new_path_len);
		
		
		if (ZOK != zoo_create(zh, node_path, port, strlen(port), &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL 
		| ZOO_SEQUENCE, new_path, new_path_len)) {
			fprintf(stderr, "Error creating znode from path %s!\n", new_path);
			exit(EXIT_FAILURE);
		}

		char* tmp = malloc(sizeof(char*)*10 + 1);
		strncpy(tmp, new_path + strlen(node_path), 10);

		znode_id = atoi(tmp);

		children_list =	(zoo_string *) malloc(sizeof(zoo_string));
		children_socks = malloc(ZDATALEN * 10); //10 servers hardcoded
		for(int i = 0; i < 10; i++) {
			children_socks[i] = malloc(ZDATALEN);
		}
		//g_children_list = (zoo_string *) malloc(sizeof(zoo_string));

		if (ZOK != zoo_wget_children(zh, chain_path, &child_watcher, watcher_ctx, children_list)) {
			fprintf(stderr, "Error setting watch at %s!\n", chain_path);
		}

		//set_next_server();

		sleep(5); //Is this rlly nec?

		free(tmp);
		free(new_path);
	}

	ops_info = malloc(sizeof(struct op_proc));

	pthread_mutex_lock(&ops_lock);

	ops_info->max_proc = 0;
	ops_info->in_progress = malloc(sizeof(int));
	ops_info->in_progress[0] = 0;

	pthread_mutex_unlock(&ops_lock);

	last_assigned = 1;

	locks_init();

	pthread_t thread;
	int thread_param = 1;

	if(pthread_create(&thread,NULL, &process_request,(void*) &thread_param) != 0){
		printf("Thread was not created sucessfully.\n");
		exit(EXIT_FAILURE);
	}
	if(pthread_detach(thread) != 0) {
		printf("Thread couldn't be detached properly.\n");
		exit(EXIT_FAILURE);
	}

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

	//Idk if its supposed to be here
	//set_next_server();

	pthread_mutex_lock(&ops_lock);

	if(verify(ops_info->in_progress[0]) == 1) {
		ops_info->in_progress[0] = 0;
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


		if(ops_info->in_progress[0] == 0) 
			ops_info->in_progress[0] = last_assigned;
		

		pthread_mutex_unlock(&ops_lock);

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

		if(ops_info->in_progress[0] == 0) {
			ops_info->in_progress[0] = last_assigned;
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

		guard_start();

		int nex_status = 0;

		if(req->op == 0) {
			int status = tree_del(server_side_tree, req->key);
			rtree_del(nex_serv_conn,req->key);

		} else {

			int status = tree_put(server_side_tree, req->key, req->data);
			struct entry_t *ent = malloc(sizeof(struct entry_t));
			ent->key = req->key;
			ent->value = req->data;
			nex_status = rtree_put(nex_serv_conn,ent);
			
		}

		if(nex_status == -1) {
			fprintf(stderr,"Error on sending request to next server. If this server is the last in the chain, please ignore this message\n");
		}

		pthread_mutex_lock(&ops_lock);
		if(ops_info->max_proc < req->op_n)
				ops_info->max_proc = req->op_n;
		pthread_mutex_unlock(&ops_lock);
		
		guard_end();

	}
	return NULL;
}

