#include "nmp_threadpool.h"
#include "nmp_thread.h"
#include "nmp_mem.h"
#include "nmp_asyncqueue.h"


typedef struct _JRealThreadPool JRealThreadPool;

struct _JRealThreadPool
{
	JThreadPool thread_pool;
	JAsyncQueue *queue;
	unsigned max_threads;
	unsigned num_threads;
};


static void *j_thread_pool_thread_proxy(void *arg)
{
	JRealThreadPool *tp = (JRealThreadPool*)arg;
	void *data;

	while ( TRUE )
	{
		data = j_async_queue_pop(tp->queue);
		(*tp->thread_pool.func)(data, tp->thread_pool.user_data);
	}

	return NULL;
}

static void j_thread_pool_start_threads(JRealThreadPool *tp, JError **err)
{
	JError *error = NULL;

	while (tp->max_threads == -1 || tp->num_threads < tp->max_threads)
	{
		j_thread_create(j_thread_pool_thread_proxy, tp, 0, &error);
		
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


JThreadPool *j_thread_pool_new(JTPFunc func, void *data, 
	unsigned max_threads, JError **err)
{
	JRealThreadPool *tp;

	tp = j_new(JRealThreadPool, 1);
	tp->thread_pool.func = func;
	tp->thread_pool.user_data = data;
	tp->queue = j_async_queue_new();
	tp->max_threads = max_threads ? max_threads : 1;
	tp->num_threads = 0;
	
	j_thread_pool_start_threads(tp, err);

	return (JThreadPool*)tp;
}


void j_thread_pool_push(JThreadPool *tp, void *data)
{
	JRealThreadPool *_tp;
	J_ASSERT(tp != NULL && data != NULL);

	_tp = (JRealThreadPool*)tp;
	j_async_queue_push(_tp->queue, data);
}


void j_thread_pool_free(JThreadPool *tp, JBool drop, JBool wait)
{
	
}

//:~ End
