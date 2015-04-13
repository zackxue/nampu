#ifndef __NMP_THREAD_POOL_H__
#define __NMP_THREAD_POOL_H__

#include "nmp_error.h"
#include "nmp_types.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef void (*nmp_threadpool_func)(void *data, void *user_data);

typedef struct _nmp_thread_pool nmp_threadpool_t;

struct _nmp_thread_pool
{
	nmp_threadpool_func func;
	void    *user_data;
};

nmp_threadpool_t *nmp_threadpool_new(nmp_threadpool_func func, void *data,
	unsigned max_threads, nmp_error_t **err);
void nmp_threadpool_push(nmp_threadpool_t *tp, void *data);
void nmp_threadpool_free(nmp_threadpool_t *tp, nmp_bool_t drop, nmp_bool_t wait);

#ifdef __cplusplus
}
#endif

#endif	/* __NMP_THREAD_POOL_H__ */
