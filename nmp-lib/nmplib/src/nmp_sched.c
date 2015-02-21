/* *
 * This file implements event scheduler.
 *
 * Copyright (C) 2012 fangyi <fangyi@szjxj.net>
 * See COPYING for more details
 *
 * */

#include <unistd.h>
#include "nmp_thread.h"
#include "nmp_list.h"
#include "nmp_mem.h"
#include "nmp_sched.h"

#define MAX_LOOP_COUNT			8
#define MIN_LOOP_COUNT			2


typedef struct _JLoop JLoop;
struct _JLoop
{
	unsigned			weight;

	JEventLoop		*loop;
	JThread			*loop_thread;
};

typedef struct _JSrcWeight JSrcWeight;
struct _JSrcWeight
{
	unsigned		w;		/* ШЈжи */

	JEvent		*src;
	JLoop		*loop;
};

struct _JLoopScher
{
	JLoop			*loops;
	JMutex			*lock;

	unsigned		count;
	unsigned		total_weight;
	JList			*w_list;
};


static void*
j_loop_thread_fun(JLoop *loop)
{
	j_event_loop_run(loop->loop);
	return NULL;
}


static __inline__ void
j_loop_init(JLoop *loop)
{
	loop->weight = 0;
	loop->loop = j_event_loop_new();
	loop->loop_thread = j_thread_create(
  		(JThreadFunc)j_loop_thread_fun,
  		loop,
  		FALSE,
  		NULL
  	);
}


static __inline__ void
j_loop_add_source(JLoop *loop, JEvent *src, unsigned weight)
{
	j_event_loop_attach(loop->loop, src);
	loop->weight += weight;
}


static __inline__ void
j_loop_del_source(JLoop *loop, JEvent *src, unsigned weight)
{
	j_event_remove(src);
	loop->weight -= weight;
}


JLoopScher*
j_sched_new(int loops)
{
	JLoopScher *sched;
	int size;

	size = loops * sizeof(JLoop);
	sched = j_new0(JLoopScher, 1);
	sched->loops = (JLoop*)j_alloc0(size);
	sched->lock = j_mutex_new();
	sched->count = loops;
	sched->total_weight = 0;
	sched->w_list = NULL;

	while (--loops >= 0)
	{
		j_loop_init(&sched->loops[loops]);
	}

	return sched;
}


static __inline__ JLoop *
j_find_best_loop(JLoopScher *sched)
{
	unsigned w, best_w = sched->total_weight;
	int best_i = 0, idx = 0;

	for (; idx < sched->count; ++idx)
	{
		w = sched->loops[idx].weight;
		if (w < best_w)
		{
			best_w = w;
			best_i = idx;
		}
	}

	return sched->loops + best_i;
}


static __inline__ void
j_sched_add_weight(JLoopScher *sched, JLoop *loop,
	JEvent *src, unsigned weight)
{
	JSrcWeight *sw;

	sw = j_new0(JSrcWeight, 1);
	sw->w = weight;
	sw->src = src;
	sw->loop = loop;

	sched->total_weight += weight;
	sched->w_list = j_list_add(sched->w_list, sw);
}


static int
j_loop_find_sw(void *a, void *src)
{
	JSrcWeight *sw = (JSrcWeight*)a;

	if (sw->src == (JEvent*)src)
		return 0;
	return 1;
}


static __inline__ int
__j_sched_remove(JLoopScher *sched, JEvent *src)
{
	JSrcWeight *sw;
	JList *list;

	list = j_list_find_custom(sched->w_list, src, j_loop_find_sw);
	if (list)
	{
		sw = (JSrcWeight*)list->data;
		sched->total_weight -= sw->w;
		sched->w_list = j_list_delete_link(sched->w_list, list);
		j_loop_del_source(sw->loop, src, sw->w);
		j_del(sw, JSrcWeight, 1);
		return 0;
	}

	return -1;
}


static __inline__ void
__j_sched_add(JLoopScher *sched, JEvent *src,
	unsigned weight)
{
	JLoop *loop = j_find_best_loop(sched);
	j_loop_add_source(loop, src, weight);
	j_sched_add_weight(sched, loop, src, weight);
}


int
j_sched_add(JLoopScher *sched, JEvent *src,
	unsigned weight)
{
	J_ASSERT(sched != NULL);

	j_mutex_lock(sched->lock);
	__j_sched_add(sched, src, weight);
	j_mutex_unlock(sched->lock);

	return 0;
}


int
j_sched_remove(JLoopScher *sched, JEvent *src)
{
	int ret;
	J_ASSERT(sched != NULL);

	j_mutex_lock(sched->lock);
	ret = __j_sched_remove(sched, src);
	j_mutex_unlock(sched->lock);

	return ret;
}


//:~ End
