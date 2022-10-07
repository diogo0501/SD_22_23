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

struct tree_t *tree_create() {

	//Criação e alocação da struct tree_t
	struct tree_t *tree = malloc(sizeof(struct tree_t));

	//Retorna null em caso de erro
	if(tree == NULL) {
		return NULL;
	}

	//Declaração de valores da struct tree_t
	tree->root = malloc(sizeof(struct node_t));

	tree->root->entry = NULL;
	tree->root->left = NULL;
	tree->root->right = NULL;

	tree->height = 0;
	tree->nrElements = 0;

	return tree;

}

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

	/*
	 * Caso a árvore nao seja null nem a sua raiz,
	 * recorre-se ao node_destroy para destruir a
	 * raiz bem como todos os seus descendes
	 */
	node_destroy(tree->root);

	free(tree);
}

int tree_put(struct tree_t *tree, char *key, struct data_t *value) {

	//Retorna -1 em caso de erro na tree, key ou value
	if(tree == NULL || key == NULL || value == NULL) {
		return -1;
	}

	//Struct cópia da struct value
	struct data_t *dupedData = data_dup(value);

	//Criação da key e da entry a partir dos argumentos dados
	char *dupedKey = malloc(strlen(key) + 1);
	strcpy(dupedKey,key);
	struct entry_t *entry = entry_create(dupedKey, dupedData);

	//Valor return da execução do node_put
	int code = node_put(NULL,tree->root, entry);

	//Em caso de erro retorna -1
	if(code == -1) {
		return code;
	}

	//No caso de sucesso, o numero de elementos é incrementado
	tree->nrElements++;

	return 0;
}

struct data_t *tree_get(struct tree_t *tree, char *key) {

	//Retorna null em caso de erro na tree ou na key
	if(tree == NULL || key == NULL) {
		return NULL;
	}

	//Entry obtida na árvore
	struct entry_t *entry = node_get(tree->root, key);

	//No caso da entry não existir
	if(entry == NULL) {
		return NULL;
	}

	//Cópia do value existente na entry obtida
	struct data_t *dupedData = data_dup(entry->value);

	return dupedData;
}

int tree_del(struct tree_t *tree, char *key) {

	//Retorna -1 em caso de erro na tree ou na key
	if(tree == NULL || key == NULL) {
		return -1;
	}

	//Valor obtida pela exucação do node_del
	int isDeleted = node_del(tree,NULL,tree->root,key);

	//Em caso de sucesso, o numero de elementos da tree é decrementado
	if(isDeleted == 0) {
		tree->nrElements--;
	}

	return isDeleted;
}

int tree_size(struct tree_t *tree) {
	return tree->nrElements;
}

int tree_height(struct tree_t *tree) {
	return node_calculateTreeHeight(tree->root);
}

char **tree_get_keys(struct tree_t *tree) {

	//Retorna null em caso de erro
	if(tree == NULL) {
		return NULL;
	}

	//Criação de char** e a alocação de memória devida
	char **keys = malloc(sizeof(char *) * (tree->nrElements + 1));

	//Atribuição de valores a char** keys
	keys = node_getKeys(tree,tree->root,keys,0);

	//Atribuição do valor null à última posição do char**
	keys[tree->nrElements] = NULL;

	//Organização do char** por ordem lexicográfica
	int i,j;
	char* temp;

	for (i = 0; i < tree->nrElements - 2; i++) {
		for (j = i+1; j < tree->nrElements - 1; j++) {
			if (keys[i] < keys[j]) {
				temp = keys[i];
				keys[i] = keys[j];
				keys[j] = temp;
			}
		}
	}

	return keys;
}

void **tree_get_values(struct tree_t *tree) {

	//Retorna null em caso de erro na tree
	if(tree == NULL) {
		return NULL;
	}

	//Criação do void** e devida alocação de memória
	void **values = malloc(sizeof(void *) * (tree->nrElements + 1));

	//Atribuição de valores a void**
	values = node_getValues(tree,tree->root,values,0);

	//Atribuição do valor null à última posição do void**
	values[tree->nrElements] = NULL;

	return values;
}

void tree_free_keys(char **keys) {

	char *string = keys[0];
	int i = 0;

	//Liberta todas as trings até ao final do char**
	while(string != NULL) {
		free(string);
		i++;
		string = keys[i];
	}

	//Liberta a última posição do char** e o char**
	free(string);
	free(keys);

}

void tree_free_values(void **values) {

	void *data = values[0];
	int i = 0;

	//Liberta todas as trings até ao final do void**
	while(data != NULL) {
		free(data);
		i++;
		data = values[i];
	}

	//Liberta a última posição do char** e o char**
	free(data);
	free(values);

}
