/*
 * File:
 *   lazy.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Lazy linked list implementation of an integer set based on Heller et al. algorithm
 *   "A Lazy Concurrent List-Based Set Algorithm"
 *   S. Heller, M. Herlihy, V. Luchangco, M. Moir, W.N. Scherer III, N. Shavit
 *   p.3-16, OPODIS 2005
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

#include "lazy-unsafe.h"
#include "node-unsafe.h"

inline int is_marked_ref_unsafe(long i) {
	return (int) (i &= LONG_MIN+1);
}

inline long unset_mark_unsafe(long i) {
	i &= LONG_MAX-1;
	return i;
}

inline long set_mark_unsafe(long i) {
	i = unset_mark_unsafe(i);
	i += 1;
	return i;
}

inline node_unsafe_l_t *get_unmarked_ref_unsafe(node_unsafe_l_t *n) {
	return (node_unsafe_l_t *) unset_mark_unsafe((long) n);
}

inline node_unsafe_l_t *get_marked_ref_unsafe(node_unsafe_l_t *n) {
	return (node_unsafe_l_t *) set_mark_unsafe((long) n);
}

/*
 * Checking that both curr and pred are both unmarked and that pred's next pointer
 * points to curr to verify that the entries are adjacent and present in the list.
 */
inline int parse_validate_unsafe(node_unsafe_l_t *pred, node_unsafe_l_t *curr) {
	return (!is_marked_ref_unsafe((long) pred) && !is_marked_ref_unsafe((long) curr) && (pred->next == curr));
}

int parse_find_unsafe(intset_unsafe_l_t *set, val_t val) {
	node_unsafe_l_t *curr;
	curr = set->head;
	while (curr->val < val)
		curr = get_unmarked_ref_unsafe(curr->next);
	return ((curr->val == val) && !is_marked_ref_unsafe((long) curr));
}

int parse_insert_unsafe(intset_unsafe_l_t *set, val_t val) {
	node_unsafe_l_t *curr, *pred, *newnode;
	int result;
	
	pred = set->head;
	curr = get_unmarked_ref_unsafe(pred->next);
	while (curr->val < val) {
		pred = curr;
		curr = get_unmarked_ref_unsafe(curr->next);
	}
	LOCK(&pred->lock);
	LOCK(&curr->lock);
	result = (parse_validate_unsafe(pred, curr) && (curr->val != val));
	if (result) {
		newnode = new_node_unsafe_l(val, curr);
		pred->next = newnode;
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
int parse_delete_unsafe(intset_unsafe_l_t *set, val_t val) {
	node_unsafe_l_t *pred, *curr;
	int result;
	
	pred = set->head;
	curr = get_unmarked_ref_unsafe(pred->next);
	while (curr->val < val) {
		pred = curr;
		curr = get_unmarked_ref_unsafe(curr->next);
	}
	LOCK(&pred->lock);
	LOCK(&curr->lock);
	result = (parse_validate_unsafe(pred, curr) && (val == curr->val));
	if (result) {
		curr->next = get_marked_ref_unsafe(curr->next);
		pred->next = get_unmarked_ref_unsafe(curr->next);
	}
	UNLOCK(&curr->lock);
	UNLOCK(&pred->lock);
	return result;
}

int parse_rq_unsafe(intset_unsafe_l_t *set, val_t low, val_t high, val_t **results,
             uint32_t *num_results) {
  node_unsafe_l_t *curr;
  val_t *r, *temp;
  uint32_t i, limit;

  limit = 1000;
  i = 0;
  r = (val_t *)malloc(sizeof(val_t) * limit);
  curr = set->head;
  while (curr->val < low) curr = get_unmarked_ref_unsafe(curr->next);
#ifdef COUNT_RQ
  /* When counting RQs we assume that high - low is the number of desired
   * elements to return */
  while (curr->val != VAL_MAX && i < (high - low)) {
    r[i++] = curr->val;
    curr = get_unmarked_ref_unsafe(curr->next);
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
    curr = get_unmarked_ref_unsafe(curr->next);
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
  *results = r;
  *num_results = i;
  return (i > 0);
}