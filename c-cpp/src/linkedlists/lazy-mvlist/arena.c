#include "arena.h"

typedef struct arena_init_args {
  uint32_t capacity;
  uint32_t *cap_ptr;
  uint32_t *curr_ptr;
  node_l_t ***next_ptr;
  timestamp_t **ts_ptr;
} arena_init_args_t;

void *arena_init_l(void *args) {
  arena_init_args_t *init_args;
  int i;

  init_args = (arena_init_args_t *)args;

  *init_args->cap_ptr = init_args->capacity;
  *init_args->curr_ptr = 0;
  *init_args->next_ptr =
      (node_l_t **)malloc(sizeof(node_l_t *) * init_args->capacity);
  *init_args->ts_ptr =
      (timestamp_t *)malloc(sizeof(timestamp_t) * init_args->capacity);
  for (i = 0; i < init_args->capacity; ++i) {
    (*init_args->next_ptr)[i] = NULL;
    (*init_args->ts_ptr)[i] = NULL_TIMESTAMP;
  }
}

arena_l_t *arena_new_l(uint32_t capacity, uint32_t num_slots) {
  arena_l_t *arena;
  pthread_t *threads;
  arena_init_args_t *init_args;
  pthread_attr_t attr;
  int i, j;

  // TODO(jacnel): Check mallocs are successful.
  arena = (arena_l_t *)malloc(sizeof(arena_l_t));
  arena->num_slots = num_slots;
  arena->capacity = (uint32_t *)malloc(sizeof(uint32_t) * num_slots);
  arena->curr = (uint32_t *)malloc(sizeof(uint32_t) * num_slots);
  arena->next = (node_l_t ***)malloc(sizeof(node_l_t **) * num_slots);
  arena->ts = (timestamp_t **)malloc(sizeof(timestamp_t *) * num_slots);

  threads = (pthread_t *)malloc(sizeof(pthread_t) * num_slots);
  init_args =
      (arena_init_args_t *)malloc(sizeof(arena_init_args_t) * num_slots);

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  for (i = 0; i < num_slots; ++i) {
    init_args[i].capacity = capacity;
    init_args[i].cap_ptr = &arena->capacity[i];
    init_args[i].curr_ptr = &arena->curr[i];
    init_args[i].next_ptr = &arena->next[i];
    init_args[i].ts_ptr = &arena->ts[i];
    if (pthread_create(&threads[i], &attr, arena_init_l,
                       (void *)(&init_args[i])) != 0) {
      fprintf(stderr, "Error creating thread to initialize arena.\n");
      exit(1);
    }
  }
  pthread_attr_destroy(&attr);

  for (i = 0; i < num_slots; ++i) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(
          stderr,
          "Error waiting for thread completion while initializing arena.\n");
      exit(1);
    }
  }
  return arena;
}

void arena_init_node_l(arena_l_t *arena, node_l_t *node, node_l_t *next,
                       timestamp_t ts, uint32_t slot_id) {
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

void arena_reclaim_node_l(arena_l_t *arena, node_l_t *node) { return; }

void arena_delete_l(arena_l_t *arena) {
  free(arena->curr);
  free(arena->capacity);
  free(arena->next);
  free(arena->ts);
  free(arena);
}