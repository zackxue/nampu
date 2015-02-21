/* *
 * This file implements event scheduler using glib and libev.
 *
 * Copyright (C) 2012 NAMPU
 * See COPYING for more details
 *
 * */

#include <unistd.h>
#include "nmpevsched.h"

#define MAX_LOOP_COUNT			8
#define MIN_LOOP_COUNT			2

#define BUG()	\
	g_assert(("BUG()!!!!!!", 0));

typedef struct _GLoop GLoop;
struct _GLoop
{
	guint			weight;

	GEventLoop		*loop;
	GThread			*loop_thread;
};

typedef struct _GSrcWeight GSrcWeight;
struct _GSrcWeight
{
	guint		w;		/* ШЈжи */

	GEvent		*src;
	GLoop		*loop;
};


typedef struct _GLoopScher GLoopScher;
struct _GLoopScher
{
	GLoop			*loops;
	GMutex			*lock;

	guint			count;
	guint			total_weight;
	GList			*w_list;			
};


static GLoopScher *scheduler;

static gpointer
g_loop_thread_fun(GLoop *loop)
{
	g_event_loop_run(loop->loop);
	return NULL;
}


static __inline__ void
g_loop_init(GLoop *loop)
{
	GError *error = NULL;

	loop->weight = 0;
	loop->loop = g_event_loop_new();
	loop->loop_thread = g_thread_create(
  		(GThreadFunc)g_loop_thread_fun,
  		loop,
  		FALSE,
  		&error
  	);

  	if (error != NULL)
  	{
  		g_critical("Can't start loop thread: %s", error->message);
  		BUG();
  	}
}


static __inline__ void
g_loop_add_source(GLoop *loop, GEvent *src, guint weight)
{
	g_event_loop_attach(loop->loop, src);
	loop->weight += weight;
}


static __inline__ void
g_loop_del_source(GLoop *loop, GEvent *src, guint weight)
{
	g_event_remove(src);
	loop->weight -= weight;
}


GLoopScher*
g_loop_scheduler_new(gint loops)
{
	GLoopScher *sched;
	gsize size;

	size = loops * sizeof(GLoop);
	sched = g_new0(GLoopScher, 1);
	sched->loops = (GLoop*)g_malloc0(size);
	sched->lock = g_mutex_new();
	sched->count = loops;
	sched->total_weight = 0;
	sched->w_list = NULL;

	while (--loops >= 0)
	{
		g_loop_init(&sched->loops[loops]);
	}

	return sched;
}


static __inline__ GLoop *
g_find_best_loop(GLoopScher *sched)
{
	guint w, best_w = sched->total_weight;
	gint best_i = 0, idx = 0;

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
g_loop_scheduler_add_weight(GLoopScher *sched, GLoop *loop,
	GEvent *src, guint weight)
{
	GSrcWeight *sw;

	sw = g_new0(GSrcWeight, 1);
	sw->w = weight;
	sw->src = src;
	sw->loop = loop;

	sched->total_weight += weight;
	sched->w_list = g_list_prepend(sched->w_list, sw);
}


static gint
g_loop_find_sw(gconstpointer a, gconstpointer src)
{
	GSrcWeight *sw = (GSrcWeight*)a;

	if (sw->src == (GEvent*)src)
		return 0;
	return 1;
}

                                                 
static __inline__ void
g_loop_scheduler_del_weight(GLoopScher *sched, GEvent *src)
{
	GSrcWeight *sw;
	GList *list;

	list = g_list_find_custom(sched->w_list, src, g_loop_find_sw);
	if (list)
	{
		sw = (GSrcWeight*)list->data;
		sched->total_weight -= sw->w;
		sched->w_list = g_list_delete_link(sched->w_list, list);
		g_loop_del_source(sw->loop, src, sw->w);
		g_free(sw);
		return;
	}

	g_critical(
		"Can't del nonexistent gsource object in scheduler."
	);
}


static __inline__ void
__g_loop_schedule_source(GLoopScher *sched, GEvent *src,
	guint weight)
{
	GLoop *loop = g_find_best_loop(sched);
	g_loop_add_source(loop, src, weight);
	g_loop_scheduler_add_weight(sched, loop, src, weight);
}


static __inline__ void
g_loop_schedule_source(GLoopScher *sched, GEvent *src,
	guint weight)
{
	g_assert(sched != NULL);

	g_mutex_lock(sched->lock);
	__g_loop_schedule_source(sched, src, weight);
	g_mutex_unlock(sched->lock);
}


static __inline__ void
g_loop_remove_source(GLoopScher *sched, GEvent *src)
{
	g_assert(sched != NULL);

	g_mutex_lock(sched->lock);
	g_loop_scheduler_del_weight(sched, src);
	g_mutex_unlock(sched->lock);
}


void
g_scheduler_init(gint nloop)
{
	static gsize init_scheduler = FALSE;

	if (nloop <= 0)
		nloop = MIN_LOOP_COUNT;

	if (nloop > MAX_LOOP_COUNT)
		nloop = MAX_LOOP_COUNT;

	if (g_once_init_enter(&init_scheduler))
	{
		scheduler = g_loop_scheduler_new(nloop);
		g_once_init_leave(&init_scheduler, TRUE);
	}
}


void
g_scheduler_add(GEvent *source, guint weight)
{
	g_loop_schedule_source(scheduler, source, weight);
}


void
g_scheduler_del(GEvent *source)
{
	g_loop_remove_source(scheduler, source);
}


//:~ End
