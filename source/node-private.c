/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
 */

#include "../include/entry.h"
#include "../include/node-private.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
void node_destroy(struct node_t *treeRoot) {

	struct node_t *node = treeRoot;

	if(node == NULL) {
		free(node);
		return;
	}

	if(node->entry == NULL) {
		entry_destroy(node->entry);
		free(node);
		return;
	}

	if(node->left == NULL && node->right == NULL) {
		free(node);
		return;
	}

	node_destroy(node->left);
	node_destroy(node->right);

}


int node_put(struct node_t *parent_node, struct node_t *node, struct entry_t *entry) {

	if(entry == NULL) {
		return -1;
	}

	if(parent_node == NULL && node == NULL) {
		node->entry = entry;
		node->left = NULL;
		node->right = NULL;
		return 0;
	}

	//Problema aqui. Nao consigo aceder a um NULL pointer e se tentar copiar a struct a referencia
	//com o no pai perde-se
	if(node == NULL && parent_node != NULL) {
		struct node_t *tmpRoot = malloc(sizeof(struct node_t));
		tmpRoot->entry = entry;
		tmpRoot->left = NULL;
		tmpRoot->right = NULL;
		node = tmpRoot;

		//Spaghetti code !!!!!!!!!
		if(strcmp(entry->key,parent_node->entry->key) < 0) {
			parent_node->left = node;
		}
		else {
			parent_node->right = node;
		}
		return 0;
	}

	if(node->entry == NULL) {
		node->entry = entry;
		return 0;
	}

	if(strcmp(entry->key,node->entry->key) < 0) {
		return node_put(node,node->left, entry);
	}

	if(strcmp(entry->key,node->entry->key) > 0) {
		return node_put(node,node->right, entry);
	}

	if(strcmp(entry->key,node->entry->key) == 0) {
		node->entry = entry;
		return 1;
	}

	return 0;
}

/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
struct entry_t *node_get(struct node_t *treeRoot, char *key) {

	if(treeRoot == NULL || key == NULL || treeRoot->entry == NULL) {
		return NULL;
	}

	if(strcmp(key,treeRoot->entry->key) < 0) {
		return node_get(treeRoot->left, key);
	}

	if(strcmp(key,treeRoot->entry->key) > 0) {
		return node_get(treeRoot->right, key);
	}

	if(strcmp(key,treeRoot->entry->key) == 0) {
		return treeRoot->entry;
	}

	return NULL;
}

/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int node_calculateTreeHeight(struct node_t *treeRoot) {

	if(treeRoot == NULL) {
		return 0;
	}
	if(treeRoot->entry == NULL) {
		return 0;
	} else {
		int heightOfLeftSubTree = node_calculateTreeHeight(treeRoot->left);
		int heightOfRightSubTree = node_calculateTreeHeight(treeRoot->right);

		return max(heightOfLeftSubTree, heightOfRightSubTree) + 1;  // ao incrementar o valor cada vez que descemos um nivel da arvore obtemos o valor da sua altura
	}

}

struct node_t *node_findLeftmostLeaf(struct node_t *node) {

	struct node_t *current_node = node;

	if(node == NULL || node->entry == NULL) {
		return current_node;
	}

	return node_findLeftmostLeaf(node->left);

}
/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int node_del(struct tree_t *tree,struct node_t *treeRoot, char *key) {

	if(tree == NULL || treeRoot == NULL || treeRoot->entry == NULL) {
		return -1;
	}

	if(strcmp(key,treeRoot->entry->key) < 0) {
		return node_del(tree, treeRoot->left,key);
	} else if(strcmp(key,treeRoot->entry->key) > 0){
		return node_del(tree, treeRoot->right,key);
	}
	if(treeRoot->left->entry == NULL) {
		struct node_t *node = treeRoot->right;
		free(treeRoot->entry);
		treeRoot = NULL;
		free(treeRoot);
		return tree_put(tree, node->entry->key, node->entry->value);
	} else if(treeRoot->right->entry == NULL) {
		struct node_t *node = treeRoot->left;
		free(treeRoot->entry);
		treeRoot = NULL;
		free(treeRoot);
		return tree_put(tree, node->entry->key, node->entry->value);
	}

	//Erro nesta parte. Falta apenas isto para o testDelExistente
	struct node_t *node = node_findLeftmostLeaf(treeRoot->right);

	if(node != NULL && node->entry != NULL) {
		treeRoot->entry = node->entry;
	}

	return node_del(tree, treeRoot->right, node->entry->key);

}

/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
char **node_getKeys(struct tree_t *tree,struct node_t *treeRoot, char **keys, int i){

	if(tree == NULL || treeRoot->entry == NULL) {
		return NULL;
	}

	if(i < tree->nrElements) {
		keys[i] = malloc(strlen(treeRoot->entry->key) + 1);
		strcpy(keys[i], treeRoot->entry->key);
		keys = node_getKeys(tree, treeRoot->left, keys, i + 1);
		keys = node_getKeys(tree, treeRoot->right, keys, i + 1);
	}
	return keys;
}

/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
void **node_getValues(struct tree_t *tree,struct node_t *treeRoot, void **values, int i){

	if(tree == NULL || treeRoot->entry == NULL) {
		return NULL;
	}

	if(i < tree->nrElements) {
		values[i] = malloc(treeRoot->entry->value->datasize);
		strcpy(values[i], treeRoot->entry->key);
		values = node_getValues(tree, treeRoot->left, values, i + 1);
		values = node_getValues(tree, treeRoot->right, values, i + 1);
	}
	return values;
}
/* Função auxiliar max.
 */
int max(int a, int b) {
	if(a > b) {
		return a;
	} else {
		return b;
	}
}
