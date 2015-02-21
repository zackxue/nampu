/*
 * nmp_scheuler.c
 *
 * This file implements io scheduler.
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
*/

#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmplib.h"
#include "nmp_scheduler.h"
#include "nmp_timer.h"


typedef struct _JWatchLoop JWatchLoop;
struct _JWatchLoop
{
	atomic_t	 watches_count;
    JEventLoop  *loop;
    JThread     *loop_thread;
    JScheduler  *sched;
};


struct _JScheduler
{
	atomic_t	ref_count;
	int			loop_count;
	int			next_loop;
	JWatchLoop  *loops;
};


static JWatchLoop  *timer_loop = NULL;
static JThread *timer_th = NULL;

static void *
j_scheduler_loop_thread(void *user_data)
{
    JEventLoop *loop = (JEventLoop*)user_data;

    j_event_loop_run(loop);
    BUG_ON( TRUE );

    return NULL;
}


static __inline__ void
j_scheduler_init_loop(JWatchLoop *loop, JScheduler *sched)
{
	atomic_set(&loop->watches_count, 0);
	loop->loop = j_event_loop_new();
	loop->loop_thread = j_thread_create(
        j_scheduler_loop_thread,
        loop->loop,
        FALSE,
        NULL
    );
    loop->sched = sched;
}


static __inline__ void
j_scheduler_init_loops(JScheduler *sched, int loop_count)
{
	while (--loop_count >= 0)
	{
		j_scheduler_init_loop(&sched->loops[loop_count], sched);
	}
}


JScheduler *
j_scheduler_new(int loop_count)
{
	JScheduler *sched;

	if (loop_count <= 0)
	{
		printf(
			"Invalid 'loop_count' while creating I/O scheduler\n"
		);
		loop_count = 1;
	}

	sched = j_new0(JScheduler, 1);
	atomic_set(&sched->ref_count, 1);
	sched->loop_count = loop_count;
	sched->loops = j_new0(JWatchLoop, loop_count);
	j_scheduler_init_loops(sched, loop_count);

	return sched;
}


JScheduler *
j_scheduler_ref(JScheduler *sched)
{
	J_ASSERT(sched && atomic_get(&sched->ref_count) > 0);

	atomic_inc(&sched->ref_count);
	return sched;
}


void
j_scheduler_unref(JScheduler *sched)
{
	J_ASSERT(sched && atomic_get(&sched->ref_count) > 0);

	printf("unref scheduler!\n");

	if (atomic_dec_and_test_zero(&sched->ref_count))
	{
		printf("free scheduler!!\n");
	}
}


static __inline__ void
j_scheduler_kill(JScheduler *sched)
{

}


void
j_scheduler_release(JScheduler *sched)
{
	J_ASSERT(sched != NULL);

	j_scheduler_kill(sched);
	j_scheduler_unref(sched);
}


static __inline__ void
j_scheduler_find_best_loop(JScheduler *scheduler)
{
	JWatchLoop *loop;
	int index, best_watches, watches;

	if (scheduler->loop_count == 1)
	{
		scheduler->next_loop = 0;
		return;
	}

	scheduler->next_loop = 0;
	loop = &scheduler->loops[0];
	best_watches = atomic_get(&loop->watches_count);

	for (index = 1; index < scheduler->loop_count; ++index)
	{
		loop = &scheduler->loops[index];
		watches = atomic_get(&loop->watches_count);
		if (watches < best_watches)
		{
			best_watches = watches;
			scheduler->next_loop = index;
		}
	}
}


static void
j_scheduler_remove_io(void *data)
{
	JWatchLoop *loop = (JWatchLoop*)data;
	J_ASSERT(data != NULL);

	atomic_add(&loop->watches_count, -1);
	j_scheduler_unref(loop->sched);
}


static __inline__ void
j_scheduler_add_to_loop(JWatchLoop *loop,  JNetIO *io)
{
	atomic_inc(&loop->watches_count);
	j_scheduler_ref(loop->sched);
	j_net_io_attach(io, loop->loop);
	j_net_io_on_destroy(io, j_scheduler_remove_io, loop);
}


void
j_scheduler_sched_io(JScheduler *sched, JNetIO *io)
{
	J_ASSERT(sched != NULL && io != NULL);

	if (sched->next_loop < 0)
	{
		BUG();
		return;
	}

	j_scheduler_find_best_loop(sched);
	j_scheduler_add_to_loop(&sched->loops[sched->next_loop], io);
}


static __inline__ void
j_scheduler_timer_init( void )
{
	timer_loop = (JWatchLoop*)j_event_loop_new();
    timer_th = j_thread_create(
        j_scheduler_loop_thread,
        timer_loop,
        FALSE,
        NULL
    );
	BUG_ON(!timer_loop);
	BUG_ON(!timer_th);
}


void *j_scheduler_add_timer(int timeout, int (*on_timer)(void*), void *data)
{
	JTimer *timer;

	if (!timer_loop)
	{
		j_scheduler_timer_init();
	}

	timer = j_timer_new(timeout, (JOnTimer)on_timer, data);
	BUG_ON(!timer);
	j_timer_attach((JEventLoop*)timer_loop, timer);

	return timer;
}


void j_scheduler_del_timer(void *handle)
{
	j_timer_del((JTimer*)handle);
}



//:~ End
