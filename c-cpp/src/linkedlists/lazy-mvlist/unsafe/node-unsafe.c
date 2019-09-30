#include "node-unsafe.h"

node_unsafe_l_t *new_node_unsafe_l(val_t val, node_unsafe_l_t *next)
{
  node_unsafe_l_t *node_unsafe_l;
  
  node_unsafe_l = (node_unsafe_l_t *)malloc(sizeof(node_unsafe_l_t));
  if (node_unsafe_l == NULL) {
    perror("malloc");
    exit(1);
  }
  node_unsafe_l->val = val;
  node_unsafe_l->next = next;
  INIT_LOCK(&node_unsafe_l->lock);	
  return node_unsafe_l;
}

void node_delete_unsafe_l(node_unsafe_l_t *node) {
   DESTROY_LOCK(&node->lock);
   free(node);
}