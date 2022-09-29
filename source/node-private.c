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
    if(node->entry == NULL) {
        return;
    }
    node_destroy(node->left);
    node_destroy(node->right);
    if(node->left == NULL && node->right == NULL) {
        free(node);
        return;
    }

}


int node_put(struct node_t *treeRoot, struct entry_t *entry) {
    
    if(entry == NULL) {
        return -1;
    }

    if(treeRoot->entry == NULL) {
        treeRoot->entry = entry;
        treeRoot->left = NULL;
        treeRoot->left->entry = NULL;
        treeRoot->right = NULL;
        treeRoot->right->entry = NULL;
        return 0;
    }

    if(entry->key < treeRoot->entry->key) {
        node_put(treeRoot->left, entry);
    }

    if(entry->key > treeRoot->entry->key) {
        node_put(treeRoot->right, entry);
    }

    if(entry->key == treeRoot->entry->key) {
        treeRoot->entry = entry;
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

    if(key == NULL) {
        return NULL;
    }

    if(key < treeRoot->entry->key) {
        node_get(treeRoot->left, key);
    }

    if(key > treeRoot->entry->key) {
        node_get(treeRoot->right, key);
    }

    if(key == treeRoot->entry->key) {
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

    if(treeRoot->entry == NULL) {
        return 0;
    } else {
        int heightOfLeftSubTree = node_calculateTreeHeight(treeRoot->left);
        int heightOfRightSubTree = node_calculateTreeHeight(treeRoot->right);

        return max(heightOfLeftSubTree, heightOfRightSubTree) + 1;              // ao incrementar o valor cada vez que descemos um nivel da arvore obtemos o valor da sua altura
    }

}

struct node_t *node_findLeftmostLeaf(struct node_t *node) {

    struct node_t *current_node = node;

    while(current_node->entry && current_node->left->entry != NULL) {
        current_node = current_node->left;
    }

    return current_node;
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

    if(treeRoot->entry == NULL) {
        return -1;
    }

    if(key < treeRoot->entry->key) {
        node_del(tree, treeRoot->left,key);
    } else if(key > treeRoot->entry->key){
        node_del(tree, treeRoot->right,key);
    } else {
        if(treeRoot->left->entry == NULL) {
            struct node_t *node = treeRoot->right;
            free(treeRoot);
            treeRoot = NULL;
            free(treeRoot->entry);
            return tree_put(tree, node->entry->key, node->entry->value);
        } else if(treeRoot->right->entry == NULL) {
            struct node_t *node = treeRoot->left;
            free(treeRoot);
            treeRoot = NULL;
            free(treeRoot->entry);
            return tree_put(tree, node->entry->key, node->entry->value);           
        }
    
    struct node_t *node = node_findLeftmostLeaf(treeRoot->right);

    treeRoot->entry = node->entry;

    node_del(tree, treeRoot->right, node->entry->key);
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