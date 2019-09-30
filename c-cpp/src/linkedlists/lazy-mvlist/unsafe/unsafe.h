/*
 * File:
 *   linkedlist-lock.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Lock-based linked list implementation of an integer set
 *
 * Copyright (c) 2009-2010.
 *
 * linkedlist-lock.c is part of Synchrobench
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

#ifndef UNSAFE_UNSAFE_H
#define UNSAFE_UNSAFE_H

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#include <atomic_ops.h>
#include "../mvl.h"

typedef struct node_unsafe_l {
  val_t val;
  struct node_unsafe_l *next;
  volatile ptlock_t lock;
} node_unsafe_l_t;

typedef struct intset_unsafe_l {
  node_unsafe_l_t *head;
} intset_unsafe_l_t;

#endif