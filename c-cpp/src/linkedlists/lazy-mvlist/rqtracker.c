#include "rqtracker.h"

rqtracker_l_t *rqtracker_new_l(uint32_t max_rq) {
  rqtracker_l_t *rqt;
  int i;

  rqt = (rqtracker_l_t *)malloc(sizeof(rqtracker_l_t));
  rqt->max_rq = max_rq;
  rqt->update_ts = MIN_TIMESTAMP;
  rqt->active_ts = NULL_TIMESTAMP;
  rqt->active = (timestamp_t *)malloc(sizeof(timestamp_t) * max_rq);
  rqt->active_flag = (uint8_t *)malloc(sizeof(uint8_t) * max_rq);
  for (i = 0; i < max_rq; ++i) {
    rqt->active[i] = NULL_TIMESTAMP;
    rqt->active_flag[i] = 0;
  }
  return rqt;
}

timestamp_t *rqtracker_snapshot_active_l(rqtracker_l_t *rqt,
                                         timestamp_t *oldest_active,
                                         timestamp_t *newest_active,
                                         uint32_t *num_active) {
  timestamp_t *s;
  timestamp_t curr;
  int i, j;

  s = (timestamp_t *)malloc(sizeof(timestamp_t) * rqt->max_rq);
  *oldest_active = MAX_TIMESTAMP;
  *newest_active = MIN_TIMESTAMP;
  /* TODO: Optimize taking a snapshot of the active RQs. */
  for (i = 0, j = 0; i < rqt->max_rq; ++i) {
    while (rqt->active_flag[i] != 0)
      ;
    curr = rqt->active[i];
    if (curr != NULL_TIMESTAMP) {
      s[j++] = curr;
      if (curr < *oldest_active) {
        *oldest_active = curr;
      }
      if (curr > *newest_active) {
        *newest_active = curr;
      }
    }
  }
  *num_active = j;
  return s;
}

timestamp_t rqtracker_start_update_l(rqtracker_l_t *rqt) {
  return AO_fetch_and_add1(&rqt->update_ts);
}

void rqtracker_end_update_l(rqtracker_l_t *rqt, timestamp_t ts) {
  timestamp_t curr_ts;
  while (rqt->active_ts != ts - 1)
    ;
  ++rqt->active_ts;
  AO_compiler_barrier();
}

timestamp_t rqtracker_start_rq_l(rqtracker_l_t *rqt, uint32_t rq_id) {
  timestamp_t ts;
  rqt->active_flag[rq_id] = 1;
  AO_compiler_barrier();
  ts = rqt->active_ts;
  rqt->active[rq_id] = ts;
  AO_compiler_barrier();
  rqt->active_flag[rq_id] = 0;
  return ts;
}

void rqtracker_end_rq_l(rqtracker_l_t *rqt, uint32_t rq_id) {
  rqt->active[rq_id] = NULL_TIMESTAMP;
}

void rqtracker_delete_l(rqtracker_l_t *rqt) {
  free((void *)rqt->active);
  free((void *)rqt->active_flag);
}