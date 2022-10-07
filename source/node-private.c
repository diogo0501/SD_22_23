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

//Acho que tem erros
void node_destroy(struct node_t *treeRoot) {

	if(treeRoot == NULL) {
		return;
	}

	if(treeRoot->entry == NULL) {
		entry_destroy(treeRoot->entry);
		free(treeRoot);
		treeRoot = NULL;
		return;
	}

	if(treeRoot->left == NULL && treeRoot->right == NULL) {
		treeRoot = NULL;
		free(treeRoot);
		return;
	}


	node_destroy(treeRoot->left);
	node_destroy(treeRoot->right);

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

struct tuple_t *node_findLeftmostLeaf(struct node_t *parent_node , struct node_t *node) {

	struct tuple_t *ret_val = malloc(sizeof(struct tuple_t));

	if(node == NULL) {
		return NULL;
	}

	if(node->left == NULL && node->right == NULL) {
		ret_val->parent_node = parent_node;
		ret_val->curr_node = node;
		return ret_val;
	}

	if(node->left == NULL || node->right == NULL) {
		if(node->left == NULL) {
			return node_findLeftmostLeaf(node,node->right);
		}
		return node_findLeftmostLeaf(node,node->left);
	}

	return node_findLeftmostLeaf(node,node->left);

}

/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int node_del(struct tree_t *tree , struct node_t *parent_node,struct node_t *treeRoot, char *key) {

	if(treeRoot == NULL || treeRoot->entry == NULL) {
		return -1;
	}

	if(strcmp(key,treeRoot->entry->key) < 0) {
		return node_del(tree,treeRoot, treeRoot->left,key);
	}

	if(strcmp(key,treeRoot->entry->key) > 0){
		return node_del(tree,treeRoot, treeRoot->right,key);
	}

	//Encontrou o node a eliminar
	if(strcmp(key,treeRoot->entry->key) == 0) {
		if(parent_node != NULL) {
			//Quando o nó é uma folha
			if(treeRoot->right == NULL && treeRoot->left == NULL) {
				if(strcmp(key,parent_node->entry->key) < 0) {
					parent_node->left = NULL;
				}
				else {
					parent_node->right = NULL;
				}
				node_destroy(treeRoot);
				return 0;
			}

			//Quando o no so tem um filho
			if(treeRoot->right == NULL || treeRoot->left == NULL) {
				if(treeRoot->right != NULL) {
					if(strcmp(key,parent_node->entry->key) < 0) {
						parent_node->left = treeRoot->right;
						node_destroy(treeRoot);
						return 0;
					}
					else {
						parent_node->right = treeRoot->right;
						node_destroy(treeRoot);
						return 0;
					}
				}
				if(strcmp(key,parent_node->entry->key) < 0) {
					parent_node->left = treeRoot->left;
					node_destroy(treeRoot);
					return 0;
				}
				else {
					parent_node->right = treeRoot->left;
					node_destroy(treeRoot);
					return 0;
				}
			}

			//Quando o no tem dois filhos

			//Tenho que obter o pai deste no para de referencia lo
			struct tuple_t *tuple = node_findLeftmostLeaf(NULL,treeRoot);
			struct node_t *leftest_node = tuple->curr_node;

			if(strcmp(leftest_node->entry->key,tuple->parent_node->entry->key) < 0) {
				tuple->parent_node->left = NULL;
			}
			else {
				tuple->parent_node->right = NULL;
			}

			leftest_node->right = treeRoot->right;
			leftest_node->left = treeRoot->left;

			memcpy(treeRoot,leftest_node,sizeof(struct node_t));
			leftest_node = NULL;

			if(strcmp(key,parent_node->entry->key) < 0) {
				parent_node->left = treeRoot;
			}
			else {
				parent_node->right = treeRoot;
			}

			node_destroy(treeRoot);

			return 0;

		}

		else {
			//Quando o nó é uma folha
			if(treeRoot->right == NULL && treeRoot->left == NULL) {
				tree->root = NULL;
				node_destroy(treeRoot);
				return 0;
			}

			//Quando tem apenas um filho
			if(treeRoot->right == NULL || treeRoot->left == NULL) {
				if(treeRoot->right != NULL) {
					tree->root = treeRoot->right;
					node_destroy(treeRoot);
					return 0;
				}
				else {
					tree->root = treeRoot->left;
					node_destroy(treeRoot);
					return 0;
				}
			}

			struct tuple_t *tuple = node_findLeftmostLeaf(NULL,treeRoot);
			struct node_t *leftest_node = tuple->curr_node;

			if(strcmp(leftest_node->entry->key,tuple->parent_node->entry->key) < 0) {
				tuple->parent_node->left = NULL;
			}
			else {
				tuple->parent_node->right = NULL;
			}

			leftest_node->right = treeRoot->right;
			leftest_node->left = treeRoot->left;

			memcpy(treeRoot,leftest_node,sizeof(struct node_t));
			leftest_node = NULL;

			if(strcmp(key,parent_node->entry->key) < 0) {
				tree->root = treeRoot;
			}
			else {
				tree->root = treeRoot;
			}

			node_destroy(treeRoot);

			return 0;


		}
	}

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

	if(tree == NULL || treeRoot == NULL || treeRoot->entry == NULL) {
		return NULL;
	}

	if(i < tree->nrElements) {
		keys[i] = malloc(strlen(treeRoot->entry->key) + 1);
		strcpy(keys[i], treeRoot->entry->key);
		node_getKeys(tree, treeRoot->left, keys, i + 1);
		node_getKeys(tree, treeRoot->right, keys, i + 2);
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

	//Falta testar
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
