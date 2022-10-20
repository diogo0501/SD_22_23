//read_all e write_all here 
#include "sdmessage.pb-c.h"

struct message_t {
    MessageT *recv_msg;
};


int send_all(int sock, uint8_t *buf, int len);

int recv_all(int sock, uint8_t *buf, unsigned len);