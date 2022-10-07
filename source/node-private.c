/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
 */

#include "entry.h"
#include "node-private.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Função para libertar toda a memória ocupada por um node.
 */
void node_destroy(struct node_t *treeRoot) {

	if(treeRoot == NULL) {
		return;
	}

	//Se entry == NULL destroi entry e o node
	if(treeRoot->entry == NULL) {
		entry_destroy(treeRoot->entry);
		free(treeRoot);
		treeRoot = NULL;
		return;
	}

	//Quando o nó é uma folha
	if(treeRoot->left == NULL && treeRoot->right == NULL) {
		treeRoot = NULL;
		free(treeRoot);
		return;
	}

	/*
	 * Percorre e destroi sub-árvores através da recursão
	 */
	node_destroy(treeRoot->left);
	node_destroy(treeRoot->right);

}

/* Função para adicionar um node contendo o par chave-valor dado à árvore.
 * Retorna 0 (ok), -1 em caso de erro ou 1 em caso de nao ter havido a necessidade de adicionar
 * um novo nodulo a arvore
 */
int node_put(struct node_t *parent_node, struct node_t *node, struct entry_t *entry) {

	//Retorna -1 quando entry é null
	if(entry == NULL) {
		return -1;
	}

	//Quando a root é null
	if(parent_node == NULL && node == NULL) {
		node->entry = entry;
		node->left = NULL;
		node->right = NULL;
		return 0;
	}

	//Criação e referenciação do node visto que este é null
	if(node == NULL && parent_node != NULL) {

		struct node_t *tmpRoot = malloc(sizeof(struct node_t));
		tmpRoot->entry = entry;
		tmpRoot->left = NULL;
		tmpRoot->right = NULL;
		node = tmpRoot;

		//Cria referencia no nó pai para o nó recentemente criado, dependendo do lado
		if(strcmp(entry->key,parent_node->entry->key) < 0) {
			parent_node->left = node;
		}
		else {
			parent_node->right = node;
		}
		return 0;
	}

	//Substituição da entry pela entry dada
	if(node->entry == NULL) {
		node->entry = entry;
		return 0;
	}

	/*
	 * Comparação dos valores ascii das keys para perceber que sub-árvore ingressar.
	 * Utiliza a recursão para percorrer a árvore
	 */
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

/* Função para obter da árvore o valor associado à chave key.
 * Devolve NULL em caso de erro.
 */
struct entry_t *node_get(struct node_t *treeRoot, char *key) {

	//Retorna NULL em caso de erro na entry ou key
	if(treeRoot == NULL || key == NULL || treeRoot->entry == NULL) {
		return NULL;
	}

	/*
	 * Comparação dos valores ascii das keys para perceber que sub-árvore ingressar.
	 * Utiliza a recursão para percorrer a árvore
	 */

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

/* Função que dado um node calcula a altura da árvore.
 */
int node_calculateTreeHeight(struct node_t *treeRoot) {

	if(treeRoot == NULL) {
		return 0;
	}
	if(treeRoot->entry == NULL) {
		return 0;
	} else {

		/*
		 * Percorre as duas sub-árvores através de recursão e
		 * calcula as suas alturas
		 */
		int heightOfLeftSubTree = node_calculateTreeHeight(treeRoot->left);
		int heightOfRightSubTree = node_calculateTreeHeight(treeRoot->right);

		//Retorna o nó atual mais alturas das sub-árvores
		return 1 + max(heightOfLeftSubTree, heightOfRightSubTree);
	}

}

/* Função que retorna o node contendo a menor key da árvore.
 */
struct tuple_t *node_findLeftmostLeaf(struct node_t *parent_node , struct node_t *node) {

	//Cria tuplo a retornar e aloca memória
	struct tuple_t *ret_val = malloc(sizeof(struct tuple_t));

	//Retorna null em caso de erro
	if(node == NULL) {
		return NULL;
	}

	//Se o nó atual for a folha pretendida
	if(node->left == NULL && node->right == NULL) {
		ret_val->parent_node = parent_node;
		ret_val->curr_node = node;
		return ret_val;
	}

	/*
	 * Se o nó atual tiver um ou dois filhos,
	 * a pesquisa deve seguir o mais à esquerda possível
	 */
	if(node->left == NULL || node->right == NULL) {
		if(node->left == NULL) {
			return node_findLeftmostLeaf(node,node->right);
		}
		return node_findLeftmostLeaf(node,node->left);
	}

	return node_findLeftmostLeaf(node,node->left);

}

/* Função para remover um elemento da árvore, indicado pela chave key,
 * Retorna um node (ok) ou NULL (error).
 */
int node_del(struct tree_t *tree, struct node_t *parent_node,struct node_t *treeRoot, char *key) {

	//Retorna -1 em caso de erro na treeRoot
	if(treeRoot == NULL || treeRoot->entry == NULL) {
		return -1;
	}

	/*
	 * Comparação dos valores ascii das keys para perceber que sub-árvore ingressar.
	 * Utiliza a recursão para percorrer a árvore
	 */
	if(strcmp(key,treeRoot->entry->key) < 0) {
		return node_del(tree,treeRoot, treeRoot->left,key);
	}

	if(strcmp(key,treeRoot->entry->key) > 0){
		return node_del(tree,treeRoot, treeRoot->right,key);
	}

	//Quando a key do nó atual é igual à key especificada
	if(strcmp(key,treeRoot->entry->key) == 0) {

		//Quando o nó atual NÃO é a raíz da árvore
		if(parent_node != NULL) {

			//Quando o nó atual é uma folha
			if(treeRoot->right == NULL && treeRoot->left == NULL) {

				//Derreferencia o nó atual do seu nó pai, quebrando a ligação entre eles
				if(strcmp(key,parent_node->entry->key) < 0) {
					parent_node->left = NULL;
				}
				else {
					parent_node->right = NULL;
				}

				//Destroi o nó atual
				node_destroy(treeRoot);

				return 0;
			}

			//Quando o nó atual so tem um filho
			if(treeRoot->right == NULL || treeRoot->left == NULL) {

				//Se o nó filho é o nó do lado direito
				if(treeRoot->right != NULL) {

					/*
					 * Referência do nó pai deixa de estar sobre o
					 * nó atual mas sim ao filho do nó atual, tornando a ligação
					 * direta
					 */

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

				//Se o nó filho é o nó do lado esquerdo
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

			//Quando o nó atual tem dois filhos

			//Tuplo que contem a folha mais à esquerda da árvore e o seu pai
			struct tuple_t *tuple = node_findLeftmostLeaf(NULL,treeRoot);

			//Folha mais à esquerda da árvore
			struct node_t *leftest_node = tuple->curr_node;

			//A folha mais à esquerda é apagada da árvore para evitar duplicações
			if(strcmp(leftest_node->entry->key,tuple->parent_node->entry->key) < 0) {
				tuple->parent_node->left = NULL;
			}
			else {
				tuple->parent_node->right = NULL;
			}

			/*
			 *A folha mais à esquerda da árvore vai assumir o lugar do nó a ser eliminado.
			 *Assim, estabelece-se referencias para os filhos do nó atual
			 */
			leftest_node->right = treeRoot->right;
			leftest_node->left = treeRoot->left;

			//Substituição do nó a ser a eliminado pela folha
			memcpy(treeRoot,leftest_node,sizeof(struct node_t));
			leftest_node = NULL;

			//O pai do nó atual passa, agora, a referenciar a sua substituição
			if(strcmp(key,parent_node->entry->key) < 0) {
				parent_node->left = treeRoot;
			}
			else {
				parent_node->right = treeRoot;
			}

			//Destroi o nó atual
			node_destroy(treeRoot);

			return 0;

		}

		//Quando o nó atual É a raíz da árvore
		else {

			//Quando o nó é uma folha
			if(treeRoot->right == NULL && treeRoot->left == NULL) {
				tree->root = NULL;
				node_destroy(treeRoot);
				return 0;
			}

			//Quando o nó atual tem apenas um filho, esse filho passa a ser a nova raíz
			if(treeRoot->right == NULL || treeRoot->left == NULL) {

				//Se o nó filho é o nó do lado direito
				if(treeRoot->right != NULL) {
					tree->root = treeRoot->right;
					node_destroy(treeRoot);
					return 0;
				}

				//Se o nó filho é o nó do lado esquerdo
				else {
					tree->root = treeRoot->left;
					node_destroy(treeRoot);
					return 0;
				}
			}

			//Quando o nó atual tem dois filhos

			//Tuplo que contem a folha mais à esquerda da árvore e o seu pai
			struct tuple_t *tuple = node_findLeftmostLeaf(NULL,treeRoot);

			//Folha mais à esquerda da árvore
			struct node_t *leftest_node = tuple->curr_node;

			//A folha mais à esquerda é apagada da árvore para evitar duplicações
			if(strcmp(leftest_node->entry->key,tuple->parent_node->entry->key) < 0) {
				tuple->parent_node->left = NULL;
			}
			else {
				tuple->parent_node->right = NULL;
			}

			/*
			 *A folha mais à esquerda da árvore vai assumir o lugar do nó a ser eliminado.
			 *Assim, estabelece-se referencias para os filhos do nó atual
			 */
			leftest_node->right = treeRoot->right;
			leftest_node->left = treeRoot->left;

			//Substituição do nó a ser a eliminado pela folha
			memcpy(treeRoot,leftest_node,sizeof(struct node_t));
			leftest_node = NULL;

			//O pai do nó atual passa, agora, a referenciar a sua substituição
			if(strcmp(key,parent_node->entry->key) < 0) {
				tree->root = treeRoot;
			}
			else {
				tree->root = treeRoot;
			}

			//Destroi o nó atual
			node_destroy(treeRoot);

			return 0;


		}
	}

}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária. As keys devem vir ordenadas segundo a ordenação lexicográfica das mesmas.
 */
char **node_getKeys(struct tree_t *tree,struct node_t *treeRoot, char **keys, int i){

	//Retorna null em caso de erro no treeRoot ou na tree
	if(tree == NULL || treeRoot == NULL || treeRoot->entry == NULL) {
		return NULL;
	}

	//Se o index for menor que o numero de elementos, obtém as keys das sub-árvores
	if(i < tree->nrElements) {
		keys[i] = malloc(strlen(treeRoot->entry->key) + 1);
		strcpy(keys[i], treeRoot->entry->key);
		node_getKeys(tree, treeRoot->left, keys, i + 1);
		node_getKeys(tree, treeRoot->right, keys, i + 2);
	}

	return keys;
}

/* Função que devolve um array de void* com a cópia de todas os values da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
void **node_getValues(struct tree_t *tree,struct node_t *treeRoot, void **values, int i){

	//Retorna null em caso de erro na tree ou na treeRoot
	if(tree == NULL || treeRoot->entry == NULL) {
		return NULL;
	}

	//Se o index for menor que o numero de elementos, obtém os values das sub-árvores
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
	return a > b ? a : b;
}
