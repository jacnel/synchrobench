#include "arena.h"

arena_l_t *arena_new_l(uint32_t capacity, uint32_t num_slots) {
  arena_l_t *arena;
  int i, j;

  // TODO(jacnel): Check mallocs are successful.
  arena = (arena_l_t *)malloc(sizeof(arena_l_t));
  arena->num_slots = num_slots;
  arena->capacity = (uint32_t *)malloc(sizeof(uint32_t) * num_slots);
  arena->curr = (uint32_t *)malloc(sizeof(uint32_t) * num_slots);
  arena->next = (node_l_t ***)malloc(sizeof(node_l_t **) * num_slots);
  arena->ts = (timestamp_t **)malloc(sizeof(timestamp_t *) * num_slots);
  for (i = 0; i < num_slots; ++i) {
    arena->capacity[i] = capacity;
    arena->curr[i] = 0;
    arena->next[i] = (node_l_t **)malloc(sizeof(node_l_t *) * capacity);
    arena->ts[i] = (timestamp_t *)malloc(sizeof(timestamp_t) * capacity);
    for (j = 0; j < capacity; ++j) {
      arena->next[i][j] = NULL;
      arena->ts[i][j] = NULL_TIMESTAMP;
    }
  }
  return arena;
}

void arena_init_node_l(arena_l_t *arena, node_l_t *node,
                          node_l_t *next, timestamp_t ts, uint32_t slot_id) {
  assert(slot_id < arena->num_slots);
  assert(arena->curr[slot_id] + node->depth < arena->capacity[slot_id]);
  node->next = &arena->next[slot_id][arena->curr[slot_id]];
  node->next[node->newest] = next;
  node->newest_next = next;
  node->ts = &arena->ts[slot_id][arena->curr[slot_id]];
  node->ts[node->newest] = ts;
  arena->curr[slot_id] += node->depth;
  if (arena->curr[slot_id] >= arena->capacity[slot_id]) {
    // TODO(jacnel): Increase capacity for the current slot.
    printf("WARNING: Pointer bank capacity reached.\n");
  }
}

void arena_reclaim_node_l(arena_l_t *arena, node_l_t *node) {
  return;
}

void arena_delete_l(arena_l_t * arena) {
  free(arena->curr);
  free(arena->capacity);
  free(arena->next);
  free(arena->ts);
  free(arena);
}