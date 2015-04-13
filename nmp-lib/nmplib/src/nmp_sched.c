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


typedef struct _nmp_loot nmp_loop_t;
struct _nmp_loot
{
	unsigned			weight;

	nmp_event_loop_t		*loop;
	nmp_thread_t			*loop_thread;
};

typedef struct _nmp_src_weight nmp_src_weight;
struct _nmp_src_weight
{
	unsigned		w;		/* ШЈжи */

	nmp_event_t		*src;
	nmp_loop_t		*loop;
};

struct _nmp_loop_scher
{
	nmp_loop_t		*loops;
	nmp_mutex_t		*lock;

	unsigned		count;
	unsigned		total_weight;
	nmp_list_t		*w_list;
};


static void*
nmp_loop_thread_fun(nmp_loop_t *loop)
{
	nmp_event_loop_run(loop->loop);
	return NULL;
}


static __inline__ void
nmp_loop_init(nmp_loop_t *loop)
{
	loop->weight = 0;
	loop->loop = nmp_event_loop_new();
	loop->loop_thread = nmp_thread_create(
  		(nmp_thread_func)nmp_loop_thread_fun,
  		loop,
  		FALSE,
  		NULL
  	);
}


static __inline__ void
nmp_loop_add_source(nmp_loop_t *loop, nmp_event_t *src, unsigned weight)
{
	nmp_event_loop_attach(loop->loop, src);
	loop->weight += weight;
}


static __inline__ void
nmp_loop_del_source(nmp_loop_t *loop, nmp_event_t *src, unsigned weight)
{
	nmp_event_remove(src);
	loop->weight -= weight;
}


nmp_loop_scher_t*
nmp_sched_new(int loops)
{
	nmp_loop_scher_t *sched;
	int size;

	size = loops * sizeof(nmp_loop_t);
	sched = nmp_new0(nmp_loop_scher_t, 1);
	sched->loops = (nmp_loop_t*)nmp_alloc0(size);
	sched->lock = nmp_mutex_new();
	sched->count = loops;
	sched->total_weight = 0;
	sched->w_list = NULL;

	while (--loops >= 0)
	{
		nmp_loop_init(&sched->loops[loops]);
	}

	return sched;
}


static __inline__ nmp_loop_t *
nmp_find_best_loop(nmp_loop_scher_t *sched)
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
nmp_sched_add_weight(nmp_loop_scher_t *sched, nmp_loop_t *loop,
	nmp_event_t *src, unsigned weight)
{
	nmp_src_weight *sw;

	sw = nmp_new0(nmp_src_weight, 1);
	sw->w = weight;
	sw->src = src;
	sw->loop = loop;

	sched->total_weight += weight;
	sched->w_list = nmp_list_add(sched->w_list, sw);
}


static int
nmp_loop_find_sw(void *a, void *src)
{
	nmp_src_weight *sw = (nmp_src_weight*)a;

	if (sw->src == (nmp_event_t*)src)
		return 0;
	return 1;
}


static __inline__ int
__nmp_sched_remove(nmp_loop_scher_t *sched, nmp_event_t *src)
{
	nmp_src_weight *sw;
	nmp_list_t *list;

	list = nmp_list_find_custom(sched->w_list, src, nmp_loop_find_sw);
	if (list)
	{
		sw = (nmp_src_weight*)list->data;
		sched->total_weight -= sw->w;
		sched->w_list = nmp_list_delete_link(sched->w_list, list);
		nmp_loop_del_source(sw->loop, src, sw->w);
		nmp_del(sw, nmp_src_weight, 1);
		return 0;
	}

	return -1;
}


static __inline__ void
__nmp_sched_add(nmp_loop_scher_t *sched, nmp_event_t *src,
	unsigned weight)
{
	nmp_loop_t *loop = nmp_find_best_loop(sched);
	nmp_loop_add_source(loop, src, weight);
	nmp_sched_add_weight(sched, loop, src, weight);
}


int
nmp_sched_add(nmp_loop_scher_t *sched, nmp_event_t *src,
	unsigned weight)
{
	NMP_ASSERT(sched != NULL);

	nmp_mutex_lock(sched->lock);
	__nmp_sched_add(sched, src, weight);
	nmp_mutex_unlock(sched->lock);

	return 0;
}


int
nmp_sched_remove(nmp_loop_scher_t *sched, nmp_event_t *src)
{
	int ret;
	NMP_ASSERT(sched != NULL);

	nmp_mutex_lock(sched->lock);
	ret = __nmp_sched_remove(sched, src);
	nmp_mutex_unlock(sched->lock);

	return ret;
}


//:~ End
