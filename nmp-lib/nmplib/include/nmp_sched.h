/* *
 * This file implements event scheduler.
 *
 * Copyright (C) 2012 fangyi <fangyi@szjxj.net>
 * See COPYING for more details
 *
 * */

#ifndef __J_SCHED_H__
#define __J_SCHED_H__

#include "nmp_event.h"

typedef struct _JLoopScher JLoopScher;

JLoopScher *j_sched_new(int loops);
int j_sched_add(JLoopScher *sched, JEvent *src, unsigned weight);
int j_sched_remove(JLoopScher *sched, JEvent *src);

#endif	/* __J_SCHEDULER_H__ */
