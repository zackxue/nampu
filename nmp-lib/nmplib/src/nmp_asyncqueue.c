#include "nmp_asyncqueue.h"
#include "nmp_mem.h"
#include "nmp_macros.h"

nmp_async_queue_t *nmp_async_queue_new( void )
{
	nmp_async_queue_t *queue = nmp_new(nmp_async_queue_t, 1);

	atomic_set(&queue->ref_count, 1);
	nmp_queue_init(&queue->queue);
	queue->mutex = nmp_mutex_new();
	queue->cond = nmp_cond_new();

	return queue;
}

static void nmp_async_queue_free(nmp_async_queue_t *queue)
{
    NMP_ASSERT(queue->ref_count == 0);

    nmp_mutex_free(queue->mutex);
    nmp_cond_free(queue->cond);
    nmp_del(queue, nmp_async_queue_t, 1);    
}

nmp_async_queue_t *nmp_async_queue_ref(nmp_async_queue_t *queue)
{
	NMP_ASSERT(queue != NULL);

	atomic_inc(&queue->ref_count);
	return queue;
}


void nmp_async_queue_unref(nmp_async_queue_t *queue)
{//TODO
    NMP_ASSERT(queue != NULL);

    atomic_dec(&queue->ref_count);
    if (queue->ref_count <= 0)
        nmp_async_queue_free(queue);

    return;
}


void nmp_async_queue_push_unlocked(nmp_async_queue_t *queue, void *data)
{
	NMP_ASSERT(queue != NULL && data != NULL);

	nmp_queue_push_tail(&queue->queue, data);
	nmp_cond_signal(queue->cond);
}


void nmp_async_queue_push(nmp_async_queue_t *queue, void *data)
{
	NMP_ASSERT(queue != NULL);

	nmp_mutex_lock(queue->mutex);
	nmp_async_queue_push_unlocked(queue, data);
	nmp_mutex_unlock(queue->mutex);
}


void *nmp_async_queue_pop_unlocked(nmp_async_queue_t *queue)
{
	void *data;

	for (;;)
	{
		data = nmp_queue_pop_head(&queue->queue);
		if (data)
			return data;

		if (!nmp_cond_timed_wait(queue->cond, queue->mutex, NULL))
			break;	/* nerver */
	}

	return NULL;
}


void *nmp_async_queue_pop(nmp_async_queue_t *queue)
{
	void *retval;
	NMP_ASSERT(queue != NULL);

	nmp_mutex_lock(queue->mutex);
	retval = nmp_async_queue_pop_unlocked(queue);
	nmp_mutex_unlock(queue->mutex);

	return retval;
}


//:~ End
