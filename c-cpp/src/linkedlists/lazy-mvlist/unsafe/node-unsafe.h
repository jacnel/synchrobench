#ifndef MVLIST_UNSAFE_NODE_H
#define MVLIST_UNSAFE_NODE_H

#include "unsafe.h"

node_unsafe_l_t *new_node_unsafe_l(val_t val, node_unsafe_l_t *next);
void node_delete_unsafe_l(node_unsafe_l_t *node);

#endif