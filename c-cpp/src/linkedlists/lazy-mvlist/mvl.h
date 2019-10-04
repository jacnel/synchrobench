#ifndef LAZYMVLIST_MVL_H
#define LAZYMVLIST_MVL_H

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <atomic_ops.h>

#define XSTR(s) STR(s)
#define STR(s) #s

#define ATOMIC_CAS_MB_FBAR(a, e, v) \
  (AO_compare_and_swap_full((volatile AO_t *)(a), (AO_t)(e), (AO_t)(v)))

#define ATOMIC_CAS_MB_NOBAR(a, e, v) \
  (AO_compare_and_swap((volatile AO_t *)(a), (AO_t)(e), (AO_t)(v)))

static volatile AO_t stop;

#define TRANSACTIONAL d->unit_tx

typedef intptr_t val_t;
#define VAL_MIN INT_MIN
#define VAL_MAX INT_MAX

#ifdef MUTEX
typedef pthread_mutex_t ptlock_t;
#define INIT_LOCK(lock) pthread_mutex_init((pthread_mutex_t *)lock, NULL);
#define DESTROY_LOCK(lock) pthread_mutex_destroy((pthread_mutex_t *)lock)
#define LOCK(lock) pthread_mutex_lock((pthread_mutex_t *)lock)
#define UNLOCK(lock) pthread_mutex_unlock((pthread_mutex_t *)lock)
#else
typedef pthread_spinlock_t ptlock_t;
#define INIT_LOCK(lock) \
  pthread_spin_init((pthread_spinlock_t *)lock, PTHREAD_PROCESS_PRIVATE);
#define DESTROY_LOCK(lock) pthread_spin_destroy((pthread_spinlock_t *)lock)
#define LOCK(lock) pthread_spin_lock((pthread_spinlock_t *)lock)
#define UNLOCK(lock) pthread_spin_unlock((pthread_spinlock_t *)lock)
#endif

typedef uint64_t timestamp_t;
#define NULL_TIMESTAMP 0
#define MIN_TIMESTAMP 1
#define MAX_TIMESTAMP UINT64_MAX

/* Keeps a pointer to the newest next for better performance. */
typedef struct node_l {
  val_t val;
  struct node_l *newest_next;
  volatile ptlock_t lock;
  uint32_t depth;
  uint32_t newest;
  struct node_l **next;
  timestamp_t *ts;
#ifdef PAD
  char padding[48];
#endif
} node_l_t;

typedef struct arena_node {
  node_l_t **next_ptr;
  timestamp_t *ts_ptr;
  struct arena_node *next;
} arena_node_t;

typedef struct arena_l {
  uint32_t num_slots;
  uint32_t depth;
  uint32_t chunk;
  arena_node_t **head;
  arena_node_t **tail;
} arena_l_t;

typedef struct rqtracker_l {
  volatile timestamp_t update_ts;
  volatile timestamp_t active_ts;
  volatile timestamp_t *active;
  pthread_rwlock_t active_rwlock;
  uint32_t max_rq;
} rqtracker_l_t;

typedef struct intset_l {
  node_l_t *head;
  rqtracker_l_t *rqt;
  arena_l_t *arena;
} intset_l_t;

#endif