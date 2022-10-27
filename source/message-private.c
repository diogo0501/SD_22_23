/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
*/

#include "message-private.h"
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

/* 
 * Returns the number of bytes received through given socket
 */
int recv_all(int sock, uint8_t* buf, unsigned len) {
    int bufsize = len;

    while(len>0) {

        int res = read(sock, buf, len);

        if(res < 1) {
            if(errno==EINTR) continue;
            perror("read failed");
            return -1;
        }

        buf += res;
        len -= res;
    }
    return bufsize;
}

/* 
 * Returns the number of bytes sent through given socket
 */
int send_all(int sock, uint8_t *buf, int len) {

    int bufsize = len;

    while(len>0) {

        int res = write(sock, buf, len);
        if(res < 0) {
            if(errno==EINTR) continue;
            perror("write failed");
            return -1;
        }

        buf += res;
        len -= res;
    }
    return bufsize;
}