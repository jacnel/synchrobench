/*
 * File:
 *   linkedlist-lock.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Lock-based linked list implementation of an integer set
 *
 * Copyright (c) 2009-2010.
 *
 * linkedlist-lock.c is part of Synchrobench
 *
 * Synchrobench is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "linkedlist-lock.h"

node_l_t *new_node_l(val_t val, node_l_t *next, timestamp_t ts, uint32_t depth,
                     int transactional) {
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
  node_l->next = (node_l_t **)malloc(sizeof(node_l_t *) * depth);
  if (node_l->next == NULL) {
    perror("malloc");
    exit(1);
  }
  node_l->ts = (timestamp_t*)malloc(sizeof(timestamp_t) * depth);
  if (node_l->ts == NULL) {
    perror("malloc");
    exit(1);
  }
  for (i = 0; i < depth; ++i) {
    node_l->next[i] = (i == 0 ? next : NULL);
    node_l->ts[i] = (i == 0 ? ts : NULL_TIMESTAMP);
  }
  node_l->newest_next = next;
  INIT_LOCK(&node_l->lock);
  return node_l;
}

intset_l_t *set_new_l(uint32_t max_rq) {
  intset_l_t *set;
  node_l_t *min, *max;
  int depth;

  if ((set = (intset_l_t *)malloc(sizeof(intset_l_t))) == NULL) {
    perror("malloc");
    exit(1);
  }
  depth = max_rq + 1;
  max = new_node_l(VAL_MAX, NULL, NULL_TIMESTAMP, depth, 0);
  min = new_node_l(VAL_MIN, max, NULL_TIMESTAMP, depth, 0);
  set->head = min;
  set->rqt = rqtracker_new_l(max_rq);

  return set;
}

void node_reclaim_edge_l(node_l_t *node, timestamp_t *active,
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

void node_delete_l(node_l_t *node) {
  DESTROY_LOCK(&node->lock);
  free(node->next);
  free(node->ts);
  free(node);
}

// TODO(jacnel): Delete all versions, not just the newest.
void set_delete_l(intset_l_t *set) {
  node_l_t *node, *next;

  node = set->head;
  while (node != NULL) {
    next = node->next[node->newest];
    node_delete_l(node);
    node = next;
  }
  free(set);
}

int set_size_l(intset_l_t *set) {
  int size = 0;
  node_l_t *node;

  /* We have at least 2 elements */
  node = set->head->newest_next;
  while (node->newest_next != NULL) {
    size++;
    node = node->newest_next;
  }

  return size;
}

rqtracker_l_t *rqtracker_new_l(uint32_t max_rq) {
  rqtracker_l_t *rqt;
  int i;

  rqt = (rqtracker_l_t *)malloc(sizeof(rqtracker_l_t));
  rqt->max_rq = max_rq;
  rqt->ts = NULL_TIMESTAMP;
  rqt->active = (timestamp_t *)malloc(sizeof(timestamp_t) * max_rq);
  for (i = 0; i < max_rq; ++i) {
    rqt->active[i] = NULL_TIMESTAMP;
  }
  INIT_LOCK(&rqt->lock);
  return rqt;
}

timestamp_t *rqtracker_snapshot_active_l(rqtracker_l_t *rqt,
                                         uint32_t *num_active) {
  timestamp_t *s;
  timestamp_t temp;
  int curr_rq, end, i, j, k;

  end = rqt->max_rq;
  s = (timestamp_t *)malloc(sizeof(timestamp_t) * rqt->max_rq);
  LOCK(&rqt->lock);
  for (i = 0; i < rqt->max_rq; ++i) {
    curr_rq = rqt->active[i];
    if (curr_rq == NULL_TIMESTAMP) {
      s[--end] = NULL_TIMESTAMP;
    } else {
      for (j = 0; j < i && s[j] < curr_rq; ++j)
        ;
      k = j;
      if (k != i) {
        for (j = end - 1; j > k; --j) {
          s[j] = s[j - 1];
        }
      }
      s[k] = curr_rq;
    }
  }
  *num_active = end;
  UNLOCK(&rqt->lock);
  return s;
}

timestamp_t rqtracker_start_update_l(rqtracker_l_t *rqt) {
  LOCK(&rqt->lock);
  ++(rqt->ts);
  assert(rqt->ts < MAX_TIMESTAMP);
  return rqt->ts;
}

void rqtracker_end_update_l(rqtracker_l_t *rqt) { UNLOCK(&rqt->lock); }

timestamp_t rqtracker_start_rq_l(rqtracker_l_t *rqt, uint32_t rq_id) {
  timestamp_t ts;
  LOCK(&rqt->lock);
  ts = rqt->ts;
  rqt->active[rq_id] = ts;
  UNLOCK(&rqt->lock);
  return ts;
}

void rqtracker_end_rq_l(rqtracker_l_t *rqt, uint32_t rq_id) {
  LOCK(&rqt->lock);
  rqt->active[rq_id] = NULL_TIMESTAMP;
  UNLOCK(&rqt->lock);
}

void rqtracker_delete_l(rqtracker_l_t *rqt) {
  free(rqt->active);
  DESTROY_LOCK(&rqt->lock);
}