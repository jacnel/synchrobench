#include "arena.h"

typedef struct arena_init_args {
  uint32_t depth;
  uint32_t chunk;
  arena_node_t **head;
  arena_node_t **tail;
} arena_init_args_t;

void *arena_init_l(void *args) {
  arena_node_t *nodes;
  timestamp_t *ts;
  node_l_t **next;
  arena_init_args_t *init_args;
  int i, num_nodes;

  init_args = (arena_init_args_t *)args;

  num_nodes = (init_args->chunk / init_args->depth);
  nodes = (arena_node_t *)malloc(sizeof(arena_node_t) * num_nodes);
  if (nodes == NULL) {
    perror("malloc");
    exit(1);
  }

  ts = (timestamp_t *)malloc(sizeof(timestamp_t) * init_args->depth * num_nodes);
  if (ts == NULL) {
    perror("malloc");
    exit(1);
  }

  next = (node_l_t**)malloc(sizeof(node_l_t*) * init_args->depth * num_nodes);
  if (next == NULL) {
    perror("malloc");
    exit(1);
  }

  for (i = 0; i < init_args->depth * num_nodes; ++i) {
    ts[i] = NULL_TIMESTAMP;
    next[i] = NULL;
  }

  for (i = 0; i < num_nodes; ++i) {
    nodes[i].next = (i < num_nodes - 1 ? &nodes[i + 1] : NULL);
    nodes[i].next_ptr = &next[i * init_args->depth];
    nodes[i].ts_ptr = &ts[i * init_args->depth];
  }

  *init_args->head = &nodes[0];
  *init_args->tail = &nodes[num_nodes - 1];
}

arena_l_t *arena_new_l(uint32_t depth, uint32_t chunk, uint32_t num_slots) {
  arena_l_t *arena;
  pthread_t *threads;
  arena_init_args_t *init_args;
  pthread_attr_t attr;
  int i, j;

  arena = (arena_l_t *)malloc(sizeof(arena_l_t));
  if (arena == NULL) {
    perror("malloc");
    exit(1);
  }

  arena->num_slots = num_slots;
  arena->depth = depth;
  arena->chunk = chunk;
  arena->head = (arena_node_t**)malloc(sizeof(arena_node_t*) * num_slots);
  if (arena->head == NULL) {
    perror("malloc");
    exit(1);
  }
  arena->tail = (arena_node_t**)malloc(sizeof(arena_node_t*) * num_slots);
  if (arena->tail == NULL) {
    perror("malloc");
    exit(1);
  }

  threads = (pthread_t *)malloc(sizeof(pthread_t) * num_slots);
  if (threads == NULL) {
    perror("malloc");
    exit(1);
  }

  init_args =
      (arena_init_args_t *)malloc(sizeof(arena_init_args_t) * num_slots);
  if (init_args == NULL) {
    perror("malloc");
    exit(1);
  }

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  for (i = 0; i < num_slots; ++i) {
    init_args[i].depth = depth;
    init_args[i].chunk = chunk;
    init_args[i].head = &arena->head[i];
    init_args[i].tail = &arena->tail[i];
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
  /* If there is no arena, then allocate on demand. */
  arena_node_t *curr;
  arena_init_args_t init_args;
  int i;
  if (arena == NULL) {
    node->newest_next = next;
    node->next = (node_l_t **)malloc(sizeof(node_l_t *) * node->depth);
    node->ts = (timestamp_t *)malloc(sizeof(timestamp_t) * node->depth);
    for (i = 0; i < node->depth; ++i) {
      node->next[i] = (i == node->newest ? next : NULL);
      node->ts[i] = (i == node->newest ? ts : NULL_TIMESTAMP);
    }
  } else {
    assert(slot_id < arena->num_slots);
    if (arena->head[slot_id] == arena->tail[slot_id]) {
      printf("WARNING: Pointer bank capacity reached.\n");
      init_args.chunk = arena->chunk;
      init_args.depth = arena->depth;
      init_args.head = &arena->head[slot_id];
      init_args.tail = &arena->tail[slot_id];
      arena_init_l((void*)&init_args);
    }
    curr = arena->head[slot_id];
    node->newest_next = next;
    node->next = curr->next_ptr;
    node->next[node->newest] = next;
    node->ts = curr->ts_ptr;
    node->ts[node->newest] = ts;
    arena->head[slot_id] = curr->next;
  }
}

void arena_reclaim_node_l(arena_l_t *arena, node_l_t *node) { return; }

void arena_delete_l(arena_l_t *arena) {
return;
}