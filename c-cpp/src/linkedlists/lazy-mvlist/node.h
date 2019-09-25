#ifndef MVLIST_NODE_H
#define MVLIST_NODE_H

#include "mvl.h"

node_l_t *new_node_l(val_t val, uint32_t depth);
void node_set_newest_l(node_l_t *node, node_l_t *next, timestamp_t ts);
void node_retire_edge_l(node_l_t *node, timestamp_t *active,
                        uint32_t num_active);
node_l_t *node_next_from_timestamp_l(node_l_t *node, timestamp_t ts);
void node_delete_l(node_l_t *node);

#endif