/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
*/

//TODO TOMORROW
#include "network_client.h"
#include "client_stub-private.h"


/* Remote tree. A definir pelo grupo em client_stub-private.h
 */
struct rtree_t;

/* Função para estabelecer uma associação entre o cliente e o servidor, 
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtree_t *rtree_connect(const char *address_port){

    struct rtree_t *rtree = malloc(sizeof(struct rtree_t));

    char *tok = strtok(address_port, ":");
    if(tok == NULL) {
        return NULL;
    }

    rtree->ip = tok;

    tok = strtok(NULL, ":");
    short port = tok;
    if(port == NULL) {

        return NULL;
    }
    rtree->port = atoi(port);

    int conectionStatus = network_connect(rtree);

    if(conectionStatus == -1) {
        rtree_disconnect(rtree);
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

    if(rtree || entry == NULL) {
        return -1;
    }

    MessageT msg;
    message_t__init(&msg);
    msg.opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;


    MessageT *resp = network_send_receive(rtree, &msg);
    if(resp == NULL) {
        message_t__free_unpacked(resp, NULL);
        return -1;
    }

    message_t__free_unpacked(resp, NULL);

    return 0;
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtree_get(struct rtree_t *rtree, char *key) {

    if(rtree || key == NULL) {
        return -1;
    }


    MessageT msg;
    message_t__init(&msg);
    msg.opcode = MESSAGE_T__OPCODE__OP_GET;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;


    MessageT *resp = network_send_receive(rtree, &msg);
    if(resp == NULL) {
        message_t__free_unpacked(resp, NULL);
        return -1;
    }

    message_t__free_unpacked(resp, NULL);

    return 0;
}

/* Função para remover um elemento da árvore. Vai libertar 
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtree_del(struct rtree_t *rtree, char *key) {

    if(rtree || key == NULL) {
        return -1;
    }

    MessageT msg;
    message_t__init(&msg);
    msg.opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;

    MessageT *resp = network_send_receive(rtree, &msg);
    if(resp == NULL) {
        message_t__free_unpacked(resp, NULL);
        return -1;
    }

    message_t__free_unpacked(resp, NULL);

    return 0;
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t *rtree) {

    if(rtree == NULL) {
        return -1;
    }

    MessageT msg;
    message_t__init(&msg);
    msg.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg.datalength = 0;
    
    MessageT *resp = network_send_receive(rtree, &msg);

    if(resp == NULL) {
        message_t__free_unpacked(resp, NULL);
        return -1;
    }

    int data_size = resp->datalength;
    message_t__free_unpacked(resp, NULL);
    return data_size;
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t *rtree) {

    if(rtree == NULL) {
        return -1;
    }

    MessageT msg;
    message_t__init(&msg);
    msg.opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;
    msg.datalength = 0;
    
    MessageT *resp = network_send_receive(rtree, &msg);

    if(resp == NULL) {
        message_t__free_unpacked(resp, NULL);
        return -1;
    }

    int data_size = resp->datalength;
    message_t__free_unpacked(resp, NULL);
    return data_size;
}

/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */
char **rtree_get_keys(struct rtree_t *rtree) {

    if(rtree == NULL) {
        return -1;
    }

    MessageT msg;
    message_t__init(&msg);
    msg.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;


    MessageT *resp = network_send_receive(rtree, &msg);
    if(resp == NULL) {
        message_t__free_unpacked(resp, NULL);
        return -1;
    }

    message_t__free_unpacked(resp, NULL);

    return keys;
}

/* Devolve um array de void* com a cópia de todas os values da árvore,
 * colocando um último elemento a NULL.
 */
void **rtree_get_values(struct rtree_t *rtree) {

    if(rtree == NULL) {
        return -1;
    }

    MessageT msg;
    message_t__init(&msg);
    msg.opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;


    MessageT *resp = network_send_receive(rtree, &msg);
    if(resp == NULL) {
        message_t__free_unpacked(resp, NULL);
        return -1;
    }

    message_t__free_unpacked(resp, NULL);

    return values;
}


