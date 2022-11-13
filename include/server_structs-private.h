#ifndef _SERVER_STRUCTS_PRIVATE_H
#define _SERVER_STRUCTS_PRIVATE_H

struct op_proc {
    int max_proc; //regista o maior identificador das operações de escrita já concluídas
    int *in_progress; //regista o identificador das operações de escrita que estão a ser atendidas por um conjunto de threads dedicadas às escritas.
};

struct request_t {
    int op_n; //o número da operação
    int op; //a operação a executar. op=0 se for um delete, op=1 se for um put
    char* key; //a chave a remover ou adicionar
    struct data_t *data; // os dados a adicionar em caso de put, ou NULL em caso de delete
    struct request_t *succ;
    //adicionar campo(s) necessário(s) para implementar fila do tipo produtor/consumidor
};

#endif