/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
*/

#include "data.h"
#include <stdlib.h>
#include <string.h>

/* Função que cria um novo elemento de dados data_t, reservando a memória
 * necessária para armazenar os dados, especificada pelo parâmetro size 
 */
struct data_t *data_create(int size) {
    if(size < 1){                           //para passar nos testes
        return NULL;
    }                            
    struct data_t *dt = malloc(sizeof(struct data_t));
    dt->datasize = size;
    dt->data = malloc(size);

    return dt;

}

/* Função que cria um novo elemento de dados data_t, inicializando o campo
 * data com o valor passado no parâmetro data, sem necessidade de reservar
 * memória para os dados.
 */
struct data_t *data_create2(int size, void *data) {
    if(size < 1 || data == NULL) {
        return NULL;
    }
    struct data_t *dt = malloc(sizeof(struct data_t));
    dt->data = data;
    dt->datasize = size;
    return dt;

} 

/* Função que elimina um bloco de dados, apontado pelo parâmetro data,
 * libertando toda a memória por ele ocupada.
 */
void data_destroy(struct data_t *data) {
    if(data == NULL) {
        return;                                 // nao se pode retornar NULL
    }
    free(data->data);
    data->data = NULL;
    free(data);                             //libertar o proprio ponteiro e o seu conteudo
    data = NULL;

}

/* Função que duplica uma estrutura data_t, reservando toda a memória
 * necessária para a nova estrutura, inclusivamente dados.
 */
struct data_t *data_dup(struct data_t *data) {
    if(data == NULL || data->data == NULL || data->datasize <= 0) {
        return NULL;
    } else {
        void *dupeddata = malloc(data->datasize);
        memcpy(dupeddata,data->data,data->datasize); //para passar nos testes eh necessario ser uma copia, e nao apenas uma referencia
        return data_create2(data->datasize, dupeddata);
    }
}

/* Função que substitui o conteúdo de um elemento de dados data_t.
*  Deve assegurar que destroi o conteúdo antigo do mesmo.
*/
void data_replace(struct data_t *data, int new_size, void *new_data) {
    data->datasize = new_size;
    data->data = new_data;
}
