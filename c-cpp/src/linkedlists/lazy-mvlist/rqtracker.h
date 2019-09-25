#ifndef MVLIST_RQTRACKER_H
#define MVLIST_RQTRACKER_H

#include "mvl.h"

rqtracker_l_t *rqtracker_new_l(uint32_t max_rq);
timestamp_t *rqtracker_snapshot_active_l(rqtracker_l_t *rqt,
                                         uint32_t *num_active);
timestamp_t rqtracker_start_update_l(rqtracker_l_t *rqt);
void rqtracker_end_update_l(rqtracker_l_t *rqt);
timestamp_t rqtracker_start_rq_l(rqtracker_l_t *rqt, uint32_t rq_id);
void rqtracker_end_rq_l(rqtracker_l_t *rqt, uint32_t rq_id);
void rqtracker_delete_l(rqtracker_l_t *rqt);

#endif