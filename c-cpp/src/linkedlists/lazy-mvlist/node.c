#include "node.h"

node_l_t *new_node_l(val_t val, uint32_t depth) {
  node_l_t *node_l;
  int i;

  node_l = (node_l_t *)malloc(sizeof(node_l_t));
  if (node_l == NULL) {
    perror("malloc");
    exit(1);
  }
  node_l->val = val;
  node_l->depth = depth;
  node_l->newest = 0;
  node_l->newest_next = NULL;
  INIT_LOCK(&node_l->lock);
  return node_l;
}

void node_recycle_edge_l(node_l_t *node, node_l_t *next, timestamp_t ts,
                         timestamp_t *active, uint32_t num_active) {
  timestamp_t curr_rq_ts, curr_edge_ts;
  int i, a, idx, found;
  uint32_t depth, newest;

  /* Find an edge to recycle. */
  depth = node->depth;
  newest = node->newest;
  if (num_active > 0) {
    found = 0;
    /* TODO(jacnel): More elegantly find the index to recycle. */
    for (i = 1; !found && i < depth; ++i) {
      idx = (newest + i) % depth;
      curr_edge_ts = node->ts[idx];
      for (a = 0; !found && a < num_active; ++a) {
        curr_rq_ts = active[a];
        if (curr_edge_ts == NULL_TIMESTAMP || curr_edge_ts < curr_rq_ts) {
          found = 1; /* Edge at index `to_recycle' may be recycled. */
        }
      }
    }
    assert(found);
  } else {
    idx = (newest + 1) % depth;
  }

  /* Replace the edge with the new next pointer. */
  node->newest_next = next; /* Visible to normal operations. */
  node->next[idx] = NULL;
  node->ts[idx] = ts;
  node->next[idx] = next;
  node->newest = idx; /* Visible to RQs. */
}

node_l_t *node_next_from_timestamp_l(node_l_t *node, timestamp_t ts) {
  int i, next_idx, temp_idx;
  uint32_t depth, start;
  timestamp_t curr_ts;
  depth = node->depth;
  start = node->newest;
  next_idx = start;
  for (i = 0; i < depth; ++i) {
    temp_idx = (depth + (start - i)) % depth;
    curr_ts = node->ts[temp_idx];
    if (curr_ts > node->ts[next_idx] && curr_ts <= ts &&
        curr_ts != NULL_TIMESTAMP) {
      while (node->next[temp_idx] == NULL)
        ;
      assert(node->next[temp_idx] != NULL);
      next_idx = temp_idx;
    }
  }
  return node->next[next_idx];
}

void node_delete_l(node_l_t *node) {
  DESTROY_LOCK(&node->lock);
  free(node);
}