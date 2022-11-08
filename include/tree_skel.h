#ifndef _TREE_SKEL_H
#define _TREE_SKEL_H

#include "sdmessage.pb-c.h"
#include "tree.h"

struct request_t {
    int op_n; //numero da op
    int op; // 0 se for um del, 1 se for put
    char* key;
    char* data; //NULL || data
    struct request_t *ant;
    struct request_t *succ;
};

struct op_proc {
    int max_proc;
    int *in_progress;
};
/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). 
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */

//agr tem um param int N 
int tree_skel_init(int N);

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy();

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
*/
int invoke(struct message_t *msg);

/* Verifica se a operação identificada por op_n foi executada.
*/
int verify(int op_n);

/* Função da thread secundária que vai processar pedidos de escrita.
*/
void * process_request(void *params);

#endif
