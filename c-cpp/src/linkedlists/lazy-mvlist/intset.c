#include "intset.h"
#include "arena.h"
#include "lazy.h"
#include "mvl.h"
#include "node.h"
#include "rqtracker.h"

intset_l_t *set_new_l(uint32_t max_rq, uint32_t chunk, uint32_t num_slots) {
  intset_l_t *set;
  node_l_t *min, *max;
  int depth;

  if ((set = (intset_l_t *)malloc(sizeof(intset_l_t))) == NULL) {
    perror("malloc");
    exit(1);
  }
  depth = max_rq + 2;
#ifdef NOPREALLOC
  set->arena = NULL;
#else
  set->arena = arena_new_l(depth, chunk, num_slots);
#endif
  max = new_node_l(VAL_MAX, depth);
  arena_init_node_l(set->arena, max, NULL, NULL_TIMESTAMP, 0);
  min = new_node_l(VAL_MIN, depth);
  arena_init_node_l(set->arena, min, max, NULL_TIMESTAMP, 0);
  set->head = min;
  set->rqt = rqtracker_new_l(max_rq);

  return set;
}

// TODO(jacnel): Delete all versions, not just the newest.
void set_delete_l(intset_l_t *set) {
  node_l_t *node, *next;

  node = set->head;
  while (node != NULL) {
    next = node->next[node->newest];
    node_delete_l(node);
    node = next;
  }
  if (set->arena != NULL) {
    arena_delete_l(set->arena);
  }
  free(set);
}

int set_size_l(intset_l_t *set) {
  int size = 0;
  node_l_t *node;

  /* We have at least 2 elements */
  node = set->head->newest_next;
  while (node->newest_next != NULL) {
    size++;
    node = node->newest_next;
  }

  return size;
}

long set_count_used_ptrs_l(intset_l_t *set) {
  long count;
  int i;
  node_l_t *node;
  
  count = 0;
  node = set->head->newest_next;
  while (node->newest_next != NULL) {
    for (i = 0; i < node->depth; ++i) {
      if (node->next[i] != NULL) {
        ++count;
      }
    }
    node = node->newest_next;
  }
  
  return count;
}

int set_contains_l(intset_l_t *set, val_t val) { return parse_find(set, val); }

int set_add_l(intset_l_t *set, val_t val, uint32_t id) {
  return parse_insert(set, val, id);
}

int set_remove_l(intset_l_t *set, val_t val) { return parse_delete(set, val); }

int set_rq_l(intset_l_t *set, val_t low, val_t high, uint32_t rq_id,
             val_t **results, uint32_t *num_results) {
  return parse_rq(set, low, high, rq_id, results, num_results);
}