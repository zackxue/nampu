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

typedef struct _JLoopOps JLoopOps;
typedef void (*JLoopOpsFunc)(JEventLoop *loop, JLoopOps *ops);

struct _JLoopOps
{
	JEvent *event;
	int ops_type;		/* 0- operation on I/O, 1- operation on timer */
	JLoopOpsFunc func;
};


struct _JEventLoop
{
	atomic_t ref_count;
	JMutex *lock;
	struct ev_loop *ev_loop;
	struct wakeup_block wakeup;
	JBool loop_running;
	JBool loop_quited;
	JList *events_list;
	JQueue *operations;
};

static void j_event_loop_force_woken_up(JEventLoop *loop);
static void j_event_loop_sync_remove(JEventLoop *loop, JEvent *event);
static void j_event_loop_remove(JEventLoop *loop, JEvent *event);

static void
j_event_loop_wakeup_cb(EV_P_ struct ev_io *w, int revents)
{
	JEventLoop *_loop;

	_loop = container_of(w, JEventLoop, wakeup.waker);

	if (revents & EV_READ)
	{
		j_event_loop_force_woken_up(_loop);
		revents &= ~EV_READ;
	}

	BUG_ON(revents);
}


static int
j_event_wakeup_init(struct wakeup_block *wb, JEventLoop *loop)
{
	if (pipe(wb->wakeup_pipe) < 0)
		BUG();

	ev_io_init(&wb->waker, j_event_loop_wakeup_cb, wb->wakeup_pipe[0], EV_READ);
	ev_io_start(loop->ev_loop, &wb->waker);

    return 0;
}


static void
j_event_block_wakup(struct wakeup_block *wb)
{
	J_ASSERT(wb != NULL);

	if (wb->request_pending)
		return;
	wb->request_pending = 1;
#ifndef _WIN32
	write(wb->wakeup_pipe[1], "x", 1);
#else
#endif
}


static __inline__ void
j_event_block_woken_up(struct wakeup_block *wb)
{
	char x;
	J_ASSERT(wb != NULL);

	wb->request_pending = 0;
#ifndef _WIN32
	read(wb->wakeup_pipe[0], &x, 1);
#else
#endif	
}


static JBool
j_event_call(JEvent *ev, int revents)
{
	J_ASSERT(ev != NULL);

	return (*ev->dispath)(ev, revents, ev->user_data);
}


static void
j_event_sync_remove(JEvent *ev)
{
	JEventLoop *loop = ev->loop;
	JList *list;
	BUG_ON(!loop);

	j_mutex_lock(loop->lock);

	if ((list = j_list_find(loop->events_list, ev)))
	{
		loop->events_list = j_list_delete_link(loop->events_list, list);
		j_event_loop_sync_remove(loop, ev);
		j_event_unref(ev);
	}

	j_mutex_unlock(loop->lock);
}


static __inline__ void
j_event_reset_timer(JEvent *ev)
{
	if (ev->timeout > 0)
	{
		ev_timer_again(ev->loop->ev_loop, &ev->timer);
	}
}
static void
j_event_cb_io(EV_P_ struct ev_io *w, int revents)
{
	JEvent *ev = container_of(w, JEvent, io);

	if (!j_event_call(ev, revents))
		j_event_sync_remove(ev);
	else
	  j_event_reset_timer(ev);
}


static void
j_event_cb_to(EV_P_ struct ev_timer *w, int revents)
{
	JEvent *ev = container_of(w, JEvent, timer);

	if (!j_event_call(ev, revents))
		j_event_sync_remove(ev);
}


JEvent *j_event_new(size_t size, int fd, int events)
{
	JEvent *ev;

	if (size < sizeof(JEvent))
		return NULL;

	ev = j_alloc0(size);
	memset(ev, 0, size);
	atomic_set(&ev->ref_count, 1);
	ev->event_size = size;

	events &= EVENTS_MASK;
	ev_io_init(&ev->io, j_event_cb_io, fd, events);
	ev_timer_init(&ev->timer, j_event_cb_to, .0, .0);

	ev->ev_fd = fd;
	ev->timeout = -1;
	ev->events = events;

	return ev;
}


JEvent *j_event_ref(JEvent *event)
{
	REF_DEBUG_TEST(event);
	atomic_inc(&event->ref_count);
	return event;
}


void j_event_unref(JEvent *event)
{
	REF_DEBUG_TEST(event);
	if (atomic_dec_and_test_zero(&event->ref_count))
	{
		if (event->destroy)
			(*event->destroy)(event);
		j_dealloc(event, event->event_size);
	}
}


void j_event_set_callback(JEvent *event, JEventDispath dispath,
	void *user_data, JEventOnDestroy notify)
{
	J_ASSERT(event != NULL);

	event->user_data = user_data;
	event->dispath = dispath;
	event->destroy = notify;
}


static  __inline__ JBool
j_event_ever_added(JEvent *event, JEventLoop *loop)
{
	J_ASSERT(event != NULL && loop != NULL);

	if (event->loop)
		return TRUE;

	event->loop = loop;
	return FALSE;
}


void j_event_set_timeout(JEvent *event, int timeout)
{
	J_ASSERT(event != NULL);

	if (!event->loop)	/* race against j_event_loop_attach() */
	{
		event->timeout = timeout;
		return;
	}
}


void j_event_mod_timer_sync(JEvent *event, ev_tstamp after)
{
	int milli_secs;
	J_ASSERT(event != NULL);

	if (J_UNLIKELY(after <= 0))
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

ev_tstamp j_event_time_now_sync(JEvent *event)
{
	J_ASSERT(event != NULL);

	return ev_now(event->loop->ev_loop);
}

void j_event_remove(JEvent *event)
{
	J_ASSERT(event != NULL);

	if (event->loop)
		j_event_loop_remove(event->loop, event);
}


static __inline__ int
j_event_loop_init(JEventLoop *loop)
{
	atomic_set(&loop->ref_count, 1);
	loop->lock = j_mutex_new();
	loop->ev_loop = ev_loop_new(EVFLAG_AUTO);
	//BUG_ON(!loop->ev_loop);
	if (!loop->ev_loop)
	{
		j_mutex_free(loop->lock);
		return -1;
	}
	if (j_event_wakeup_init(&loop->wakeup, loop))
	{
		ev_loop_destroy(loop->ev_loop);
		j_mutex_free(loop->lock);
		return -1;
	}
	loop->loop_running = FALSE;
	loop->loop_quited = FALSE;
	loop->events_list = NULL;
	loop->operations = j_queue_new();
	return 0;
}

JEventLoop *j_event_loop_new( void )
{
	JEventLoop *loop;

	loop = j_new0(JEventLoop, 1);
	if (j_event_loop_init(loop))
	{
		j_del(loop, JEventLoop, 1);
		return NULL;
	}
    
	return loop;
}

JEventLoop *j_event_loop_ref(JEventLoop *loop)
{
	return loop;
}


void j_event_loop_unref(JEventLoop *loop)
{
}

void j_event_loop_run(JEventLoop *loop)
{
	JBool to_run;
	J_ASSERT(loop != NULL);

	j_mutex_lock(loop->lock);

	if (loop->loop_quited)
		to_run = FALSE;
	else
	{
		to_run = !loop->loop_running;
		loop->loop_running = TRUE;
	}

	j_mutex_unlock(loop->lock);

	if (to_run)
		ev_run(loop->ev_loop, 0);
}


static void
j_event_loop_wakeup(JEventLoop *loop)
{
	j_event_block_wakup(&loop->wakeup);
}


static __inline__ void
j_event_loop_sync_add(JEventLoop *loop, JEvent *event)
{
	struct ev_loop *_loop;
	ev_tstamp to;
	J_ASSERT(loop != NULL && event != NULL);

	_loop = loop->ev_loop;
	BUG_ON(!_loop);

	if (event->ev_fd >= 0)
	{
//		printf("new event added!!\n");
		ev_io_init(&event->io, j_event_cb_io, event->ev_fd, event->events);

		if (event->events)
			ev_io_start(_loop, &event->io);
	}

	if (event->timeout >= 0)
	{
		//ev_timer_init(), will be started 500 secs later.
		to = ((ev_tstamp)event->timeout)/1000;
		ev_timer_init(&event->timer, j_event_cb_to, to, to);
		ev_timer_start(_loop, &event->timer);
	}

	event->flags |= FLAGS_EVENT_IN_LOOP;
}


static void
j_event_loop_sync_remove(JEventLoop *loop, JEvent *event)
{
	struct ev_loop *_loop;
	J_ASSERT(loop != NULL && event != NULL);

	_loop = loop->ev_loop;
	BUG_ON(!_loop);

	if (event->ev_fd >= 0)
		ev_io_stop(_loop, &event->io);

	if (event->timeout >= 0)
		ev_timer_stop(_loop, &event->timer);

	event->flags &= ~FLAGS_EVENT_IN_LOOP;
}


static void
j_event_loop_sync_io_modified(JEventLoop *loop, JEvent *event, int type)
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

			ev_io_init(&event->io, j_event_cb_io, event->ev_fd, event->events);
			if (event->events)
			{
//		        printf("event restart:events:%d\n", event->events);
				ev_io_start(_loop, &event->io);
			}
		}
	}
}
void j_event_remove_events_sync(JEvent *event, int events)
{
	J_ASSERT(event != NULL);

	if (event->events & events)
	{
		event->events &= ~events;
		j_event_loop_sync_io_modified(event->loop, event, 0);
	}
}
void j_event_add_events_sync(JEvent *event, int events)
{
	J_ASSERT(event != NULL);

	if ((event->events & events) != events)
	{
		event->events |= events;
		j_event_loop_sync_io_modified(event->loop, event, 0);
	}
}


static void
j_event_loop_ops_add(JEventLoop *loop, JLoopOps *ops)
{
	JEvent *event;

	event = ops->event;
	loop->events_list = j_list_add(loop->events_list, event);
	j_event_ref(event);
	j_event_loop_sync_add(loop, event);
}


static void
j_event_loop_ops_remove(JEventLoop *loop, JLoopOps *ops)
{
	JList *list;

	list = j_list_find(loop->events_list, ops->event);
	if (list)
	{
		loop->events_list = j_list_delete_link(
			loop->events_list, list
		);
		j_event_loop_sync_remove(loop, ops->event);
		j_event_unref(ops->event);
	}
}


static void
j_event_realse_one(void *data_orig, void *data_custom)
{
	JEvent *event;
	J_ASSERT(data_orig != NULL);

	event = (JEvent*)data_orig;
	BUG_ON(!event->loop);
	j_event_loop_sync_remove(event->loop, event);
	j_event_unref(event);
}


static void
j_event_loop_ops_modify(JEventLoop *loop, JLoopOps *ops)
{//printf("\n@@@@@@@@@@@@@@@@@@@@ modify events:%d ###################\n", ops->event->events);
	j_event_loop_sync_io_modified(loop, ops->event, ops->ops_type);
}


static void
j_event_loop_ops_quit(JEventLoop *loop, JLoopOps *null)
{
	j_list_foreach(
		loop->events_list,
		j_event_realse_one,
		NULL
	);

	j_list_free(loop->events_list);
	loop->events_list = NULL;

	ev_break(loop->ev_loop, EVBREAK_ALL);
}


static __inline__ JBool
j_event_loop_add(JEventLoop *loop, JEvent *event)
{
	JBool ret = FALSE;
	JLoopOps *ops;

	j_mutex_lock(loop->lock);

	if (j_event_ever_added(event, loop))
		BUG();

	if (!loop->loop_quited)
	{
		ops = j_new0(JLoopOps, 1);
		ops->event = event;
		ops->func = j_event_loop_ops_add;
		j_event_ref(event);

		j_queue_push_tail(loop->operations, ops);
		j_event_loop_wakeup(loop);
		ret = TRUE;
	}

	j_mutex_unlock(loop->lock);

	return ret;
}


JBool j_event_loop_attach(JEventLoop *loop, JEvent *event)
{
	J_ASSERT(loop != NULL && event != NULL);

	return j_event_loop_add(loop, event);
}


static void
j_event_loop_remove(JEventLoop *loop, JEvent *event)
{
	JLoopOps *ops;
	J_ASSERT(loop != NULL && event != NULL);

	j_mutex_lock(loop->lock);

	if (event->loop != loop)
		BUG();

	if (!loop->loop_quited)
	{
		ops = j_new0(JLoopOps, 1);
		ops->event = event;
		ops->func = j_event_loop_ops_remove;
		j_event_ref(event);

		j_queue_push_tail(loop->operations, ops);
		j_event_loop_wakeup(loop);
	}

	j_mutex_unlock(loop->lock);
}


void j_event_loop_quit(JEventLoop *loop)
{
	JLoopOps *ops;
	J_ASSERT(loop != NULL);

	j_mutex_lock(loop->lock);

	if (!loop->loop_quited)
	{
		ops = j_new0(JLoopOps, 1);
		ops->func = j_event_loop_ops_quit;

		loop->loop_quited = TRUE;
		j_event_loop_wakeup(loop);
	}

	j_mutex_unlock(loop->lock);
}


static __inline__ void
j_event_loop_modify_event(JEventLoop *loop, JEvent *event, int ops_type)
{
	JLoopOps *ops;

	j_mutex_lock(loop->lock);

	if (!loop->loop_quited)
	{
		ops = j_new0(JLoopOps, 1);
		ops->event = event;
		ops->ops_type = ops_type;
		ops->func = j_event_loop_ops_modify;
		j_event_ref(event);

		j_queue_push_tail(loop->operations, ops);
		j_event_loop_wakeup(loop);
	}

	j_mutex_unlock(loop->lock);
}


void j_event_remove_events(JEvent *event, int events)
{
	J_ASSERT(event != NULL);

	if (!events)
		return;

	if (event->events & events)					/* fix me */
	{
		event->events &= ~events;
		if (event->loop)
			j_event_loop_modify_event(event->loop, event, 0);
	}
}


void j_event_add_events(JEvent *event, int events)
{
	J_ASSERT(event != NULL);

	if (!events)
		return;

	if ((event->events & events) != events)		/* fix me */
	{
		event->events |= events;
		if (event->loop)
			j_event_loop_modify_event(event->loop, event, 0);
	}
}


static __inline__ void
j_event_loop_exec_operations(JEventLoop *loop)
{
	JLoopOps *ops;

	while ((ops = j_queue_pop_head(loop->operations)))
	{
		(*ops->func)(loop, ops);
		j_mutex_unlock(loop->lock);

		if (ops->event)
			j_event_unref(ops->event);
		j_del(ops, JLoopOps, 1);
		j_mutex_lock(loop->lock);
	}
}


static void
j_event_loop_force_woken_up(JEventLoop *loop)
{
	J_ASSERT(loop != NULL);

	j_mutex_lock(loop->lock);

	j_event_block_woken_up(&loop->wakeup);
	j_event_loop_exec_operations(loop);

	j_mutex_unlock(loop->lock);
}


//:~ End
