#ifndef LAZYMVLIST_INTSET_H
#define LAZYMVLIST_INTSET_H

#include "mvl.h"

intset_l_t *set_new_l(uint32_t max_rq, uint32_t capacity, uint32_t num_slots);
void set_delete_l(intset_l_t *set);
int set_size_l(intset_l_t *set);
long set_count_used_ptrs_l(intset_l_t *set);

int set_contains_l(intset_l_t *set, val_t val);
int set_add_l(intset_l_t *set, val_t val, uint32_t id);
int set_remove_l(intset_l_t *set, val_t val);
int set_rq_l(intset_l_t *set, val_t low, val_t high, uint32_t rq_id,
             val_t **results, uint32_t *num_results);

#endif