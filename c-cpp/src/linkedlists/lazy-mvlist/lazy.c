/*
 * File:
 *   lazy.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Lazy linked list implementation of an integer set based on Heller et al.
 * algorithm "A Lazy Concurrent List-Based Set Algorithm" S. Heller, M. Herlihy,
 * V. Luchangco, M. Moir, W.N. Scherer III, N. Shavit p.3-16, OPODIS 2005
 *
 * Copyright (c) 2009-2010.
 *
 * lazy.c is part of Synchrobench
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

#include "lazy.h"
#include "node.h"
#include "rqtracker.h"

inline int is_marked_ref(long i) { return (int)(i &= LONG_MIN + 1); }

inline long unset_mark(long i) {
  i &= LONG_MAX - 1;
  return i;
}

inline long set_mark(long i) {
  i = unset_mark(i);
  i += 1;
  return i;
}

inline node_l_t *get_unmarked_ref(node_l_t *n) {
  return (node_l_t *)unset_mark((long)n);
}

inline node_l_t *get_marked_ref(node_l_t *n) {
  return (node_l_t *)set_mark((long)n);
}

/*
 * Checking that both curr and pred are both unmarked and that pred's next
 * pointer points to curr to verify that the entries are adjacent and present in
 * the list.
 */
inline int parse_validate(node_l_t *pred, node_l_t *curr) {
  return (!is_marked_ref((long)pred) && !is_marked_ref((long)curr) &&
          (pred->newest_next == curr));
}

int parse_find(intset_l_t *set, val_t val) {
  node_l_t *curr;
  curr = set->head;
  while (curr->val < val) curr = get_unmarked_ref(curr->newest_next);
  return ((curr->val == val) && !is_marked_ref((long)curr));
}

int parse_insert(intset_l_t *set, val_t val, uint32_t tid) {
  node_l_t *curr, *pred, *newnode;
  timestamp_t ts, *s;
  int r, result;
  uint32_t num_active, newest;

  pred = set->head;
  curr = get_unmarked_ref(pred->newest_next);
  while (curr->val < val) {
    pred = curr;
    curr = get_unmarked_ref(curr->newest_next);
  }
  LOCK(&pred->lock);
  LOCK(&curr->lock);
  result = (parse_validate(pred, curr) && (curr->val != val));
  if (result) {
    s = rqtracker_snapshot_active_l(set->rqt, &num_active);
    node_retire_edge_l(pred, s, num_active);
    newnode = new_node_l(val, set->rqt->max_rq + 2);
    arena_init_node_l(set->arena, newnode, curr, NULL_TIMESTAMP, tid);
    newest = (pred->newest + 1) % pred->depth;
    ts = rqtracker_start_update_l(set->rqt);
    newnode->ts[newnode->newest] = ts;
    pred->newest_next = newnode;
    pred->next[newest] = newnode;
    pred->ts[newest] = ts;
    pred->newest = newest;
    rqtracker_end_update_l(set->rqt);
  }
  UNLOCK(&curr->lock);
  UNLOCK(&pred->lock);
  return result;
}

/*
 * Logically remove an element by setting a mark bit to 1
 * before removing it physically.
 *
 * NB. it is not safe to free the element after physical deletion as a
 * pre-empted find operation may currently be parsing the element.
 * TODO: must implement a stop-the-world garbage collector to correctly
 * free the memory.
 */
int parse_delete(intset_l_t *set, val_t val) {
  node_l_t *pred, *curr;
  timestamp_t ts, *s;
  int result;
  uint32_t num_active, curr_newest, pred_newest;

  pred = set->head;
  curr = get_unmarked_ref(pred->newest_next);
  while (curr->val < val) {
    pred = curr;
    curr = get_unmarked_ref(curr->newest_next);
  }
  LOCK(&pred->lock);
  LOCK(&curr->lock);
  result = (parse_validate(pred, curr) && (val == curr->val));
  if (result) {
    s = rqtracker_snapshot_active_l(set->rqt, &num_active);
    node_retire_edge_l(pred, s, num_active);
    curr_newest = curr->newest;
    pred_newest = (pred->newest + 1) % pred->depth;
    ts = rqtracker_start_update_l(set->rqt);
    curr->newest_next = get_marked_ref(curr->next[curr_newest]);
    curr->next[curr_newest] = curr->newest_next;
    pred->newest_next = get_unmarked_ref(curr->next[curr->newest]);
    pred->next[pred_newest] = pred->newest_next;
    pred->ts[pred_newest] = ts;
    pred->newest = pred_newest;
    rqtracker_end_update_l(set->rqt);
  }
  UNLOCK(&curr->lock);
  UNLOCK(&pred->lock);
  return result;
}

int parse_rq(intset_l_t *set, val_t low, val_t high, uint32_t rq_id,
             val_t **results, uint32_t *num_results) {
  node_l_t *curr;
  timestamp_t ts;
  val_t *r, *temp;
  uint32_t i, limit;

  limit = 1000;
  i = 0;
  r = (val_t *)malloc(sizeof(val_t) * limit);
  curr = set->head;
  ts = rqtracker_start_rq_l(set->rqt, rq_id);
  while (curr->val < low)
    curr = get_unmarked_ref(node_next_from_timestamp_l(curr, ts));
#ifdef COUNT_RQ
  /* When counting RQs we assume that high - low is the number of desired
   * elements to return */
  while (curr->val != VAL_MAX && i < (high - low)) {
    r[i++] = curr->val;
    curr = get_unmarked_ref(node_next_from_timestamp_l(curr, ts));
    if (i == limit) {
      temp = (val_t *)malloc(sizeof(val_t) * limit * 2);
      for (i = 0; i < limit; ++i) {
        temp[i] = r[i];
      }
      free(r);
      limit = limit * 2;
      r = temp;
    }
  }
#else
  while (curr->val <= high && i < limit) {
    r[i++] = curr->val;
    curr = get_unmarked_ref(node_next_from_timestamp_l(curr, ts));
    if (i == limit) {
      temp = (val_t *)malloc(sizeof(val_t) * limit * 2);
      for (i = 0; i < limit; ++i) {
        temp[i] = r[i];
      }
      free(r);
      limit = limit * 2;
      r = temp;
    }
  }
#endif
  rqtracker_end_rq_l(set->rqt, rq_id);
  *results = r;
  *num_results = i;
  return (i > 0);
}
