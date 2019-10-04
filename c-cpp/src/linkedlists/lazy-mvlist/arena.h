#ifndef MVLIST_ARENA_H
#define MVLIST_ARENA_H

#include "mvl.h"
#include "node.h"

arena_l_t *arena_new_l(uint32_t depth, uint32_t chunk, uint32_t num_slots);
void arena_init_node_l(arena_l_t *arena, node_l_t *node, node_l_t *next,
                       timestamp_t ts, uint32_t slot_id);
void arena_reclaim_node_l(arena_l_t *arena, node_l_t *node);
void arena_delete_l(arena_l_t *arena);

#endif