#include "timer.h"

struct __Timer
{
	JEvent base;
	OnTimer on_timer;
	OnDel on_del;
	void *data;
};

typedef struct __TimerLoop TimerLoop;
struct __TimerLoop
{
	JEventLoop	*loop;
	JThread		*loop_thread;
};

static TimerLoop *timer_loop = NULL;


static JBool
on_timer_internal(JEvent *ev, int32_t revents, void *user_data)
{
	Timer *timer = (Timer*)ev;

	return (*timer->on_timer)(timer, timer->data) == TIMER_CONT ? TRUE : FALSE;
}


static void
on_timer_del(JEvent *ev)
{
	Timer *t = (Timer*)ev;

	if (t->on_del)
	{
		(*t->on_del)(t->data);
	}
}


static __inline__ Timer *
timer_new(int32_t timeout, OnTimer on_timer, OnDel on_del, void *data)
{
	Timer *timer;

	timer = (Timer*)j_event_new(sizeof(Timer), -1, 0);
	timer->on_timer = on_timer;
	timer->on_del = on_del;
	timer->data = data;

	j_event_set_callback((JEvent*)timer,
	                     on_timer_internal,
	                     NULL,
	                     on_timer_del);
	j_event_set_timeout((JEvent*)timer, timeout);

	return timer;
}


static __inline__ int32_t
timer_attach(JEventLoop *loop, Timer *timer)
{
	J_ASSERT(loop != NULL);

	return !j_event_loop_attach(loop, (JEvent*)timer);
}


static __inline__ void
timer_del(Timer *timer)
{
	j_event_remove((JEvent*)timer);
}


static void *
timer_loop_thread(void *user_data)
{
    JEventLoop *loop = (JEventLoop*)user_data;

    j_event_loop_run(loop);
    BUG_ON( TRUE );

    return NULL;
}


static __inline__ TimerLoop *
alloc_timer_loop( void )
{
	TimerLoop *l = j_alloc0(sizeof(*l));

	if (l)
	{
		l->loop = j_event_loop_new();
		l->loop_thread =j_thread_create(timer_loop_thread,
			                              l->loop,
			                              FALSE,
			                              NULL);
	}

	return l;
}


int32_t
init_timer_facility( void )
{
	if (!timer_loop)
	{
		timer_loop = alloc_timer_loop();
	}

	return 0;
}


Timer*
set_timer(int32_t timeout, OnTimer on_timer, OnDel on_del, void *data)
{
	Timer *t = NULL;

	if (timer_loop)
	{
		t = timer_new(timeout, on_timer, on_del, data);
		if (timer_attach(timer_loop->loop, t))
		{
			j_event_unref((JEvent*)t);
			t = NULL;
		}
	}

	return t;
}


void
mod_timer(Timer *timer, int32_t milli_secs)
{
	ev_tstamp sleep = ((ev_tstamp)milli_secs) / 1000;
	j_event_mod_timer_sync((JEvent*)timer, sleep);
}


void
del_timer(Timer *timer)
{
	j_event_remove((JEvent*)timer);
	j_event_unref((JEvent*)timer);
}


//:~ End
