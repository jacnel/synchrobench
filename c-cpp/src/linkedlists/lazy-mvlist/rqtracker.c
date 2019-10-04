#include "rqtracker.h"

rqtracker_l_t *rqtracker_new_l(uint32_t max_rq) {
  rqtracker_l_t *rqt;
  int i;

  rqt = (rqtracker_l_t *)malloc(sizeof(rqtracker_l_t));
  rqt->max_rq = max_rq;
  rqt->update_ts = MIN_TIMESTAMP;
  rqt->active_ts = NULL_TIMESTAMP;
  rqt->active = (timestamp_t *)malloc(sizeof(timestamp_t) * max_rq);
  for (i = 0; i < max_rq; ++i) {
    rqt->active[i] = NULL_TIMESTAMP;
  }
  INIT_LOCK(&rqt->active_rwlock);
  return rqt;
}

timestamp_t *rqtracker_snapshot_active_l(rqtracker_l_t *rqt,
                                         uint32_t *num_active) {
  timestamp_t *s;
  timestamp_t curr;
  int i, j;

  s = (timestamp_t *)malloc(sizeof(timestamp_t) * rqt->max_rq);
  /* TODO(jacnel): Optimize taking a snapshot of the active RQs. */
  LOCK(&rqt->active_rwlock);
  for (i = 0, j = 0; i < rqt->max_rq; ++i) {
    curr = rqt->active[i];
    if (curr != NULL_TIMESTAMP) {
      s[j++] = curr;
    }
  }
  UNLOCK(&rqt->active_rwlock);
  *num_active = j;
  return s;
}

timestamp_t rqtracker_start_update_l(rqtracker_l_t *rqt) {
  return AO_fetch_and_add1(&rqt->update_ts);
}

void rqtracker_end_update_l(rqtracker_l_t *rqt, timestamp_t ts) {
  timestamp_t curr_ts;
  curr_ts = rqt->active_ts;
  while (curr_ts < ts) {
    AO_compare_and_swap_full(&rqt->active_ts, curr_ts, ts);
    curr_ts = rqt->active_ts;
  }
}

timestamp_t rqtracker_start_rq_l(rqtracker_l_t *rqt, uint32_t rq_id) {
  timestamp_t ts;
  LOCK(&rqt->active_rwlock);
  ts = rqt->active_ts;
  rqt->active[rq_id] = ts;
  UNLOCK(&rqt->active_rwlock);
  return ts;
}

void rqtracker_end_rq_l(rqtracker_l_t *rqt, uint32_t rq_id) {
  LOCK(&rqt->active_rwlock);
  rqt->active[rq_id] = NULL_TIMESTAMP;
  UNLOCK(&rqt->active_rwlock);
}

void rqtracker_delete_l(rqtracker_l_t *rqt) {
  free((void *)rqt->active);
  DESTROY_LOCK(&rqt->active_rwlock);
}