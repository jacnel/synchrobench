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
  INIT_LOCK(&rqt->update_lock);
  INIT_LOCK(&rqt->active_lock);
  return rqt;
}

timestamp_t *rqtracker_snapshot_active_l(rqtracker_l_t *rqt,
                                         uint32_t *num_active) {
  timestamp_t *s;
  timestamp_t curr;
  int i, j;

  s = (timestamp_t *)malloc(sizeof(timestamp_t) * rqt->max_rq);
  /* TODO(jacnel): Optimize taking a snapshot of the active RQs. */
  LOCK(&rqt->active_lock);
  for (i = 0, j = 0; i < rqt->max_rq; ++i) {
    curr = rqt->active[i];
    if (curr != NULL_TIMESTAMP) {
      s[j++] = curr;
    }
  }
  UNLOCK(&rqt->active_lock);
  *num_active = j;
  return s;
}

timestamp_t rqtracker_start_update_l(rqtracker_l_t *rqt) {
  LOCK(&rqt->update_lock);
  return rqt->ts + 1;
}

void rqtracker_end_update_l(rqtracker_l_t *rqt) {
  ++rqt->ts;
  UNLOCK(&rqt->update_lock);
}

timestamp_t rqtracker_start_rq_l(rqtracker_l_t *rqt, uint32_t rq_id) {
  timestamp_t ts;
  LOCK(&rqt->active_lock);
  ts = rqt->ts;
  rqt->active[rq_id] = ts;
  UNLOCK(&rqt->active_lock);
  return ts;
}

void rqtracker_end_rq_l(rqtracker_l_t *rqt, uint32_t rq_id) {
  rqt->active[rq_id] = NULL_TIMESTAMP;
}

void rqtracker_delete_l(rqtracker_l_t *rqt) {
  free((void *)rqt->active);
  DESTROY_LOCK(&rqt->active_lock);
}