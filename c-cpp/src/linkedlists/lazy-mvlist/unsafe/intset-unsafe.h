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

#ifndef MVLIST_UNSAFE_INTSETUNSAFE_H
#define MVLIST_UNSAFE_INTSETUNSAFE_H

#include "unsafe.h"

intset_unsafe_l_t *set_new_unsafe_l();
void set_delete_unsafe_l(intset_unsafe_l_t *set);
int set_size_unsafe_l(intset_unsafe_l_t *set);
int set_contains_unsafe_l(intset_unsafe_l_t *set, val_t val);
int set_add_unsafe_l(intset_unsafe_l_t *set, val_t val);
int set_remove_unsafe_l(intset_unsafe_l_t *set, val_t val);
int set_rq_unsafe_l(intset_unsafe_l_t *set, val_t low, val_t high, val_t **results, uint32_t *num_results);

#endif