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

#include "intset.h"
#include "coupling.h"
#include "lazy.h"

int set_contains_l(intset_l_t *set, val_t val, int transactional) {
  if (transactional == 2)
    return parse_find(set, val);
  else
    return lockc_find(set, val);
}

int set_add_l(intset_l_t *set, val_t val, int transactional) {
  if (transactional == 2)
    return parse_insert(set, val);
  else
    return lockc_insert(set, val);
}

int set_remove_l(intset_l_t *set, val_t val, int transactional) {
  if (transactional == 2)
    return parse_delete(set, val);
  else
    return lockc_delete(set, val);
}

int set_rq_l(intset_l_t *set, val_t low, val_t high, uint32_t rq_id,
             val_t **results, uint32_t *num_results, int transactional) {
  if (transactional == 2)
    return parse_rq(set, low, high, rq_id, results, num_results);
  else
    return lockc_rq(set, low, high, rq_id, results, num_results);
}