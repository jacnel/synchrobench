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

#ifndef MVLIST_UNSAFE_LAZYUNSAFE_H
#define MVLIST_UNSAFE_LAZYUNSAFE_H

#include "unsafe.h"

/* handling logical deletion flag */
inline int is_marked_ref_unsafe(long i);
inline long unset_mark_unsafe(long i);
inline long set_mark_unsafe(long i);
inline node_unsafe_l_t *get_unmarked_ref_unsafe(node_unsafe_l_t *n);
inline node_unsafe_l_t *get_marked_ref_unsafe(node_unsafe_l_t *n);

/* linked list accesses */
int parse_validate_unsafe(node_unsafe_l_t *pred, node_unsafe_l_t *curr);
int parse_find_unsafe(intset_unsafe_l_t *set, val_t val);
int parse_insert_unsafe(intset_unsafe_l_t *set, val_t val);
int parse_delete_unsafe(intset_unsafe_l_t *set, val_t val);
int parse_rq_unsafe(intset_unsafe_l_t *set, val_t low, val_t high,
                    val_t **results, uint32_t *num_results);

#endif