/*
 * File:
 *   intset.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Linked list integer set operations
 *
 * Copyright (c) 2009-2010.
 *
 * intset.c is part of Synchrobench
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

#include "intset-unsafe.h"
#include "lazy-unsafe.h"
#include "node-unsafe.h"

intset_unsafe_l_t *set_new_unsafe_l() {
  intset_unsafe_l_t *set;
  node_unsafe_l_t *min, *max;

  if ((set = (intset_unsafe_l_t *)malloc(sizeof(intset_unsafe_l_t))) == NULL) {
    perror("malloc");
    exit(1);
  }
  max = new_node_unsafe_l(VAL_MAX, NULL);
  min = new_node_unsafe_l(VAL_MIN, max);
  set->head = min;

  return set;
}

void set_delete_unsafe_l(intset_unsafe_l_t *set) {
  node_unsafe_l_t *node, *next;

  node = set->head;
  while (node != NULL) {
    next = node->next;
    DESTROY_LOCK(&node->lock);
    free(node);
    node = next;
  }
  free(set);
}

int set_size_unsafe_l(intset_unsafe_l_t *set) {
  int size = 0;
  node_unsafe_l_t *node;

  /* We have at least 2 elements */
  node = set->head->next;
  while (node->next != NULL) {
    size++;
    node = node->next;
  }

  return size;
}

int set_contains_unsafe_l(intset_unsafe_l_t *set, val_t val) {
  return parse_find_unsafe(set, val);
}

int set_add_unsafe_l(intset_unsafe_l_t *set, val_t val) {
  return parse_insert_unsafe(set, val);
}

int set_remove_unsafe_l(intset_unsafe_l_t *set, val_t val) {
  return parse_delete_unsafe(set, val);
}
int set_rq_unsafe_l(intset_unsafe_l_t *set, val_t low, val_t high, val_t **results,
             uint32_t *num_results) {
  return parse_rq_unsafe(set, low, high, results, num_results);
}