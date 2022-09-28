/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
*/

#include "entry.h"
#include "data.h"
#include <stdlib.h>
#include <string.h>

/* Função que cria uma entry, reservando a memória necessária para a
 * estrutura e inicializando os campos key e value, respetivamente, com a
 * string e o bloco de dados passados como parâmetros, sem reservar
 * memória para estes campos.

 */
struct entry_t *entry_create(char *key, struct data_t *data) {
    struct entry_t *et = malloc(sizeof(struct entry_t));
    et->key = key;
    et->value = data;
    return et;
}

/* Função que elimina uma entry, libertando a memória por ela ocupada
 */
void entry_destroy(struct entry_t *entry) {
    if(entry == NULL) {
        return;
    }else {
        data_destroy(entry->value);
        free(entry->key);
        entry->key = NULL;
        free(entry);
        entry = NULL;
    }
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 */
struct entry_t *entry_dup(struct entry_t *entry) {
    char *dupedKey = malloc(strlen(entry->key));
    memcpy(dupedKey, entry->key, strlen(entry->key) + 1);
    struct data_t *dupedData = data_dup(entry->value);
    return entry_create(dupedKey, dupedData);
}

/* Função que substitui o conteúdo de uma entrada entry_t.
*  Deve assegurar que destroi o conteúdo antigo da mesma.
*/
void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value) {
    free(entry->key);
    entry->key = NULL;
    entry->key = new_key;
    free(entry->value);
    entry->value = NULL;
    entry->value = new_value;
}

/* Função que compara duas entradas e retorna a ordem das mesmas.
*  Ordem das entradas é definida pela ordem das suas chaves.
*  A função devolve 0 se forem iguais, -1 se entry1<entry2, e 1 caso contrário.
*/
int entry_compare(struct entry_t *entry1, struct entry_t *entry2) {
    int m = strcmp(entry1->key,entry2->key);
    if(m<0) {
        return -1;
    }    
    if(m>0) {
        return 1;
    }
    return 0;
}
