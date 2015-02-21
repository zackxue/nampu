#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "nmpev.h"

#define CONTAINER_OF(ptr, struct_type, member) \
	(struct_type*)((char*)(ptr) - G_STRUCT_OFFSET(struct_type, member));

#define FLAGS_EVENT_IN_LOOP				0x01

struct wakeup_block		/* for loop waking up */
{
	gint request_pending;	/* set if request is pending */
	struct ev_io waker;
#ifndef _WIN32
	gint wakeup_pipe[2];
#else
	HANDLE wakeup_sem;
#endif
};

typedef struct _GLoopOps GLoopOps;
typedef void (*GLoopOpsFunc)(GEventLoop *loop, GLoopOps *ops);

struct _GLoopOps
{
	GEvent *event;
	gint ops_type;		/* 0- operation on I/O, 1- operation on timer */
	GLoopOpsFunc func;
};


struct _GEventLoop
{
	gint ref_count;
	GMutex *lock;
	struct ev_loop *ev_loop;
	struct wakeup_block wakeup;
	gboolean loop_running;
	gboolean loop_quited;
	GList *events_list;			/* event object that attached to this loop */
	GQueue *operations;			/* operation queue */
};

static void g_event_loop_force_woken_up(GEventLoop *loop);
static void g_event_loop_sync_remove(GEventLoop *loop, GEvent *event);
static void g_event_loop_remove(GEventLoop *loop, GEvent *event);


static void
g_event_loop_wakeup_cb(EV_P_ struct ev_io *w, gint revents)
{
	GEventLoop *gev_loop;

	gev_loop = CONTAINER_OF(w, GEventLoop, wakeup.waker);

	if (revents & EV_READ)
	{
		g_event_loop_force_woken_up(gev_loop);
		revents &= ~EV_READ;
	}
}


static gint
g_event_wakeup_init(struct wakeup_block *wb, GEventLoop *loop)
{
	if (pipe(wb->wakeup_pipe) < 0)
		return -errno;

	ev_io_init(&wb->waker, g_event_loop_wakeup_cb, wb->wakeup_pipe[0], EV_READ);
	ev_io_start(loop->ev_loop, &wb->waker);

	return 0;
}


static void
g_event_block_wakup(struct wakeup_block *wb)
{
	g_assert(wb != NULL);

	if (wb->request_pending)
		return;
	wb->request_pending = 1;
#ifndef _WIN32
	write(wb->wakeup_pipe[1], "x", 1);
#else
#endif
}


static __inline__ void
g_event_block_woken_up(struct wakeup_block *wb)
{
	gchar x;
	g_assert(wb != NULL);

	wb->request_pending = 0;
#ifndef _WIN32
	read(wb->wakeup_pipe[0], &x, 1);
#else
#endif	
}


static gboolean
g_event_call(GEvent *ev, gint revents)
{
	g_assert(ev != NULL);

	return (*ev->dispath)(ev, revents, ev->user_data);
}


static void
g_event_sync_remove(GEvent *ev)
{
	GEventLoop *loop = ev->loop;
	GList *list;

	g_mutex_lock(loop->lock);

	if ((list = g_list_find(loop->events_list, ev)))
	{
		loop->events_list = g_list_delete_link(loop->events_list, list);
		g_event_loop_sync_remove(loop, ev);
		g_event_unref(ev);
	}

	g_mutex_unlock(loop->lock);
}


static __inline__ void
g_event_reset_timer(GEvent *ev)
{
	if (ev->timeout > 0)
	{
		ev_timer_again(ev->loop->ev_loop, &ev->timer);
	}
}


static void
g_event_cb_io(EV_P_ struct ev_io *w, gint revents)
{
	GEvent *ev = CONTAINER_OF(w, GEvent, io);

	if (!g_event_call(ev, revents))
		g_event_sync_remove(ev);
	else
		g_event_reset_timer(ev);
}


static void
g_event_cb_to(EV_P_ struct ev_timer *w, gint revents)
{
	GEvent *ev = CONTAINER_OF(w, GEvent, timer);

	if (!g_event_call(ev, revents))
		g_event_sync_remove(ev);
}


GEvent *g_event_new(size_t size, gint fd, gint events)
{
	GEvent *ev;

	if (size < sizeof(GEvent))
		return NULL;

	ev = g_malloc(size);
	memset(ev, 0, size);
	ev->ref_count = 1;
	ev->event_size = size;

	events &= EVENTS_MASK;
	ev_io_init(&ev->io, g_event_cb_io, fd, events);
	ev_timer_init(&ev->timer, g_event_cb_to, .0, .0);

	ev->ev_fd = fd;
	ev->timeout = -1;
	ev->events = events;

	return ev;
}


GEvent *g_event_ref(GEvent *event)
{
	REF_DEBUG_TEST(event);

	g_atomic_int_add(&event->ref_count, 1);
	return event;
}


void g_event_unref(GEvent *event)
{
	REF_DEBUG_TEST(event);

	if (g_atomic_int_dec_and_test(&event->ref_count))
	{
		if (event->finalize)
			(*event->finalize)(event);
		g_free(event);
	}
}


void g_event_set_callback(GEvent *event, GEventDispath dispath,
	void *user_data, GEventFinalize fin)
{
	g_assert(event != NULL);

	event->user_data = user_data;
	event->dispath = dispath;
	event->finalize = fin;
}


static __inline__ gboolean
g_event_ever_added(GEvent *event, GEventLoop *loop)
{
	g_assert(event != NULL && loop != NULL);

	if (event->loop)
		return TRUE;

	event->loop = loop;
	return FALSE;
}


void g_event_set_timeout(GEvent *event, gint timeout)
{
	g_assert(event != NULL);

	if (!event->loop)	/* race against g_event_loop_attach() */
	{
		event->timeout = timeout;
		return;
	}
}


void g_event_mod_timer_sync(GEvent *event, ev_tstamp after)
{
	gint milli_secs;
	g_assert(event != NULL);

	if (G_UNLIKELY(after <= 0))
	{
		after = .000001;
		milli_secs = 1;
	}
	else
	{
		milli_secs = after * 1000;
		if (milli_secs <= 0)
			milli_secs = 1;		
	}

	event->timeout = milli_secs;
	ev_timer_set(&event->timer, .0, after);
	ev_timer_again(event->loop->ev_loop, &event->timer);	
}


ev_tstamp g_event_time_now_sync(GEvent *event)
{
	g_assert(event != NULL);

	return ev_now(event->loop->ev_loop);
}


void g_event_remove(GEvent *event)
{
	g_assert(event != NULL);

	if (event->loop)
		g_event_loop_remove(event->loop, event);
}


static __inline__ gint
g_event_loop_init(GEventLoop *loop)
{
	loop->ref_count = 1;
	loop->lock = g_mutex_new();

	loop->ev_loop = ev_loop_new(EVFLAG_AUTO);
	if (!loop->ev_loop)
	{
		g_mutex_free(loop->lock);
		return -1;
	}

	if (g_event_wakeup_init(&loop->wakeup, loop))
	{
		ev_loop_destroy(loop->ev_loop);
		g_mutex_free(loop->lock);
		return -1;
	}

	loop->loop_running = FALSE;
	loop->loop_quited = FALSE;
	loop->events_list = NULL;
	loop->operations = g_queue_new();

	ev_set_timeout_collect_interval(loop->ev_loop, 0.1);
	ev_set_io_collect_interval(loop->ev_loop, 0.01);    /* 10ms */

	return 0;
}


GEventLoop *g_event_loop_new( void )
{
	GEventLoop *loop;

	loop = g_new0(GEventLoop, 1);
	if (g_event_loop_init(loop))
	{
		g_free(loop);
		return NULL;
	}

	return loop;
}


GEventLoop *g_event_loop_ref(GEventLoop *loop)
{
	return loop;
}


void g_event_loop_unref(GEventLoop *loop)
{
}


void g_event_loop_run(GEventLoop *loop)
{
	gboolean to_run;
	g_assert(loop != NULL);

	g_mutex_lock(loop->lock);

	if (loop->loop_quited)
		to_run = FALSE;
	else
	{
		to_run = !loop->loop_running;
		loop->loop_running = TRUE;
	}

	g_mutex_unlock(loop->lock);

	if (to_run)
		ev_run(loop->ev_loop, 0);
}


static void
g_event_loop_wakeup(GEventLoop *loop)
{
	g_event_block_wakup(&loop->wakeup);
}


static __inline__ void
g_event_loop_sync_add(GEventLoop *loop, GEvent *event)
{
	struct ev_loop *gev_loop;
	ev_tstamp to;
	g_assert(loop != NULL && event != NULL);

	gev_loop = loop->ev_loop;

	if (event->ev_fd >= 0)
	{
		ev_io_init(&event->io, g_event_cb_io, event->ev_fd, event->events);

		if (event->events)
			ev_io_start(gev_loop, &event->io);
	}

	if (event->timeout > 0)
	{
		to = ((ev_tstamp)event->timeout)/1000;
		ev_timer_init(&event->timer, g_event_cb_to, to, to);
		ev_timer_start(gev_loop, &event->timer);
	}

	event->flags |= FLAGS_EVENT_IN_LOOP;
}


static void
g_event_loop_sync_remove(GEventLoop *loop, GEvent *event)
{
	struct ev_loop *gev_loop;
	g_assert(loop != NULL && event != NULL);

	gev_loop = loop->ev_loop;

	if (event->ev_fd >= 0)
		ev_io_stop(gev_loop, &event->io);

	if (event->timeout >= 0)
		ev_timer_stop(gev_loop, &event->timer);

	event->flags &= ~FLAGS_EVENT_IN_LOOP;
}


static void
g_event_loop_sync_io_modified(GEventLoop *loop, GEvent *event, gint type)
{
	struct ev_loop *gev_loop = loop->ev_loop;

	if (!(event->flags  & FLAGS_EVENT_IN_LOOP))
		return;	

	if (type == 0)
	{
		if (event->ev_fd >= 0)
		{
			ev_io_stop(gev_loop, &event->io);

			ev_io_init(&event->io, g_event_cb_io, event->ev_fd, event->events);
			if (event->events)
			{
				ev_io_start(gev_loop, &event->io);
			}
		}
	}
}


void g_event_remove_events_sync(GEvent *event, gint events)
{
	g_assert(event != NULL);

	if (event->events & events)
	{
		event->events &= ~events;
		g_event_loop_sync_io_modified(event->loop, event, 0);
	}
}


void g_event_add_events_sync(GEvent *event, gint events)
{
	g_assert(event != NULL);

	if ((event->events & events) != events)
	{
		event->events |= events;
		g_event_loop_sync_io_modified(event->loop, event, 0);
	}
}


static void
g_event_loop_ops_add(GEventLoop *loop, GLoopOps *ops)
{
	GEvent *event;

	event = ops->event;
	loop->events_list = g_list_prepend(loop->events_list, event);
	g_event_ref(event);
	g_event_loop_sync_add(loop, event);
}


static void
g_event_loop_ops_remove(GEventLoop *loop, GLoopOps *ops)
{
	GList *list;

	list = g_list_find(loop->events_list, ops->event);
	if (list)
	{
		loop->events_list = g_list_delete_link(
			loop->events_list, list
		);
		g_event_loop_sync_remove(loop, ops->event);
		g_event_unref(ops->event);
	}
}


static void
g_event_realse_one(gpointer data_orig, gpointer data_custom)
{
	GEvent *event;
	g_assert(data_orig != NULL);

	event = (GEvent*)data_orig;
	g_event_loop_sync_remove(event->loop, event);
	g_event_unref(event);
}


static void
g_event_loop_ops_modify(GEventLoop *loop, GLoopOps *ops)
{
	g_event_loop_sync_io_modified(loop, ops->event, ops->ops_type);
}


static void
g_event_loop_ops_quit(GEventLoop *loop, GLoopOps *null)
{
	g_list_foreach(
		loop->events_list,
		g_event_realse_one,
		NULL
	);

	g_list_free(loop->events_list);
	loop->events_list = NULL;

	ev_break(loop->ev_loop, EVBREAK_ALL);
}


static __inline__ gboolean
g_event_loop_add(GEventLoop *loop, GEvent *event)
{
	gboolean ret = FALSE;
	GLoopOps *ops;

	g_mutex_lock(loop->lock);

	if (!g_event_ever_added(event, loop))
	{
		if (!loop->loop_quited)
		{
			ops = g_new0(GLoopOps, 1);
			ops->event = event;
			ops->func = g_event_loop_ops_add;
			g_event_ref(event);

			g_queue_push_tail(loop->operations, ops);
			g_event_loop_wakeup(loop);
			ret = TRUE;
		}
	}

	g_mutex_unlock(loop->lock);

	return ret;
}


gboolean
g_event_loop_attach(GEventLoop *loop, GEvent *event)
{
	g_assert(loop != NULL && event != NULL);

	return g_event_loop_add(loop, event);
}


static void
g_event_loop_remove(GEventLoop *loop, GEvent *event)
{
	GLoopOps *ops;
	g_assert(loop != NULL && event != NULL);

	g_mutex_lock(loop->lock);

	if (!loop->loop_quited)
	{
		ops = g_new0(GLoopOps, 1);
		ops->event = event;
		ops->func = g_event_loop_ops_remove;
		g_event_ref(event);

		g_queue_push_tail(loop->operations, ops);
		g_event_loop_wakeup(loop);
	}

	g_mutex_unlock(loop->lock);
}


void g_event_loop_quit(GEventLoop *loop)
{
	GLoopOps *ops;
	g_assert(loop != NULL);

	g_mutex_lock(loop->lock);

	if (!loop->loop_quited)
	{
		ops = g_new0(GLoopOps, 1);
		ops->func = g_event_loop_ops_quit;

		loop->loop_quited = TRUE;
		g_event_loop_wakeup(loop);
	}

	g_mutex_unlock(loop->lock);
}


static __inline__ void
g_event_loop_modify_event(GEventLoop *loop, GEvent *event, gint ops_type)
{
	GLoopOps *ops;

	g_mutex_lock(loop->lock);

	if (!loop->loop_quited)
	{
		ops = g_new0(GLoopOps, 1);
		ops->event = event;
		ops->ops_type = ops_type;
		ops->func = g_event_loop_ops_modify;
		g_event_ref(event);

		g_queue_push_tail(loop->operations, ops);
		g_event_loop_wakeup(loop);
	}

	g_mutex_unlock(loop->lock);
}


void g_event_remove_events(GEvent *event, gint events)
{
	g_assert(event != NULL);

	if (!events)
		return;

	if (event->events & events)					/* fix me */
	{
		event->events &= ~events;
		if (event->loop)
			g_event_loop_modify_event(event->loop, event, 0);
	}
}


void g_event_add_events(GEvent *event, gint events)
{
	g_assert(event != NULL);

	if (!events)
		return;

	if ((event->events & events) != events)		/* fix me */
	{
		event->events |= events;
		if (event->loop)
			g_event_loop_modify_event(event->loop, event, 0);
	}
}


static __inline__ void
g_event_loop_exec_operations(GEventLoop *loop)
{
	GLoopOps *ops;

	while ((ops = g_queue_pop_head(loop->operations)))
	{
		(*ops->func)(loop, ops);
		g_mutex_unlock(loop->lock);

		if (ops->event)
			g_event_unref(ops->event);
		g_free(ops);

		g_mutex_lock(loop->lock);
	}
}


static void
g_event_loop_force_woken_up(GEventLoop *loop)
{
	g_assert(loop != NULL);

	g_mutex_lock(loop->lock);

	g_event_block_woken_up(&loop->wakeup);
	g_event_loop_exec_operations(loop);

	g_mutex_unlock(loop->lock);
}


//:~ End
