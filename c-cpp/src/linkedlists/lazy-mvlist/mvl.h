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

#define DEFAULT_DURATION 10000
#define DEFAULT_INITIAL 256
#define DEFAULT_NB_THREADS 1
#define DEFAULT_MAX_RQ 8
#define DEFAULT_NB_RQ_THREADS 0
#define DEFAULT_RQ_RATE 20
#define DEFAULT_RANGE 0x7FFFFFFF
#define DEFAULT_RQ 0
#define DEFAULT_SEED 0
#define DEFAULT_UPDATE 20
#define DEFAULT_LOCKTYPE 2
#define DEFAULT_ALTERNATE 0
#define DEFAULT_EFFECTIVE 1
#define DEFAULT_CAPACITY 1000

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

#endif