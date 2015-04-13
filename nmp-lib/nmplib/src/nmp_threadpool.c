#include <assert.h>	////remove
#include "nmp_threadpool.h"
#include "nmp_thread.h"
#include "nmp_mem.h"
#include "nmp_asyncqueue.h"


typedef struct _nmp_real_thread_pool nmp_real_thread_pool_t;

struct _nmp_real_thread_pool
{
	nmp_threadpool_t   thread_pool;
	nmp_async_queue_t   *queue;
	unsigned            max_threads;
	unsigned            num_threads;
};


static void *nmp_threadpool_thread_proxy(void *arg)
{
	nmp_real_thread_pool_t *tp = (nmp_real_thread_pool_t*)arg;
	void *data;

	while ( TRUE )
	{
		data = nmp_async_queue_pop(tp->queue);
		(*tp->thread_pool.func)(data, tp->thread_pool.user_data);
	}

	return NULL;
}

static void nmp_threadpool_start_threads(nmp_real_thread_pool_t *tp, nmp_error_t **err)
{
	nmp_error_t *error = NULL;

	while (tp->max_threads == -1 || tp->num_threads < tp->max_threads)
	{
		nmp_thread_create(nmp_threadpool_thread_proxy, tp, 0, &error);
		
		if (error)
		{
			if (err)
				*err = error;
			return;
		}

		++tp->num_threads;

		if (tp->max_threads == -1)
			break;
	}
}


nmp_threadpool_t *nmp_threadpool_new(nmp_threadpool_func func, void *data, 
	unsigned max_threads, nmp_error_t **err)
{
	nmp_real_thread_pool_t *tp;

	tp = nmp_new(nmp_real_thread_pool_t, 1);
	tp->thread_pool.func = func;
	tp->thread_pool.user_data = data;
	tp->queue = nmp_async_queue_new();
	tp->max_threads = max_threads ? max_threads : 1;
	tp->num_threads = 0;
	
	nmp_threadpool_start_threads(tp, err);

	return (nmp_threadpool_t*)tp;
}


void nmp_threadpool_push(nmp_threadpool_t *tp, void *data)
{
	nmp_real_thread_pool_t *_tp;
	NMP_ASSERT(tp != NULL && data != NULL);

	_tp = (nmp_real_thread_pool_t*)tp;
	nmp_async_queue_push(_tp->queue, data);
}


void nmp_threadpool_free(nmp_threadpool_t *tp, nmp_bool_t drop, nmp_bool_t wait)
{
	
}

//:~ End
