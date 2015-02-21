/* *
 * This file implements event scheduler using glib and libev.
 *
 * Copyright (C) 2012 NAMPU
 * See COPYING for more details
 *
 * */
 
#ifndef __NMPEV_SCHED_H__
#define __NMPEV_SCHED_H__

#include "nmpev.h"

void g_scheduler_init(gint nloop);
void g_scheduler_add(GEvent *source, guint weight);
void g_scheduler_del(GEvent *source);

#endif	/* __GEV_SCHED_H__ */
