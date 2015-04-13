/* *
 * This file implements event scheduler.
 *
 * Copyright (C) 2012 fangyi <fangyi@szjxj.net>
 * See COPYING for more details
 *
 * */

#ifndef __NMP_SCHED_H__
#define __NMP_SCHED_H__

#include "nmp_event.h"

typedef struct _nmp_loop_scher nmp_loop_scher_t;

nmp_loop_scher_t *nmp_sched_new(int loops);
int nmp_sched_add(nmp_loop_scher_t *sched, nmp_event_t *src, unsigned weight);
int nmp_sched_remove(nmp_loop_scher_t *sched, nmp_event_t *src);

#endif	/* __NMP_SCHEDULER_H__ */
