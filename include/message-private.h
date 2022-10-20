#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

//read_all e write_all here 
#include "sdmessage.pb-c.h"

struct message_t {
    MessageT *recv_msg;
};

/* 
 * TODO
 */
int send_all(int sock, uint8_t *buf, int len);

/* 
 * TODO
 */
int recv_all(int sock, uint8_t *buf, unsigned len);


#endif