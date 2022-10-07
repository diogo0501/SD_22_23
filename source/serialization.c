/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
 */

#include "serialization.h"
#include <string.h>
#include <stdlib.h>

int keyArray_to_buffer(char **keys, char **keys_buf){
    int size = sizeof(keys)/sizeof(char*);
    keys_buf = malloc(sizeof(int)+size);
    if(keys_buf==NULL){
        return -1;
    }

    memcpy(keys_buf,&size,sizeof(int));
    memcpy(keys_buf+sizeof(int),&keys,size);
    return size+sizeof(int);
}

char** buffer_to_keyArray(char *keys_buf, int keys_buf_size){
    char** keys;
    int size= strlen(keys_buf-sizeof(int)+1);
    keys= malloc(size);
    char* keysFromBuffer= keys_buf+sizeof(int);
    memcpy(keys,&keysFromBuffer,size);
    return keys;
}