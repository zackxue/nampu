#include "nmp_debug.h"

#define MAX_LOOP_COUNT			8
#define DEFAULT_LOOP_COUNT		2


typedef struct _NmpLoop NmpLoop;
struct _NmpLoop
{
	guint			weight;

	GMainContext	*loop_context;		/* 上下文 */
	GMainLoop		*loop;
	GThread			*loop_thread;
};

typedef struct _NmpSrcWeight NmpSrcWeight;
struct _NmpSrcWeight
{
	guint		w;		/* 权重 */

	GSource		*src;
	NmpLoop		*loop;
};


typedef struct _NmpLoopScher NmpLoopScher;
struct _NmpLoopScher
{
	NmpLoop			*loops;
	GMutex			*lock;

	guint			count;
	guint			total_weight;
	GList			*w_list;			
};


static NmpLoopScher *scheduler;

static gpointer
nmp_loop_thread_fun(NmpLoop *loop)
{
	g_main_loop_run(loop->loop);
	return NULL;
}


static __inline__ void
nmp_loop_init(NmpLoop *loop)
{
	GError *error = NULL;

	loop->loop_context = g_main_context_new();
	loop->loop = g_main_loop_new(loop->loop_context, FALSE);

	loop->loop_thread = g_thread_create(
  		(GThreadFunc)nmp_loop_thread_fun,
  		loop,
  		FALSE,
  		&error
  	);

  	if (error != NULL)
  	{
  		g_critical("Can't start loop thread: %s", error->message);
  		FATAL_ERROR_EXIT;
  	}
}


static __inline__ void
nmp_loop_add_source(NmpLoop *loop, GSource *src, guint weight)
{
	g_source_attach(src, loop->loop_context);
	loop->weight += weight;
}


static __inline__ void
nmp_loop_del_source(NmpLoop *loop, GSource *src, guint weight)
{
	g_source_destroy(src);
	loop->weight -= weight;
}


NmpLoopScher*
nmp_loop_scheduler_new(gint loops)
{
	NmpLoopScher *sched;
	gsize size;

	if (loops > MAX_LOOP_COUNT)
		loops = MAX_LOOP_COUNT;

	if (loops <= 0)
		loops = 1;	/* At least one loop */

	size = loops * sizeof(NmpLoop);
	sched = g_new0(NmpLoopScher, 1);
	sched->loops = (NmpLoop*)g_malloc0(size);
	sched->lock = g_mutex_new();
	sched->count = loops;
	sched->total_weight = 0;
	sched->w_list = NULL;

	while (--loops >= 0)
	{
		nmp_loop_init(&sched->loops[loops]);
	}

	return sched;
}


static __inline__ NmpLoop *
nmp_find_best_loop(NmpLoopScher *sched)
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
printf("==========================================> best loop: %d\n", best_i);
	return sched->loops + best_i;
}


static __inline__ void
nmp_loop_scheduler_add_weight(NmpLoopScher *sched, NmpLoop *loop,
	GSource *src, guint weight)
{
	NmpSrcWeight *sw;

	sw = g_new0(NmpSrcWeight, 1);
	sw->w = weight;
	sw->src = src;
	sw->loop = loop;

	sched->total_weight += weight;
	sched->w_list = g_list_prepend(sched->w_list, sw);
}


static gint
nmp_loop_find_sw(gconstpointer a, gconstpointer src)
{
	NmpSrcWeight *sw = (NmpSrcWeight*)a;

	if (sw->src == (GSource*)src)
		return 0;
	return 1;
}

                                                 
static __inline__ void
nmp_loop_scheduler_del_weight(NmpLoopScher *sched, GSource *src)
{
	NmpSrcWeight *sw;
	GList *list;

	list = g_list_find_custom(sched->w_list, src, nmp_loop_find_sw);
	if (list)
	{
		sw = (NmpSrcWeight*)list->data;
		sched->total_weight -= sw->w;
		sched->w_list = g_list_delete_link(sched->w_list, list);
		nmp_loop_del_source(sw->loop, src, sw->w);
		g_free(sw);
		return;
	}

	nmp_warning(
		"Can't del nonexistent gsource object in scheduler."
	);
}


static __inline__ void
__nmp_loop_schedule_source(NmpLoopScher *sched, GSource *src,
	guint weight)
{
	NmpLoop *loop = nmp_find_best_loop(sched);
	nmp_loop_add_source(loop, src, weight);
	nmp_loop_scheduler_add_weight(sched, loop, src, weight);
}


static __inline__ void
nmp_loop_schedule_source(NmpLoopScher *sched, GSource *src,
	guint weight)
{
	G_ASSERT(sched != NULL);

	g_mutex_lock(sched->lock);
	__nmp_loop_schedule_source(sched, src, weight);
	g_mutex_unlock(sched->lock);
}


static __inline__ void
nmp_loop_remove_source(NmpLoopScher *sched, GSource *src)
{
	G_ASSERT(sched != NULL);

	g_mutex_lock(sched->lock);
	nmp_loop_scheduler_del_weight(sched, src);
	g_mutex_unlock(sched->lock);
}


void
nmp_scheduler_init( void )
{
	scheduler = nmp_loop_scheduler_new(DEFAULT_LOOP_COUNT);
}


void
nmp_scheduler_add(GSource *source, guint weight)
{
	nmp_loop_schedule_source(scheduler, source, weight);
}


void
nmp_scheduler_del(GSource *source)
{
	nmp_loop_remove_source(scheduler, source);
}


//:~ End
