#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

struct rtree_t {
    char *ip;
    short port;
    int sockfd;
    struct rtree_t *head;
    char *head_path;
    struct rtree_t *tail;
    char *tail_path;
};



#endif