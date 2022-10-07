/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
 */

#include "serialization.h"
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>

/* Serializa todas as keys presentes no array de strings keys
 * para o buffer keys_buf que será alocado dentro da função.
 * O array de keys a passar em argumento pode ser obtido através 
 * da função tree_get_keys. Para além disso, retorna o tamanho do
 * buffer alocado ou -1 em caso de erro.
 */
int keyArray_to_buffer(char **keys, char **keys_buf){

    if(keys == NULL || keys_buf == NULL) {
        return -1;
    }

    int totalSize = 0;

    for(int i = 0; i < keys[i] != NULL; i++) {
        int keySize = strlen(keys[i]) + 1;
        totalSize += (keySize + sizeof(int));
    }

    char **keys_buf = malloc(totalSize);

    for(int i = 0; i < keys[i] != NULL; i++) {
        int keySize = strlen(keys[i]) + 1;
        memcpy(keys_buf[i], &keySize, sizeof(int));
        memcpy(keys_buf[i] + sizeof(int), &keys[i], keySize);
    }

    return totalSize;
}

/* De-serializa a mensagem contida em keys_buf, com tamanho
 * keys_buf_size, colocando-a e retornando-a num array char**,
 * cujo espaco em memória deve ser reservado. Devolve NULL
 * em caso de erro.
 */
char** buffer_to_keyArray(char *keys_buf, int keys_buf_size){

    if(keys_buf == NULL || keys_buf_size <= 0) {
        return NULL;
    }

    char **keyArray;

    int i = 0;
    while(keys_buf_size != 0) {
        int *keySize = malloc(sizeof(int));
        memcpy(keySize, keys_buf, sizeof(int));
        keyArray[i] = malloc(*keySize);
        memcpy(keyArray[i], keys_buf + sizeof(int), *keySize);
        keys_buf_size -= (sizeof(int) + *keySize);
        free(keySize);
        i++;
    }

    return keyArray;
}