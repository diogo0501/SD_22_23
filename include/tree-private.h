#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "tree.h"
/* Struct node_t representa um nodulo cujo conteudo eh uma entry e referencias para os seus filhos
 */
struct node_t {
	struct entry_t *entry;
	struct node_t *left, *right;
};

/* Struct tree_t representa uma arvore composta por elementos da estrutura node_t
 */
struct tree_t {
	struct node_t *root;
	int nrElements;
	int height;
};

#endif
