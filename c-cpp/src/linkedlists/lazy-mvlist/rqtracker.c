#include "rqtracker.h"

rqtracker_l_t *rqtracker_new_l(uint32_t max_rq) {
  rqtracker_l_t *rqt;
  int i;

  rqt = (rqtracker_l_t *)malloc(sizeof(rqtracker_l_t));
  rqt->max_rq = max_rq;
  rqt->ts = MIN_TIMESTAMP;
  rqt->active = (timestamp_t *)malloc(sizeof(timestamp_t) * max_rq);
  for (i = 0; i < max_rq; ++i) {
    rqt->active[i] = NULL_TIMESTAMP;
  }
  rqt->num_active = 0;
  return rqt;
}

timestamp_t *rqtracker_snapshot_active_l(rqtracker_l_t *rqt,
                                         uint32_t *num_active) {
  timestamp_t *s;
  timestamp_t temp;
  int curr_rq, end, i, j, k;

  if (rqt->num_active == 0) {
    *num_active = 0;
    return NULL;
  }

  end = rqt->max_rq;
  s = (timestamp_t *)malloc(sizeof(timestamp_t) * rqt->max_rq);
  /* TODO(jacnel): Optimize taking a snapshot of the active RQs. */
  for (i = 0; i < rqt->max_rq; ++i) {
    curr_rq = rqt->active[i];
    if (curr_rq == NULL_TIMESTAMP) {
      s[--end] = NULL_TIMESTAMP;
    } else {
      for (j = 0; j < i && s[j] < curr_rq; ++j)
        ;
      k = j;
      if (k != i) {
        for (j = end - 1; j > k; --j) {
          s[j] = s[j - 1];
        }
      }
      s[k] = curr_rq;
    }
  }
  *num_active = end;
  return s;
}

timestamp_t rqtracker_start_update_l(rqtracker_l_t *rqt) {
  while (AO_compare_and_swap_full(&rqt->flag, 0, 1))
    ;
  return rqt->ts + 1;
}

void rqtracker_end_update_l(rqtracker_l_t *rqt) {
  ++rqt->ts;
  rqt->flag = 0;
  AO_compiler_barrier();
}

timestamp_t rqtracker_start_rq_l(rqtracker_l_t *rqt, uint32_t rq_id) {
  timestamp_t ts;
  ts = rqt->ts;
  rqt->active[rq_id] = ts;
  AO_int_fetch_and_add1(&rqt->num_active);
  return ts;
}

void rqtracker_end_rq_l(rqtracker_l_t *rqt, uint32_t rq_id) {
  rqt->active[rq_id] = NULL_TIMESTAMP;
  AO_int_fetch_and_sub1(&rqt->num_active);
}

void rqtracker_delete_l(rqtracker_l_t *rqt) {
  free(rqt->active);
}