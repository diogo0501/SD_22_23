#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "tree.h"

struct node_t {
	struct entry_t *entry;
	struct node_t *left, *right;
};

struct tree_t {
	struct node_t *root;
	int nrElements;
	int height;
};

#endif
