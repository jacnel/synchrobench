#ifndef LAZYMVLIST_INTSET_H
#define LAZYMVLIST_INTSET_H

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
#include "mvl.h"
#include "node.h"
#include "rqtracker.h"
#include "arena.h"

typedef struct intset_l {
  node_l_t *head;
  rqtracker_l_t *rqt;
  arena_l_t *arena;
} intset_l_t;

intset_l_t *set_new_l(uint32_t max_rq, uint32_t capacity, uint32_t num_slots);
void set_delete_l(intset_l_t *set);
int set_size_l(intset_l_t *set);

int set_contains_l(intset_l_t *set, val_t val);
int set_add_l(intset_l_t *set, val_t val, uint32_t id);
int set_remove_l(intset_l_t *set, val_t val);
int set_rq_l(intset_l_t *set, val_t low, val_t high, uint32_t rq_id,
             val_t **results, uint32_t *num_results);

#endif