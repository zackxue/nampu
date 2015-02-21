#include "nmp_asyncqueue.h"
#include "nmp_mem.h"
#include "nmp_macros.h"

JAsyncQueue *j_async_queue_new( void )
{
	JAsyncQueue *queue = j_new(JAsyncQueue, 1);

	atomic_set(&queue->ref_count, 1);
	j_queue_init(&queue->queue);
	queue->mutex = j_mutex_new();
	queue->cond = j_cond_new();

	return queue;
}


JAsyncQueue *j_async_queue_ref(JAsyncQueue *queue)
{
	J_ASSERT(queue != NULL);

	atomic_inc(&queue->ref_count);
	return queue;
}


void j_async_queue_unref(JAsyncQueue *queue)
{//TODO
}


void j_async_queue_push_unlocked(JAsyncQueue *queue, void *data)
{
	J_ASSERT(queue != NULL && data != NULL);

	j_queue_push_tail(&queue->queue, data);
	j_cond_signal(queue->cond);
}


void j_async_queue_push(JAsyncQueue *queue, void *data)
{
	J_ASSERT(queue != NULL);

	j_mutex_lock(queue->mutex);
	j_async_queue_push_unlocked(queue, data);
	j_mutex_unlock(queue->mutex);
}


void *j_async_queue_pop_unlocked(JAsyncQueue *queue)
{
	void *data;

	for (;;)
	{
		data = j_queue_pop_head(&queue->queue);
		if (data)
			return data;

		if (!j_cond_timed_wait(queue->cond, queue->mutex, NULL))
			break;	/* nerver */
	}

	return NULL;
}


void *j_async_queue_pop(JAsyncQueue *queue)
{
	void *retval;
	J_ASSERT(queue != NULL);

	j_mutex_lock(queue->mutex);
	retval = j_async_queue_pop_unlocked(queue);
	j_mutex_unlock(queue->mutex);

	return retval;
}


//:~ End
