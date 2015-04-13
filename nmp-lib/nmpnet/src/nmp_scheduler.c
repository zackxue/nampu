/*
 * nmp_scheuler.c
 *
 * This file implements io scheduler.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmplib.h"
#include "nmp_scheduler.h"
#include "nmp_timer.h"


typedef struct _nmp_watch_loop nmp_watch_loop_t;
struct _nmp_watch_loop
{
	atomic_t			watches_count;
    nmp_event_loop_t 	*loop;
    nmp_thread_t		*loop_thread;
    nmp_scheduler_t		*sched;
};


struct _nmp_scheduler
{
	atomic_t	ref_count;
	int			loop_count;
	int			next_loop;
	nmp_watch_loop_t  *loops;
};


static nmp_watch_loop_t  *timer_loop = NULL;
static nmp_thread_t *timer_th = NULL;

static void *
nmp_scheduler_loop_thread(void *user_data)
{
    nmp_event_loop_t *loop = (nmp_event_loop_t*)user_data;

    nmp_event_loop_run(loop);
    BUG_ON( TRUE );

    return NULL;
}


static __inline__ void
nmp_scheduler_init_loop(nmp_watch_loop_t *loop, nmp_scheduler_t *sched)
{
	atomic_set(&loop->watches_count, 0);
	loop->loop = nmp_event_loop_new();
	loop->loop_thread = nmp_thread_create(
        nmp_scheduler_loop_thread,
        loop->loop,
        FALSE,
        NULL
    );
    loop->sched = sched;
}


static __inline__ void
nmp_scheduler_init_loops(nmp_scheduler_t *sched, int loop_count)
{
	while (--loop_count >= 0)
	{
		nmp_scheduler_init_loop(&sched->loops[loop_count], sched);
	}
}


nmp_scheduler_t *
nmp_scheduler_new(int loop_count)
{
	nmp_scheduler_t *sched;

	if (loop_count <= 0)
	{
		printf(
			"Invalid 'loop_count' while creating I/O scheduler\n"
		);
		loop_count = 1;
	}

	sched = nmp_new0(nmp_scheduler_t, 1);
	atomic_set(&sched->ref_count, 1);
	sched->loop_count = loop_count;
	sched->loops = nmp_new0(nmp_watch_loop_t, loop_count);
	nmp_scheduler_init_loops(sched, loop_count);

	return sched;
}


nmp_scheduler_t *
nmp_scheduler_ref(nmp_scheduler_t *sched)
{
	NMP_ASSERT(sched && atomic_get(&sched->ref_count) > 0);

	atomic_inc(&sched->ref_count);
	return sched;
}


void
nmp_scheduler_unref(nmp_scheduler_t *sched)
{
	NMP_ASSERT(sched && atomic_get(&sched->ref_count) > 0);

	printf("unref scheduler!\n");

	if (atomic_dec_and_test_zero(&sched->ref_count))
	{
		printf("free scheduler!!\n");
	}
}


static __inline__ void
nmp_scheduler_kill(nmp_scheduler_t *sched)
{

}


void
nmp_scheduler_release(nmp_scheduler_t *sched)
{
	NMP_ASSERT(sched != NULL);

	nmp_scheduler_kill(sched);
	nmp_scheduler_unref(sched);
}


static __inline__ void
nmp_scheduler_find_best_loop(nmp_scheduler_t *scheduler)
{
	nmp_watch_loop_t *loop;
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
nmp_scheduler_remove_io(void *data)
{
	nmp_watch_loop_t *loop = (nmp_watch_loop_t*)data;
	NMP_ASSERT(data != NULL);

	atomic_add(&loop->watches_count, -1);
	nmp_scheduler_unref(loop->sched);
}


static __inline__ void
nmp_scheduler_add_to_loop(nmp_watch_loop_t *loop,  nmp_netio_t *io)
{
	atomic_inc(&loop->watches_count);
	nmp_scheduler_ref(loop->sched);
	nmp_net_io_attach(io, loop->loop);
	nmp_net_io_on_destroy(io, nmp_scheduler_remove_io, loop);
}


void
nmp_scheduler_sched_io(nmp_scheduler_t *sched, nmp_netio_t *io)
{
	NMP_ASSERT(sched != NULL && io != NULL);

	if (sched->next_loop < 0)
	{
		BUG();
		return;
	}

	nmp_scheduler_find_best_loop(sched);
	nmp_scheduler_add_to_loop(&sched->loops[sched->next_loop], io);
}


static __inline__ void
nmp_scheduler_timer_init( void )
{
	timer_loop = (nmp_watch_loop_t*)nmp_event_loop_new();
    timer_th = nmp_thread_create(
        nmp_scheduler_loop_thread,
        timer_loop,
        FALSE,
        NULL
    );
	BUG_ON(!timer_loop);
	BUG_ON(!timer_th);
}


void *nmp_scheduler_add_timer(int timeout, int (*on_timer)(void*), void *data)
{
	nmp_timer_t *timer;

	if (!timer_loop)
	{
		nmp_scheduler_timer_init();
	}

	timer = nmp_timer_new(timeout, (nmp_on_timer)on_timer, data);
	BUG_ON(!timer);
	nmp_timer_attach((nmp_event_loop_t*)timer_loop, timer);

	return timer;
}


void nmp_scheduler_del_timer(void *handle)
{
	nmp_timer_del((nmp_timer_t*)handle);
}



//:~ End
