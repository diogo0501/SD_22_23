#ifndef _NODE_PRIVATE_H
#define _NODE_PRIVATE_H

#include "tree-private.h"

/* Função para libertar toda a memória ocupada por um node.
 */
void node_destroy(struct node_t *treeRoot);

/* Função para adicionar um node contendo o par chave-valor dado à árvore.
 * Retorna 0 (ok), -1 em caso de erro ou 1 em caso de nao ter havido a necessidade de adicionar
 * um novo nodulo a arvore
 */
int node_put(struct node_t *parent_node , struct node_t *node, struct entry_t *entry);

/* Função para obter da árvore o valor associado à chave key.
 * Devolve NULL em caso de erro.
 */
struct entry_t *node_get(struct node_t *treeRoot, char *key);

/* Função que dado um node calcula a altura da árvore.
 */
int node_calculateTreeHeight(struct node_t *treeRoot);

/* Função que retorna o node contendo a menor key da árvore.
 */
struct tuple_t *node_findLeftmostLeaf(struct node_t *parent_node , struct node_t *node);

/* Função para remover um elemento da árvore, indicado pela chave key,
 * Retorna um node (ok) ou NULL (error).
 */
int node_del(struct tree_t *tree , struct node_t *parent_node , struct node_t *treeRoot, char *key);


/* Função que devolve um array de char* com a cópia de todas as keys da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária. As keys devem vir ordenadas segundo a ordenação lexicográfica das mesmas.
 */
char **node_getKeys(struct tree_t *tree,struct node_t *treeRoot, char **keys, int i);

/* Função que devolve um array de void* com a cópia de todas os values da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
void **node_getValues(struct tree_t *tree,struct node_t *treeRoot, void **values, int i);

/* Função auxiliar max.
 */
int max(int a, int b);

/* Struct tuple_t representa um tuplo de node_t
 */
struct tuple_t {
	struct node_t *parent_node;
	struct node_t *curr_node;
};

#endif
