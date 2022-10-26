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
 * TODO
 */
// int send_all(int sock, uint8_t *buf, int len) {
//     char *ptr = (char *)buf;
//     int total = 0;
//     int bytesleft = len;
//     int n;
//     while (total < len)
//     {
//         {
//             n = send(sock, buf + total, bytesleft, 0);
//             if (n == -1)
//             {
//                 break;
//             }
//             total += n;

//             bytesleft -= n;
//         }
//     }
//     return n == -1 ? -1 : total; //Retorna a totalidade dos bytes enviados
// }

int send_all(int sock, uint8_t *buf, int len) {

    int bufsize = len;

    while(len>0) {

        int res = write(sock, buf, len);
        if(res < 0) {
            if(errno==EINTR) continue;
            perror("end failed:");
            return -1;
        }

        buf += res;
        len -= res;
    }
    return bufsize;
}

/* 
 * TODO
 */
// int recv_all(int sock, uint8_t* buf, unsigned len) {
//     char *ptr = (char *)buf;
//     int total = 0;
//     int bytesleft = len;
//     int n;
//     while (total < len)
//     {
//         {
//             n = recv(sock, buf + total, bytesleft, 0);
//             if (n == -1 || n == 0)
//             {
//                 n = -1;
//                 break;
//             }
//             total += n;

//             bytesleft -= n;
//         }
//     }
//     return n == -1 ? -1 : total; //Retorna a totalidade dos bytes enviados
// }

int recv_all(int sock, uint8_t* buf, unsigned len) {
    int bufsize = len;

    while(len>0) {

        int res = read(sock, buf, len);

        if(res < 1) {
            if(errno==EINTR) continue;
            perror("end failed:");
            return -1;
        }

        buf += res;
        len -= res;
    }
    return bufsize;
}