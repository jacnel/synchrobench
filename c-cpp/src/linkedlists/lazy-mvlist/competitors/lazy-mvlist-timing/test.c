/*
 * File:
 *   test.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Concurrent accesses to the linked list integer set
 *
 * Copyright (c) 2009-2010.
 *
 * test.c is part of Synchrobench
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

#include "intset.h"

#define UPDATE 0
#define RANGE_QUERY 1
#define CONTAINS 2

typedef struct barrier {
  pthread_cond_t complete;
  pthread_mutex_t mutex;
  int count;
  int crossing;
} barrier_t;

void barrier_init(barrier_t *b, int n) {
  pthread_cond_init(&b->complete, NULL);
  pthread_mutex_init(&b->mutex, NULL);
  b->count = n;
  b->crossing = 0;
}

void barrier_cross(barrier_t *b) {
  pthread_mutex_lock(&b->mutex);
  /* One more thread through */
  b->crossing++;
  /* If not all here, wait */
  if (b->crossing < b->count) {
    pthread_cond_wait(&b->complete, &b->mutex);
  } else {
    pthread_cond_broadcast(&b->complete);
    /* Reset for next time */
    b->crossing = 0;
  }
  pthread_mutex_unlock(&b->mutex);
}

/*
 * Returns a pseudo-random value in [1; range].
 * Depending on the symbolic constant RAND_MAX>=32767 defined in stdlib.h,
 * the granularity of rand() could be lower-bounded by the 32767^th which might
 * be too high for given program options [r]ange and [i]nitial.
 *
 * Note: this is not thread-safe and will introduce futex locks
 */
inline long rand_range(long r) {
  int m = RAND_MAX;
  int d, v = 0;

  do {
    d = (m > r ? r : m);
    v += 1 + (int)(d * ((double)rand() / ((double)(m) + 1.0)));
    r -= m;
  } while (r > 0);
  return v;
}
long rand_range(long r);

/* Thread-safe, re-entrant version of rand_range(r) */
inline long rand_range_re(unsigned int *seed, long r) {
  int m = RAND_MAX;
  int d, v = 0;

  do {
    d = (m > r ? r : m);
    v += 1 + (int)(d * ((double)rand_r(seed) / ((double)(m) + 1.0)));
    r -= m;
  } while (r > 0);
  return v;
}
long rand_range_re(unsigned int *seed, long r);

typedef struct thread_data {
  int tid;
  val_t first;
  long range;
  int update;
  int rq_rate;
  int unit_tx;
  int alternate;
  int effective;
  unsigned long nb_add;
  unsigned long nb_added;
  unsigned long nb_remove;
  unsigned long nb_removed;
  unsigned long nb_contains;
  unsigned long nb_found;
  unsigned long nb_rqs;
  unsigned long nb_successful_rqs;
  unsigned long nb_aborts;
  unsigned long nb_aborts_locked_read;
  unsigned long nb_aborts_locked_write;
  unsigned long nb_aborts_validate_read;
  unsigned long nb_aborts_validate_write;
  unsigned long nb_aborts_validate_commit;
  unsigned long nb_aborts_invalid_memory;
  unsigned long max_retries;
  unsigned int seed;
  intset_l_t *set;
  barrier_t *barrier;
  timeing_l_t timing;
} thread_data_t;

int get_rand_op(unsigned int *seed, int update, int rq_rate, int tid) {
  int r, op;
  r = (rand_range_re(seed, 100) - 1);
  if (tid < 0) {
    if (r < update) {
      op = UPDATE;
    } else {
      op = CONTAINS;
    }
  } else {
    if (r < rq_rate) {
      op = RANGE_QUERY;
    } else if (r < (update / (double)100) * (100 - (rq_rate))) {
      op = UPDATE;
    } else {
      op = CONTAINS;
    }
  }
  return op;
}

void *test(void *data) {
  int op, num_results, last = -1;
  val_t low, high, temp, *results, val = 0;
  clock_t start, end;
  uint32_t duration;

  thread_data_t *d = (thread_data_t *)data;

  /* Wait on barrier */
  barrier_cross(d->barrier);

  /* Is the first op an update? */
  op = get_rand_op(&d->seed, d->update, d->rq_rate, d->tid);

  while (stop == 0) {
    /* Postive tid threads perform 100% range queries */
    if (op == UPDATE) {  // update
      if (last < 0) {    // add
        val = rand_range_re(&d->seed, d->range);
        start = clock();
        if (set_add_l(d->set, val, TRANSACTIONAL, &d->timing.insert_crit)) {
          d->nb_added++;
          last = val;
        }
        end = clock();
        d->nb_add++;
      } else {               // remove
        if (d->alternate) {  // alternate mode
          start = clock();
          if (set_remove_l(d->set, last, TRANSACTIONAL,
                           &d->timing.remove_crit)) {
            d->nb_removed++;
          }
          end = clock();
          last = -1;
        } else {
          val = rand_range_re(&d->seed, d->range);
          start = clock();
          if (set_remove_l(d->set, val, TRANSACTIONAL,
                           &d->timing.remove_crit)) {
            d->nb_removed++;
            last = -1;
          }
          end = clock();
        }
        d->nb_remove++;
      }
    } else if (op == RANGE_QUERY) {
      low = rand_range_re(&d->seed, d->range);
      high = rand_range_re(&d->seed, d->range);
      if (high < low) {
        temp = high;
        high = low;
        low = temp;
      }
      start = clock();
      if (set_rq_l(d->set, low, high, d->tid, &results, &num_results,
                   TRANSACTIONAL)) {
        d->nb_successful_rqs++;
      }
      end = clock();
      d->nb_rqs++;
    } else {  // read
      if (d->alternate) {
        if (d->update == 0) {
          if (last < 0) {
            val = d->first;
            last = val;
          } else {  // last >= 0
            val = rand_range_re(&d->seed, d->range);
            last = -1;
          }
        } else {  // update != 0
          if (last < 0) {
            val = rand_range_re(&d->seed, d->range);
            // last = val;
          } else {
            val = last;
          }
        }
      } else
        val = rand_range_re(&d->seed, d->range);

      start = clock();
      if (set_contains_l(d->set, val, TRANSACTIONAL)) d->nb_found++;
      d->nb_contains++;
      end = clock();
    }

    duration = (end - start);
    if (op == CONTAINS) {
      d->timing.read_dur += duration;
    } else if (op == RANGE_QUERY) {
      d->timing.rq_dur += duration;
    } else if (op == UPDATE && last < 0) {
      d->timing.remove_dur += duration;
    } else {
      d->timing.insert_dur += duration;
    }
    d->timing.total_dur += duration;

    /* Is the next op an update? */
    if (d->effective && d->tid < 0) {  // a failed remove/add is a read-only tx
      if ((100 * (d->nb_added + d->nb_removed)) <
          (d->update * (d->nb_add + d->nb_remove + d->nb_contains))) {
        op = UPDATE;
      } else {
        op = CONTAINS;
      }
    } else if (d->effective) {
      if ((100 * (d->nb_rqs)) <= (d->rq_rate * (d->nb_add + d->nb_remove +
                                                d->nb_contains + d->nb_rqs))) {
        op = RANGE_QUERY;
      } else if ((100 * (d->nb_added + d->nb_removed)) <
                 (d->update * (d->nb_add + d->nb_remove + d->nb_contains))) {
        op = UPDATE;
      } else {
        op = CONTAINS;
      }
    } else {  // remove/add (even failed) is considered an update
      op = get_rand_op(&d->seed, d->update, d->rq_rate, d->tid);
    }
  }
  return NULL;
}

int main(int argc, char **argv) {
  struct option long_options[] = {
      // These options don't set a flag
      {"help", no_argument, NULL, 'h'},
      {"duration", required_argument, NULL, 'd'},
      {"initial-size", required_argument, NULL, 'i'},
      {"thread-num", required_argument, NULL, 't'},
      {"range", required_argument, NULL, 'r'},
      {"max-rqs", required_argument, NULL, 'm'},
      {"rq-threads", required_argument, NULL, 'q'},
      {"rq-rate", required_argument, NULL, 'R'},
      {"seed", required_argument, NULL, 'S'},
      {"update-rate", required_argument, NULL, 'u'},
      {"unit-tx", required_argument, NULL, 'x'},
      {NULL, 0, NULL, 0}};

  intset_l_t *set;
  int i, c, size;
  val_t last = 0;
  val_t val = 0;
  unsigned long reads, effreads, rqs, effrqs, updates, effupds, inserts,
      effinserts, removes, effremoves, aborts, aborts_locked_read,
      aborts_locked_write, aborts_validate_read, aborts_validate_write,
      aborts_validate_commit, aborts_invalid_memory, max_retries;
  thread_data_t *data;
  pthread_t *threads;
  pthread_attr_t attr;
  barrier_t barrier;
  struct timeval start, end;
  struct timespec timeout;
  int duration = DEFAULT_DURATION;
  int initial = DEFAULT_INITIAL;
  int nb_threads = DEFAULT_NB_THREADS;
  int max_rq_threads = DEFAULT_MAX_RQ;
  int nb_rq_threads = DEFAULT_NB_RQ_THREADS;
  int rq_rate = DEFAULT_RQ_RATE;
  long range = DEFAULT_RANGE;
  int seed = DEFAULT_SEED;
  int update = DEFAULT_UPDATE;
  int unit_tx = DEFAULT_LOCKTYPE;
  int alternate = DEFAULT_ALTERNATE;
  int effective = DEFAULT_EFFECTIVE;
  sigset_t block_set;
  uint64_t read_dur, insert_dur, insert_crit, remove_dur, remove_crit, rq_dur,
      total_dur;

  while (1) {
    i = 0;
    c = getopt_long(argc, argv, "hAf:d:i:t:r:S:u:x:R:q:m:R:", long_options,
                    &i);

    if (c == -1) break;

    if (c == 0 && long_options[i].flag == 0) c = long_options[i].val;

    switch (c) {
      case 0:
        /* Flag is automatically set */
        break;
      case 'h':
        printf("intset -- STM stress test "
	     "(linked list)\n"
	     "\n"
	     "Usage:\n"
	     "  intset [options...]\n"
	     "\n"
	     "Options:\n"
	     "  -h, --help\n"
	     "        Print this message\n"
	     "  -A, --alternate (default="XSTR(DEFAULT_ALTERNATE)")\n"
	     "        Consecutive insert/remove target the same value\n"
	     "  -f, --effective <int>\n"
	     "        update txs must effectively write (0=trial, 1=effective, default=" XSTR(DEFAULT_EFFECTIVE) ")\n"
	     "  -d, --duration <int>\n"
	     "        Test duration in milliseconds (0=infinite, default=" XSTR(DEFAULT_DURATION) ")\n"
	     "  -i, --initial-size <int>\n"
	     "        Number of elements to insert before test (default=" XSTR(DEFAULT_INITIAL) ")\n"
	     "  -m, --max-rqs <int>\n"
	     "        Maximum number of threads performing range queries (default=" XSTR(DEFAULT_MAX_RQ) ")\n"
	     "  -q, --rq-threads <int>\n"
	     "        Number of threads running range queries (default=" XSTR(DEFAULT_NB_RQ_THREADS) ")\n"
	     "  -t, --thread-num <int>\n"
	     "        Number of threads (default=" XSTR(DEFAULT_NB_THREADS) ")\n"
	     "  -R, --rq-rate <int>\n"
	     "        Percentage of operations that rq threads are performing range queries (default=" XSTR(DEFAULT_RQ_RATE) ")\n"
	     "  -r, --range <int>\n"
	     "        Range of integer values inserted in set (default=" XSTR(DEFAULT_RANGE) ")\n"
	     "  -S, --seed <int>\n"
	     "        RNG seed (0=time-based, default=" XSTR(DEFAULT_SEED) ")\n"
	     "  -u, --update-rate <int>\n"
	     "        Percentage of update transactions (default=" XSTR(DEFAULT_UPDATE) ")\n"
	     "  -x, --lock-based algorithm (default=1)\n"
	     "        Use lock-based algorithm\n"
	     "        1 = lock-coupling,\n"
	     "        2 = lazy algorithm\n"
	     );
        exit(0);
      case 'A':
        alternate = 1;
        break;
      case 'f':
        effective = atoi(optarg);
        break;
      case 'd':
        duration = atoi(optarg);
        break;
      case 'i':
        initial = atoi(optarg);
        break;
      case 'm':
        max_rq_threads = atoi(optarg);
        break;
      case 't':
        nb_threads = atoi(optarg);
        break;
      case 'q':
        nb_rq_threads = atoi(optarg);
        break;
      case 'R':
        rq_rate = atoi(optarg);
        break;
      case 'r':
        range = atol(optarg);
        break;
      case 'S':
        seed = atoi(optarg);
        break;
      case 'u':
        update = atoi(optarg);
        break;
      case 'x':
        printf("The parameter x is not valid for this benchmark.\n");
        exit(0);
      case 'a':
        printf("The parameter a is not valid for this benchmark.\n");
        exit(0);
      case 's':
        printf("The parameter s is not valid for this benchmark.\n");
        exit(0);
      case '?':
        printf("Use -h or --help for help.\n");
        exit(0);
      default:
        exit(1);
    }
  }

  assert(duration >= 0);
  assert(initial >= 0);
  assert(nb_threads > 0);
  assert(range > 0 && range >= initial);
  assert(update >= 0 && update <= 100);
  assert(rq_rate >= 0 && rq_rate <= 100);
  assert(nb_rq_threads >= 0);
  assert(max_rq_threads > 0);
  assert(nb_rq_threads <= max_rq_threads);

  printf("Set type     : lazy linked list\n");
  printf("Length       : %d\n", duration);
  printf("Initial size : %d\n", initial);
  printf("Thread num   : %d\n", nb_threads);
  printf("RQ threads   : %d\n", nb_rq_threads);
  printf("Max RQs      : %d\n", max_rq_threads);
  printf("RQ rate      : %d\n", rq_rate);
  printf("Size of node : %lu\n", sizeof(node_l_t));
  printf("Value range  : %ld\n", range);
  printf("Seed         : %d\n", seed);
  printf("Update rate  : %d\n", update);
  printf("Lock alg     : %d\n", unit_tx);
  printf("Alternate    : %d\n", alternate);
  printf("Effective    : %d\n", effective);
  printf("Type sizes   : int=%d/long=%d/ptr=%d/word=%d\n", (int)sizeof(int),
         (int)sizeof(long), (int)sizeof(void *), (int)sizeof(uintptr_t));

  timeout.tv_sec = duration / 1000;
  timeout.tv_nsec = (duration % 1000) * 1000000;

  if ((data = (thread_data_t *)malloc(nb_threads * sizeof(thread_data_t))) ==
      NULL) {
    perror("malloc");
    exit(1);
  }
  if ((threads = (pthread_t *)malloc(nb_threads * sizeof(pthread_t))) == NULL) {
    perror("malloc");
    exit(1);
  }

  if (seed == 0)
    srand((int)time(0));
  else
    srand(seed);

  set = set_new_l(max_rq_threads);

  stop = 0;

  /* Init STM */
  printf("Initializing STM\n");

  /* Populate set */
  printf("Adding %d entries to set\n", initial);
  i = 0;
  while (i < initial) {
    val = (rand() % range) + 1;
    if (set_add_l(set, val, unit_tx, &insert_crit)) {
      last = val;
      i++;
    }
  }
  size = set_size_l(set);
  printf("Set size     : %d\n", size);

  /* Access set from all threads */
  barrier_init(&barrier, nb_threads + 1);
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  for (i = 0; i < nb_threads; i++) {
    printf("Creating thread %d\n", i);
    data[i].tid = (i < nb_rq_threads ? i : -1);
    data[i].first = last;
    data[i].range = range;
    data[i].update = update;
    data[i].rq_rate = rq_rate;
    data[i].alternate = alternate;
    data[i].unit_tx = unit_tx;
    data[i].alternate = alternate;
    data[i].effective = effective;
    data[i].nb_add = 0;
    data[i].nb_added = 0;
    data[i].nb_remove = 0;
    data[i].nb_removed = 0;
    data[i].nb_contains = 0;
    data[i].nb_found = 0;
    data[i].nb_rqs = 0;
    data[i].nb_successful_rqs = 0;
    data[i].nb_aborts = 0;
    data[i].nb_aborts_locked_read = 0;
    data[i].nb_aborts_locked_write = 0;
    data[i].nb_aborts_validate_read = 0;
    data[i].nb_aborts_validate_write = 0;
    data[i].nb_aborts_validate_commit = 0;
    data[i].nb_aborts_invalid_memory = 0;
    data[i].max_retries = 0;
    data[i].seed = rand();
    data[i].set = set;
    data[i].barrier = &barrier;
    data[i].timing.read_dur = 0;
    data[i].timing.insert_dur = 0;
    data[i].timing.insert_crit = 0;
    data[i].timing.remove_dur = 0;
    data[i].timing.remove_crit = 0;
    data[i].timing.rq_dur = 0;
    data[i].timing.total_dur = 0;
    if (pthread_create(&threads[i], &attr, test, (void *)(&data[i])) != 0) {
      fprintf(stderr, "Error creating thread\n");
      exit(1);
    }
  }
  pthread_attr_destroy(&attr);

  /* Start threads */
  barrier_cross(&barrier);

  printf("STARTING...\n");
  gettimeofday(&start, NULL);
  if (duration > 0) {
    nanosleep(&timeout, NULL);
  } else {
    sigemptyset(&block_set);
    sigsuspend(&block_set);
  }
  AO_store_full(&stop, 1);
  gettimeofday(&end, NULL);
  printf("STOPPING...\n");

  /* Wait for thread completion */
  for (i = 0; i < nb_threads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "Error waiting for thread completion\n");
      exit(1);
    }
  }

  duration = (end.tv_sec * 1000 + end.tv_usec / 1000) -
             (start.tv_sec * 1000 + start.tv_usec / 1000);
  aborts = 0;
  aborts_locked_read = 0;
  aborts_locked_write = 0;
  aborts_validate_read = 0;
  aborts_validate_write = 0;
  aborts_validate_commit = 0;
  aborts_invalid_memory = 0;
  reads = 0;
  effreads = 0;
  rqs = 0;
  effrqs = 0;
  updates = 0;
  effupds = 0;
  inserts = 0;
  effinserts = 0;
  removes = 0;
  effremoves = 0;
  read_dur = 0;
  insert_dur = 0;
  insert_crit = 0;
  remove_dur = 0;
  remove_crit = 0;
  rq_dur = 0;
  total_dur = 0;
  max_retries = 0;
  for (i = 0; i < nb_threads; i++) {
    printf("Thread %d\n", i);
    printf("  #add        : %lu\n", data[i].nb_add);
    printf("    #added    : %lu\n", data[i].nb_added);
    printf("  #remove     : %lu\n", data[i].nb_remove);
    printf("    #removed  : %lu\n", data[i].nb_removed);
    printf("  #contains   : %lu\n", data[i].nb_contains);
    printf("    #found    : %lu\n", data[i].nb_found);
    printf("  #rqs        : %lu\n", data[i].nb_rqs);
    printf("    #rqs found: %lu\n", data[i].nb_successful_rqs);
    printf("  #aborts     : %lu\n", data[i].nb_aborts);
    printf("    #lock-r   : %lu\n", data[i].nb_aborts_locked_read);
    printf("    #lock-w   : %lu\n", data[i].nb_aborts_locked_write);
    printf("    #val-r    : %lu\n", data[i].nb_aborts_validate_read);
    printf("    #val-w    : %lu\n", data[i].nb_aborts_validate_write);
    printf("    #val-c    : %lu\n", data[i].nb_aborts_validate_commit);
    printf("    #inv-mem  : %lu\n", data[i].nb_aborts_invalid_memory);
    printf("  Max retries : %lu\n", data[i].max_retries);
    aborts += data[i].nb_aborts;
    aborts_locked_read += data[i].nb_aborts_locked_read;
    aborts_locked_write += data[i].nb_aborts_locked_write;
    aborts_validate_read += data[i].nb_aborts_validate_read;
    aborts_validate_write += data[i].nb_aborts_validate_write;
    aborts_validate_commit += data[i].nb_aborts_validate_commit;
    aborts_invalid_memory += data[i].nb_aborts_invalid_memory;
    reads += data[i].nb_contains;
    effreads += data[i].nb_contains + (data[i].nb_add - data[i].nb_added) +
                (data[i].nb_remove - data[i].nb_removed);
    rqs += data[i].nb_rqs;
    effrqs += data[i].nb_successful_rqs;
    updates += (data[i].nb_add + data[i].nb_remove);
    effupds += data[i].nb_removed + data[i].nb_added;
    inserts += data[i].nb_add;
    effinserts += data[i].nb_added;
    removes += data[i].nb_remove;
    effremoves += data[i].nb_removed;

    /* Timing */
    read_dur += data[i].timing.read_dur;
    insert_dur += data[i].timing.insert_dur;
    insert_crit += data[i].timing.insert_crit;
    remove_dur += data[i].timing.remove_dur;
    remove_crit += data[i].timing.remove_crit;
    rq_dur += data[i].timing.rq_dur;
    total_dur += data[i].timing.total_dur;

    // size += data[i].diff;
    size += data[i].nb_added - data[i].nb_removed;
    if (max_retries < data[i].max_retries) max_retries = data[i].max_retries;
  }
  printf("Set size      : %d (expected: %d)\n", set_size_l(set), size);
  printf("Duration      : %d (ms)\n", duration);
  printf("#txs          : %lu (%f / s)\n", reads + updates + rqs,
         (reads + updates + rqs) * 1000.0 / duration);

  printf("#read txs     : ");
  if (effective) {
    printf("%lu (%f / s)\n", effreads, effreads * 1000.0 / duration);
    printf("  #contains   : %lu (%f / s)\n", reads, reads * 1000.0 / duration);
  } else
    printf("%lu (%f / s)\n", reads, reads * 1000.0 / duration);
  printf("  latency (us): %4.2f\n",
         ((read_dur * MILLION) / CLOCKS_PER_SEC) / (double)reads);

  printf("#rq txs       : ");
  if (effective) {
    printf("%lu (%f / s)\n", effrqs, effrqs * 1000.0 / duration);
    printf("  #rq trials  : %lu (%f / s)\n", rqs, rqs * 1000.0 / duration);
  } else {
    printf("%lu (%f / s)\n", rqs, rqs * 1000.0 / duration);
  }
  printf("  latency (us): %4.2f\n",
         ((rq_dur * MILLION) / CLOCKS_PER_SEC) / (double)rqs);

  printf("#eff. upd rate: %f \n", 100.0 * effupds / (effupds + effreads));

  printf("#update txs   : ");
  if (effective) {
    printf("%lu (%f / s)\n", effupds, effupds * 1000.0 / duration);
    printf("  #upd trials : %lu (%f / s)\n", updates,
           updates * 1000.0 / duration);
  } else
    printf("%lu (%f / s)\n", updates, updates * 1000.0 / duration);

  /* Insert and remove timing */
  printf("total latency :\n");
  printf("  insert (us) : %4.2f\n",
         ((insert_dur * MILLION) / CLOCKS_PER_SEC) / (double)inserts);
  printf("  remove (us) : %4.2f\n",
         ((remove_dur * MILLION) / CLOCKS_PER_SEC) / (double)removes);
  printf("custom latency: "XSTR(TIMING)"\n");
  printf("  insert (us) : %4.2f\n", ((insert_crit * MILLION) / CLOCKS_PER_SEC) / (double)effinserts);
  printf("  remove (us) : %4.2f\n", ((remove_crit * MILLION) / CLOCKS_PER_SEC) / (double)effremoves);

  printf("#aborts       : %lu (%f / s)\n", aborts, aborts * 1000.0 / duration);
  printf("  #lock-r     : %lu (%f / s)\n", aborts_locked_read,
         aborts_locked_read * 1000.0 / duration);
  printf("  #lock-w     : %lu (%f / s)\n", aborts_locked_write,
         aborts_locked_write * 1000.0 / duration);
  printf("  #val-r      : %lu (%f / s)\n", aborts_validate_read,
         aborts_validate_read * 1000.0 / duration);
  printf("  #val-w      : %lu (%f / s)\n", aborts_validate_write,
         aborts_validate_write * 1000.0 / duration);
  printf("  #val-c      : %lu (%f / s)\n", aborts_validate_commit,
         aborts_validate_commit * 1000.0 / duration);
  printf("  #inv-mem    : %lu (%f / s)\n", aborts_invalid_memory,
         aborts_invalid_memory * 1000.0 / duration);
  printf("Max retries   : %lu\n", max_retries);

  /* Delete set */
  set_delete_l(set);

  free(threads);
  free(data);
  return 0;
}
