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

#include "intset.h"

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
  node_l->ts = (uint32_t *)malloc(sizeof(uint32_t) * depth);
  if (node_l->ts == NULL) {
    perror("malloc");
    exit(1);
  }
  for (i = 0; i < depth; ++i) {
    node_l->next[i] = (i == 0 ? next : NULL);
    node_l->ts[i] = NULL_TIMESTAMP;
  }
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
  set->rqt = new_rqtracker_l(max_rq);

  return set;
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
    next = node->next[0];
    node_delete_l(node);
    node = next;
  }
  free(set);
}

int set_size_l(intset_l_t *set) {
  int size = 0;
  node_l_t *node;

  /* We have at least 2 elements */
  node = set->head->next[0];
  while (node->next[0] != NULL) {
    size++;
    node = node->next[0];
  }

  return size;
}

rqtracker_l_t *new_rqtracker_l(uint32_t max_rq) {
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

timestamp_t *snapshot_active_l(rqtracker_l_t *rqt) {
  timestamp_t *s;
  timestamp_t temp;
  int curr_rq, end, i, j, k;

  end = rqt->max_rq;
  s = (timestamp_t *)malloc(sizeof(timestamp_t) * rqt->max_rq);
  LOCK(&rqt->lock);
  s[0] = rqt->active[0];
  for (i = 1; i < end; ++i) {
    curr_rq = rqt->active[i];
    if (curr_rq == NULL_TIMESTAMP) {
      s[--end] = NULL_TIMESTAMP;
      --i; /* Redo the iteration. */
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
  return s;
}

void rqtracker_delete_l(rqtracker_l_t *rqt) {
  free(rqt->active);
  DESTROY_LOCK(&rqt->lock);
}