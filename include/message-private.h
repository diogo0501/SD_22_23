#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

//read_all e write_all here 
#include "sdmessage.pb-c.h"

struct message_t {
    MessageT *recv_msg;
};

/* 
 * Returns the number of bytes sent through given socket
 */
int send_all(int sock, uint8_t *buf, int len);

/* 
 * Returns the number of bytes received through given socket
 */
int recv_all(int sock, uint8_t *buf, unsigned len);


#endif
