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

void node_retire_edge_l(node_l_t *node, timestamp_t *active,
                        uint32_t num_active) {
  timestamp_t curr_rq, curr_edge, next_edge;
  int a, i, j, curr_idx, next_idx, new_idx, prev_idx;
  uint32_t depth, newest;

  if (num_active == 0) {
    return;
  }

  depth = node->depth;
  newest = (node->newest + 1) % depth;
  for (i = 0, a = 0; i < depth - 1; ++i, ++a) {
    if (a < num_active) {
      curr_rq = active[a];
    } else {
      /* Automatically trigger reuse. */
      curr_rq = MAX_TIMESTAMP;
    }

    curr_idx = (newest + i) % depth;
    next_idx = (curr_idx + 1) % depth;
    curr_edge = node->ts[curr_idx];
    next_edge = node->ts[next_idx];

    if (curr_edge == NULL_TIMESTAMP ||
        (curr_edge < curr_rq && next_edge <= curr_rq)) {
      for (j = 0; j < depth - 1; ++j) {
        new_idx = (depth + (curr_idx - j)) % depth;
        if (new_idx == newest) {
          break;
        } else {
          /* Shift edges to make room. */
          prev_idx = (depth + (new_idx - 1)) % depth;
          node->next[new_idx] = node->next[prev_idx];
          node->ts[new_idx] = node->ts[prev_idx];
        }
      }
      return;
    }
  }
  assert(0);
}

node_l_t *node_next_from_timestamp_l(node_l_t *node, timestamp_t ts) {
  int i, idx;
  uint32_t depth;
  depth = node->depth;
  for (i = 0; i < depth; ++i) {
    idx = (depth + (node->newest - i)) % depth;
    if (node->ts[idx] <= ts) {
      return node->next[idx];
    }
  }
  return NULL;
}

void node_delete_l(node_l_t *node) {
  DESTROY_LOCK(&node->lock);
  free(node);
}