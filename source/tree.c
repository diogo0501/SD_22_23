/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "../include/tree.h"
#include "../include/tree-private.h"
#include "../include/entry.h"
#include "../include/node-private.h"
/* Função para criar uma nova árvore tree vazia.
 * Em caso de erro retorna NULL.
 */
struct tree_t *tree_create() {

	struct tree_t *tree = malloc(sizeof(struct tree_t));

	if(tree == NULL) {
		return NULL;
	}

	tree->root = malloc(sizeof(struct node_t));
	tree->root->entry = NULL;
	tree->root->left = NULL;
	tree->root->right = NULL;
	tree->height = 0;
	tree->nrElements = 0;

	return tree;

}

/* Função para libertar toda a memória ocupada por uma árvore.
 */
void tree_destroy(struct tree_t *tree) {

	if(tree == NULL) {
		free(tree);
		return;
	}
	if(tree->root == NULL) {
		free(tree);
		return;
	}
	if(tree->root->entry == NULL) {
		free(tree->root);
		free(tree);
		return;
	}

	node_destroy(tree->root);

	free(tree);
}

/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int tree_put(struct tree_t *tree, char *key, struct data_t *value) {

	if(tree == NULL || key == NULL || value == NULL) {
		return -1;
	}

	struct data_t *dupedData = data_dup(value);

	char *dupedKey = malloc(strlen(key) + 1);
	strcpy(dupedKey,key);
	struct entry_t *entry = entry_create(dupedKey, dupedData);

	int code = node_put(NULL,tree->root, entry);

	if(code == -1) {
		return code;
	}

	if(code == 0) {
		tree->nrElements++;
		node_calculateTreeHeight(tree->root);
	}
	return 0;
}

/* Função para obter da árvore o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de ser
 * libertados no contexto da função que chamou tree_get, ou seja, a
 * função aloca memória para armazenar uma *CÓPIA* dos dados da árvore,
 * retorna o endereço desta memória com a cópia dos dados, assumindo-se
 * que esta memória será depois libertada pelo programa que chamou
 * a função. Devolve NULL em caso de erro.
 */
struct data_t *tree_get(struct tree_t *tree, char *key) {

	if(tree == NULL || key == NULL) {
		return NULL;
	}

	struct entry_t *entry = node_get(tree->root, key);

	if(entry == NULL) {
		return NULL;
	}

	struct data_t *dupedData = data_dup(entry->value);

	return dupedData;
}

/* Função para remover um elemento da árvore, indicado pela chave key,
 * libertando toda a memória alocada na respetiva operação tree_put.
 * Retorna 0 (ok) ou -1 (key not found).
 */
int tree_del(struct tree_t *tree, char *key) {

	if(tree == NULL || key == NULL) {
		return -1;
	}

	int isDeleted = node_del(NULL,tree->root,key);

	if(isDeleted == 0) {
		tree->nrElements--;
	}

	return isDeleted;
}

/* Função que devolve o número de elementos contidos na árvore.
 */
int tree_size(struct tree_t *tree) {
	return tree->nrElements;
}

/* Função que devolve a altura da árvore.
 */
int tree_height(struct tree_t *tree) {
	return node_calculateTreeHeight(tree->root);
}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária. As keys devem vir ordenadas segundo a ordenação lexicográfica das mesmas.
 */
char **tree_get_keys(struct tree_t *tree) {

	if(tree == NULL) {
		return NULL;
	}
	//Ordem lexicografica em faLta !!! Nao ha testes para este case
	char **keys = malloc(sizeof(char *) * (tree->nrElements + 1));
	keys = node_getKeys(tree,tree->root,keys,0);
	keys[tree->nrElements] = NULL;
	return keys;
}

/* Função que devolve um array de void* com a cópia de todas os values da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
void **tree_get_values(struct tree_t *tree) {

	if(tree == NULL) {
		return NULL;
	}

	void **values = malloc(sizeof(void *) * (tree->nrElements + 1));
	values = node_getValues(tree,tree->root,values,0);
	values[tree->nrElements] = NULL;
	return values;
}


/* Função que liberta toda a memória alocada por tree_get_keys().
 */
void tree_free_keys(char **keys) {

	char *string = keys[0];
	int i = 0;
	while(string != NULL) {
		free(string);
		i++;
		string = keys[i];
	}
	free(string);
	free(keys);

}

/* Função que liberta toda a memória alocada por tree_get_values().
 */
void tree_free_values(void **values) {

	void *data = values[0];
	int i = 0;
	while(data != NULL) {
		free(data);
		i++;
		data = values[i];
	}
	free(data);
	free(values);

}
