#include <string.h>
#include <unistd.h>
#include "nmp_macros.h"
#include "nmp_event.h"
#include "nmp_thread.h"
#include "nmp_mem.h"
#include "nmp_queue.h"

#define CONTAINER_OF(ptr, struct_type, member) \
	(struct_type*)((char*)(ptr) - G_STRUCT_OFFSET(struct_type, member));

#define FLAGS_EVENT_IN_LOOP				0x01

struct wakeup_block		/* for loop waking up */
{
	int request_pending;
	struct ev_io waker;
#ifndef _WIN32
	int wakeup_pipe[2];
#else
	HANDLE wakeup_sem;
#endif
};

typedef struct _nmp_loop_ops nmp_loop_ops_t;
typedef void (*nmp_loop_ops_func)(nmp_event_loop_t *loop, nmp_loop_ops_t *ops);

struct _nmp_loop_ops
{
	nmp_event_t *event;
	int ops_type;		/* 0- operation on I/O, 1- operation on timer */
	nmp_loop_ops_func func;
};


struct _nmp_event_loop
{
	atomic_t ref_count;
	nmp_mutex_t *lock;
	struct ev_loop *ev_loop;
	struct wakeup_block wakeup;
	nmp_bool_t loop_running;
	nmp_bool_t loop_quited;
	nmp_list_t *events_list;
	nmp_queue_t *operations;
};

static void nmp_event_loop_force_woken_up(nmp_event_loop_t *loop);
static void nmp_event_loop_sync_remove(nmp_event_loop_t *loop, nmp_event_t *event);
static void nmp_event_loop_remove(nmp_event_loop_t *loop, nmp_event_t *event);

static void
nmp_event_loop_wakeup_cb(EV_P_ struct ev_io *w, int revents)
{
	nmp_event_loop_t *_loop;

	_loop = container_of(w, nmp_event_loop_t, wakeup.waker);

	if (revents & EV_READ)
	{
		nmp_event_loop_force_woken_up(_loop);
		revents &= ~EV_READ;
	}

	BUG_ON(revents);
}


static int
nmp_event_wakeup_init(struct wakeup_block *wb, nmp_event_loop_t *loop)
{
	if (pipe(wb->wakeup_pipe) < 0)
		BUG();

	ev_io_init(&wb->waker, nmp_event_loop_wakeup_cb, wb->wakeup_pipe[0], EV_READ);
	ev_io_start(loop->ev_loop, &wb->waker);

    return 0;
}


static void
nmp_event_block_wakup(struct wakeup_block *wb)
{
	NMP_ASSERT(wb != NULL);

	if (wb->request_pending)
		return;
	wb->request_pending = 1;
#ifndef _WIN32
	write(wb->wakeup_pipe[1], "x", 1);
#else
#endif
}


static __inline__ void
nmp_event_block_woken_up(struct wakeup_block *wb)
{
	char x;
	NMP_ASSERT(wb != NULL);

	wb->request_pending = 0;
#ifndef _WIN32
	read(wb->wakeup_pipe[0], &x, 1);
#else
#endif	
}


static nmp_bool_t
nmp_event_call(nmp_event_t *ev, int revents)
{
	NMP_ASSERT(ev != NULL);

	return (*ev->dispath)(ev, revents, ev->user_data);
}


static void
nmp_event_sync_remove(nmp_event_t *ev)
{
	nmp_event_loop_t *loop = ev->loop;
	nmp_list_t *list;
	BUG_ON(!loop);

	nmp_mutex_lock(loop->lock);

	if ((list = nmp_list_find(loop->events_list, ev)))
	{
		loop->events_list = nmp_list_delete_link(loop->events_list, list);
		nmp_event_loop_sync_remove(loop, ev);
		nmp_event_unref(ev);
	}

	nmp_mutex_unlock(loop->lock);
}


static __inline__ void
nmp_event_reset_timer(nmp_event_t *ev)
{
	if (ev->timeout > 0)
	{
		ev_timer_again(ev->loop->ev_loop, &ev->timer);
	}
}
static void
nmp_event_cb_io(EV_P_ struct ev_io *w, int revents)
{
	nmp_event_t *ev = container_of(w, nmp_event_t, io);

	if (!nmp_event_call(ev, revents))
		nmp_event_sync_remove(ev);
	else
	  nmp_event_reset_timer(ev);
}


static void
nmp_event_cb_to(EV_P_ struct ev_timer *w, int revents)
{
	nmp_event_t *ev = container_of(w, nmp_event_t, timer);

	if (!nmp_event_call(ev, revents))
		nmp_event_sync_remove(ev);
}


nmp_event_t *nmp_event_new(size_t size, int fd, int events)
{
	nmp_event_t *ev;

	if (size < sizeof(nmp_event_t))
		return NULL;

	ev = nmp_alloc0(size);
	memset(ev, 0, size);
	atomic_set(&ev->ref_count, 1);
	ev->event_size = size;

	events &= EVENTS_MASK;
	ev_io_init(&ev->io, nmp_event_cb_io, fd, events);
	ev_timer_init(&ev->timer, nmp_event_cb_to, .0, .0);

	ev->ev_fd = fd;
	ev->timeout = -1;
	ev->events = events;

	return ev;
}


nmp_event_t *nmp_event_ref(nmp_event_t *event)
{
	REF_DEBUG_TEST(event);
	atomic_inc(&event->ref_count);
	return event;
}


void nmp_event_unref(nmp_event_t *event)
{
	REF_DEBUG_TEST(event);
	if (atomic_dec_and_test_zero(&event->ref_count))
	{
		if (event->destroy)
			(*event->destroy)(event);
		nmp_dealloc(event, event->event_size);
	}
}


void nmp_event_set_callback(nmp_event_t *event, nmp_event_dispatch dispath,
	void *user_data, nmp_event_on_destroy notify)
{
	NMP_ASSERT(event != NULL);

	event->user_data = user_data;
	event->dispath = dispath;
	event->destroy = notify;
}


static  __inline__ nmp_bool_t
nmp_event_ever_added(nmp_event_t *event, nmp_event_loop_t *loop)
{
	NMP_ASSERT(event != NULL && loop != NULL);

	if (event->loop)
		return TRUE;

	event->loop = loop;
	return FALSE;
}


void nmp_event_set_timeout(nmp_event_t *event, int timeout)
{
	NMP_ASSERT(event != NULL);

	if (!event->loop)	/* race against nmp_event_loop_attach() */
	{
		event->timeout = timeout;
		return;
	}
}


void nmp_event_mod_timer_sync(nmp_event_t *event, ev_tstamp after)
{
	int milli_secs;
	NMP_ASSERT(event != NULL);

	if (NMP_UNLIKELY(after <= 0))
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

ev_tstamp nmp_event_time_now_sync(nmp_event_t *event)
{
	NMP_ASSERT(event != NULL);

	return ev_now(event->loop->ev_loop);
}

void nmp_event_remove(nmp_event_t *event)
{
	NMP_ASSERT(event != NULL);

	if (event->loop)
		nmp_event_loop_remove(event->loop, event);
}


static __inline__ int
nmp_event_loop_init(nmp_event_loop_t *loop)
{
	atomic_set(&loop->ref_count, 1);
	loop->lock = nmp_mutex_new();
	loop->ev_loop = ev_loop_new(EVFLAG_AUTO);
	//BUG_ON(!loop->ev_loop);
	if (!loop->ev_loop)
	{
		nmp_mutex_free(loop->lock);
		return -1;
	}
	if (nmp_event_wakeup_init(&loop->wakeup, loop))
	{
		ev_loop_destroy(loop->ev_loop);
		nmp_mutex_free(loop->lock);
		return -1;
	}
	loop->loop_running = FALSE;
	loop->loop_quited = FALSE;
	loop->events_list = NULL;
	loop->operations = nmp_queue_new();
	return 0;
}

nmp_event_loop_t *nmp_event_loop_new( void )
{
	nmp_event_loop_t *loop;

	loop = nmp_new0(nmp_event_loop_t, 1);
	if (nmp_event_loop_init(loop))
	{
		nmp_del(loop, nmp_event_loop_t, 1);
		return NULL;
	}
    
	return loop;
}

nmp_event_loop_t *nmp_event_loop_ref(nmp_event_loop_t *loop)
{
	return loop;
}


void nmp_event_loop_unref(nmp_event_loop_t *loop)
{
}

void nmp_event_loop_run(nmp_event_loop_t *loop)
{
	nmp_bool_t to_run;
	NMP_ASSERT(loop != NULL);

	nmp_mutex_lock(loop->lock);

	if (loop->loop_quited)
		to_run = FALSE;
	else
	{
		to_run = !loop->loop_running;
		loop->loop_running = TRUE;
	}

	nmp_mutex_unlock(loop->lock);

	if (to_run)
		ev_run(loop->ev_loop, 0);
}


static void
nmp_event_loop_wakeup(nmp_event_loop_t *loop)
{
	nmp_event_block_wakup(&loop->wakeup);
}


static __inline__ void
nmp_event_loop_sync_add(nmp_event_loop_t *loop, nmp_event_t *event)
{
	struct ev_loop *_loop;
	ev_tstamp to;
	NMP_ASSERT(loop != NULL && event != NULL);

	_loop = loop->ev_loop;
	BUG_ON(!_loop);

	if (event->ev_fd >= 0)
	{
//		printf("new event added!!\n");
		ev_io_init(&event->io, nmp_event_cb_io, event->ev_fd, event->events);

		if (event->events)
			ev_io_start(_loop, &event->io);
	}

	if (event->timeout >= 0)
	{
		//ev_timer_init(), will be started 500 secs later.
		to = ((ev_tstamp)event->timeout)/1000;
		ev_timer_init(&event->timer, nmp_event_cb_to, to, to);
		ev_timer_start(_loop, &event->timer);
	}

	event->flags |= FLAGS_EVENT_IN_LOOP;
}


static void
nmp_event_loop_sync_remove(nmp_event_loop_t *loop, nmp_event_t *event)
{
	struct ev_loop *_loop;
	NMP_ASSERT(loop != NULL && event != NULL);

	_loop = loop->ev_loop;
	BUG_ON(!_loop);

	if (event->ev_fd >= 0)
		ev_io_stop(_loop, &event->io);

	if (event->timeout >= 0)
		ev_timer_stop(_loop, &event->timer);

	event->flags &= ~FLAGS_EVENT_IN_LOOP;
}


static void
nmp_event_loop_sync_io_modified(nmp_event_loop_t *loop, nmp_event_t *event, int type)
{
	struct ev_loop *_loop;

	_loop = loop->ev_loop;
	BUG_ON(!_loop);

	if (!(event->flags  & FLAGS_EVENT_IN_LOOP))
		return;

	if (type == 0)
	{
		if (event->ev_fd >= 0)
		{
			ev_io_stop(_loop, &event->io);

			ev_io_init(&event->io, nmp_event_cb_io, event->ev_fd, event->events);
			if (event->events)
			{
//		        printf("event restart:events:%d\n", event->events);
				ev_io_start(_loop, &event->io);
			}
		}
	}
}
void nmp_event_remove_events_sync(nmp_event_t *event, int events)
{
	NMP_ASSERT(event != NULL);

	if (event->events & events)
	{
		event->events &= ~events;
		nmp_event_loop_sync_io_modified(event->loop, event, 0);
	}
}
void nmp_event_add_events_sync(nmp_event_t *event, int events)
{
	NMP_ASSERT(event != NULL);

	if ((event->events & events) != events)
	{
		event->events |= events;
		nmp_event_loop_sync_io_modified(event->loop, event, 0);
	}
}


static void
nmp_event_loop_ops_add(nmp_event_loop_t *loop, nmp_loop_ops_t *ops)
{
	nmp_event_t *event;

	event = ops->event;
	loop->events_list = nmp_list_add(loop->events_list, event);
	nmp_event_ref(event);
	nmp_event_loop_sync_add(loop, event);
}


static void
nmp_event_loop_ops_remove(nmp_event_loop_t *loop, nmp_loop_ops_t *ops)
{
	nmp_list_t *list;

	list = nmp_list_find(loop->events_list, ops->event);
	if (list)
	{
		loop->events_list = nmp_list_delete_link(
			loop->events_list, list
		);
		nmp_event_loop_sync_remove(loop, ops->event);
		nmp_event_unref(ops->event);
	}
}


static void
nmp_event_realse_one(void *data_orig, void *data_custom)
{
	nmp_event_t *event;
	NMP_ASSERT(data_orig != NULL);

	event = (nmp_event_t*)data_orig;
	BUG_ON(!event->loop);
	nmp_event_loop_sync_remove(event->loop, event);
	nmp_event_unref(event);
}


static void
nmp_event_loop_ops_modify(nmp_event_loop_t *loop, nmp_loop_ops_t *ops)
{//printf("\n@@@@@@@@@@@@@@@@@@@@ modify events:%d ###################\n", ops->event->events);
	nmp_event_loop_sync_io_modified(loop, ops->event, ops->ops_type);
}


static void
nmp_event_loop_ops_quit(nmp_event_loop_t *loop, nmp_loop_ops_t *null)
{
	nmp_list_foreach(
		loop->events_list,
		nmp_event_realse_one,
		NULL
	);

	nmp_list_free(loop->events_list);
	loop->events_list = NULL;

	ev_break(loop->ev_loop, EVBREAK_ALL);
}


static __inline__ nmp_bool_t
nmp_event_loop_add(nmp_event_loop_t *loop, nmp_event_t *event)
{
	nmp_bool_t ret = FALSE;
	nmp_loop_ops_t *ops;

	nmp_mutex_lock(loop->lock);

	if (nmp_event_ever_added(event, loop))
		BUG();

	if (!loop->loop_quited)
	{
		ops = nmp_new0(nmp_loop_ops_t, 1);
		ops->event = event;
		ops->func = nmp_event_loop_ops_add;
		nmp_event_ref(event);

		nmp_queue_push_tail(loop->operations, ops);
		nmp_event_loop_wakeup(loop);
		ret = TRUE;
	}

	nmp_mutex_unlock(loop->lock);

	return ret;
}


nmp_bool_t nmp_event_loop_attach(nmp_event_loop_t *loop, nmp_event_t *event)
{
	NMP_ASSERT(loop != NULL && event != NULL);

	return nmp_event_loop_add(loop, event);
}


static void
nmp_event_loop_remove(nmp_event_loop_t *loop, nmp_event_t *event)
{
	nmp_loop_ops_t *ops;
	NMP_ASSERT(loop != NULL && event != NULL);

	nmp_mutex_lock(loop->lock);

	if (event->loop != loop)
		BUG();

	if (!loop->loop_quited)
	{
		ops = nmp_new0(nmp_loop_ops_t, 1);
		ops->event = event;
		ops->func = nmp_event_loop_ops_remove;
		nmp_event_ref(event);

		nmp_queue_push_tail(loop->operations, ops);
		nmp_event_loop_wakeup(loop);
	}

	nmp_mutex_unlock(loop->lock);
}


void nmp_event_loop_quit(nmp_event_loop_t *loop)
{
	nmp_loop_ops_t *ops;
	NMP_ASSERT(loop != NULL);

	nmp_mutex_lock(loop->lock);

	if (!loop->loop_quited)
	{
		ops = nmp_new0(nmp_loop_ops_t, 1);
		ops->func = nmp_event_loop_ops_quit;

		loop->loop_quited = TRUE;
		nmp_event_loop_wakeup(loop);
	}

	nmp_mutex_unlock(loop->lock);
}


static __inline__ void
nmp_event_loop_modify_event(nmp_event_loop_t *loop, nmp_event_t *event, int ops_type)
{
	nmp_loop_ops_t *ops;

	nmp_mutex_lock(loop->lock);

	if (!loop->loop_quited)
	{
		ops = nmp_new0(nmp_loop_ops_t, 1);
		ops->event = event;
		ops->ops_type = ops_type;
		ops->func = nmp_event_loop_ops_modify;
		nmp_event_ref(event);

		nmp_queue_push_tail(loop->operations, ops);
		nmp_event_loop_wakeup(loop);
	}

	nmp_mutex_unlock(loop->lock);
}


void nmp_event_remove_events(nmp_event_t *event, int events)
{
	NMP_ASSERT(event != NULL);

	if (!events)
		return;

	if (event->events & events)					/* fix me */
	{
		event->events &= ~events;
		if (event->loop)
			nmp_event_loop_modify_event(event->loop, event, 0);
	}
}


void nmp_event_add_events(nmp_event_t *event, int events)
{
	NMP_ASSERT(event != NULL);

	if (!events)
		return;

	if ((event->events & events) != events)		/* fix me */
	{
		event->events |= events;
		if (event->loop)
			nmp_event_loop_modify_event(event->loop, event, 0);
	}
}


static __inline__ void
nmp_event_loop_exec_operations(nmp_event_loop_t *loop)
{
	nmp_loop_ops_t *ops;

	while ((ops = nmp_queue_pop_head(loop->operations)))
	{
		(*ops->func)(loop, ops);
		nmp_mutex_unlock(loop->lock);

		if (ops->event)
			nmp_event_unref(ops->event);
		nmp_del(ops, nmp_loop_ops_t, 1);
		nmp_mutex_lock(loop->lock);
	}
}


static void
nmp_event_loop_force_woken_up(nmp_event_loop_t *loop)
{
	NMP_ASSERT(loop != NULL);

	nmp_mutex_lock(loop->lock);

	nmp_event_block_woken_up(&loop->wakeup);
	nmp_event_loop_exec_operations(loop);

	nmp_mutex_unlock(loop->lock);
}


//:~ End
