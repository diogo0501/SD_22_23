#include "../include/serialization.h"
#include <string.h>
#include <stdlib.h>

int keyArray_to_buffer(char **keys, char **keys_buf){
    int size = strlen(*keys)+1;
    keys_buf = malloc(sizeof(int)+size);
    if(keys_buf==NULL){
        return -1;
    }

    memcpy(keys_buf,&size,sizeof(int));
    memcpy(keys_buf+sizeof(int),&keys,size);
    return(strlen(*keys_buf));
}

char** buffer_to_keyArray(char *keys_buf, int keys_buf_size){
    char** keys;
    int size= strlen(keys_buf-sizeof(int)+1);
    keys= malloc(size);
    char* keysFromBuffer= keys_buf+sizeof(int);
    memcpy(keys,&keysFromBuffer,size);
    return keys;
}