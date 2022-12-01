#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

struct rtree_t {
    char *ip;
    short port;
    int sockfd;
    struct rtree_t *head;
    struct rtree_t *tail;
};



#endif