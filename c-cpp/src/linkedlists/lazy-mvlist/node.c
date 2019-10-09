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

/* Find an edge to recycle. It WILL NOT be the newest edge */
int node_find_edge_to_recycle_l(node_l_t *node, timestamp_t *active,
                                timestamp_t oldest_active,
                                timestamp_t newest_active,
                                uint32_t num_active) {
  timestamp_t active_rq_ts, to_recycle_ts, next_edge_ts, temp_edge_ts;
  int i, j, a, to_recycle, found;
  uint32_t depth, newest;

  depth = node->depth;
  newest = node->newest;

  if (num_active == 0 || oldest_active >= node->ts[newest]) {
    /* If there are no active RQs or the oldest active RQ is newer than the
     * newest edge then any edge can be recycled */
    return (newest + 1) % depth;
  }

  for (i = 1; i < depth; ++i) {
    to_recycle = (newest + i) % depth;
    to_recycle_ts = node->ts[to_recycle];

    if (to_recycle_ts == NULL_TIMESTAMP && to_recycle_ts > newest_active) {
      return to_recycle;
    } else {
      /* The edge is not being set for the first time and the edge is older than
       * the newest active RQ, then we must check if we can recycle it */
      next_edge_ts = MAX_TIMESTAMP;
      for (j = 1; j < depth; ++j) {
        /* Search for the next newest edge in the node */
        temp_edge_ts = node->ts[(to_recycle + j) % depth];
        if (temp_edge_ts > to_recycle_ts && temp_edge_ts < next_edge_ts) {
          next_edge_ts = temp_edge_ts;
        }
      }
      /* Only the newest edge should have no greater edge */
      assert(next_edge_ts != MAX_TIMESTAMP);

      /* Assume the edge is recylcable and check for conflicts in active RQs*/
      for (a = 0; a < num_active; ++a) {
        active_rq_ts = active[a];
        if (to_recycle_ts < active_rq_ts && next_edge_ts <= active_rq_ts) {
          return to_recycle;
        }
      }
    }
  }
  assert(0); /* This should never be reached */
}

node_l_t *node_next_from_timestamp_l(node_l_t *node, timestamp_t ts) {
  int i, next_idx, temp_idx;
  uint32_t depth, start, iter;
  timestamp_t curr_ts;
  volatile node_l_t *next;
  depth = node->depth;
  start = node->newest;
  next_idx = start;
  for (i = 0; i < depth; ++i) {
    temp_idx = (depth + (start - i)) % depth;
    curr_ts = node->ts[temp_idx];
    if (curr_ts > node->ts[next_idx] && curr_ts <= ts &&
        curr_ts != NULL_TIMESTAMP) {
      assert(node->next[temp_idx] != NULL);
      next_idx = temp_idx;
    }
  }
  return node->next[next_idx];
}

void node_dump_l(node_l_t *node) {
  int i;
  printf("node: %p\n", node);
  for (i = 0; i < node->depth; ++i) {
    printf("\t[%lu, %p]\n", node->ts[i], node->next[i]);
  }
}

void node_delete_l(node_l_t *node) {
  DESTROY_LOCK(&node->lock);
  free(node);
}