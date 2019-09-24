#ifndef MVLIST_NODE_H
#define MVLIST_NODE_H

#include "mvl.h"

/* Keeps a pointer to the newest next for better performance. */
typedef struct node_l {
  val_t val;
  struct node_l *newest_next;
  volatile ptlock_t lock;
  uint32_t depth;
  uint32_t newest;
  struct node_l **next;
  timestamp_t *ts;
#ifdef PAD
  char padding[48];
#endif
} node_l_t;

node_l_t *new_node_l(val_t val, uint32_t depth);
void node_set_newest_l(node_l_t *node, node_l_t *next, timestamp_t ts);
void node_retire_edge_l(node_l_t *node, timestamp_t *active,
                        uint32_t num_active);
node_l_t *node_next_from_timestamp_l(node_l_t *node, timestamp_t ts);
void node_delete_l(node_l_t *node);

#endif