#ifndef __J_ASYNCQUEUE_H__
#define __J_ASYNCQUEUE_H__

#include "nmp_queue.h"
#include "nmp_thread.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _JAsyncQueue JAsyncQueue;

struct _JAsyncQueue
{
	atomic_t ref_count;
	JQueue queue;
	JMutex *mutex;
	JCond *cond;
};

JAsyncQueue *j_async_queue_new( void );
JAsyncQueue *j_async_queue_ref(JAsyncQueue *queue);
void j_async_queue_unref(JAsyncQueue *queue);
void j_async_queue_push(JAsyncQueue *queue, void *data);
void *j_async_queue_pop(JAsyncQueue *queue);

#ifdef __cplusplus
}
#endif


#endif	/* __J_ASYNCQUEUE_H__ */
