#ifndef __NMP_ASYNCQUEUE_H__
#define __NMP_ASYNCQUEUE_H__

#include "nmp_queue.h"
#include "nmp_thread.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _nmp_async_queue nmp_async_queue_t;

struct _nmp_async_queue
{
	atomic_t ref_count;
	nmp_queue_t queue;
	nmp_mutex_t *mutex;
	nmp_cond_t *cond;
};

nmp_async_queue_t *nmp_async_queue_new( void );
nmp_async_queue_t *nmp_async_queue_ref(nmp_async_queue_t *queue);
void nmp_async_queue_unref(nmp_async_queue_t *queue);
void nmp_async_queue_push(nmp_async_queue_t *queue, void *data);
void *nmp_async_queue_pop(nmp_async_queue_t *queue);

#ifdef __cplusplus
}
#endif


#endif	/* __NMP_ASYNCQUEUE_H__ */
