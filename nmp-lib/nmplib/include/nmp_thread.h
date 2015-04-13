#ifndef __NMP_THREAD_H__
#define __NMP_THREAD_H__

#include "nmp_error.h"
#include "nmp_types.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*nmp_thread_func)(void *data);
typedef void (*nmp_destroy_notify)(void *data);

typedef struct _nmp_thread  nmp_thread_t;
typedef struct _nmp_mutex   nmp_mutex_t;
typedef struct _nmp_cond    nmp_cond_t;
typedef struct _nmp_private nmp_private_t;

typedef enum
{
	THREAD_PRIORITY_LOW,
	THREAD_PRIORITY_NORMAL,
	THREAD_PRIORITY_HIGH,
	THREAD_PRIORITY_URGENT
}nmp_thread_priority;

struct _nmp_thread
{
	nmp_thread_func func;
	void *data;
	int joinable;
	nmp_thread_priority priority;
};

typedef struct _nmp_thread_interfaces nmp_thread_interfaces_t;

struct _nmp_thread_interfaces
{
	nmp_mutex_t* (*mutex_new)(void);
	void (*mutex_lock)(nmp_mutex_t *mutex);
	int  (*mutex_trylock)(nmp_mutex_t *mutex);
	void (*mutex_unlock)(nmp_mutex_t *mutex);
	void (*mutex_free)(nmp_mutex_t *mutex);

	nmp_cond_t* (*cond_new)(void);
	void (*cond_signal)(nmp_cond_t *cond);
	void (*cond_broadcast)(nmp_cond_t *cond);
	void (*cond_wait)(nmp_cond_t *cond, nmp_mutex_t *mutex);
	int  (*cond_timed_wait)(nmp_cond_t *cond, nmp_mutex_t *mutex, nmp_timeval_t *end_time);
	void (*cond_free)(nmp_cond_t *cond);

	void (*thread_create)(nmp_thread_func func, void *arg, unsigned long stack_size,
						int joinable, int bound, nmp_thread_priority priority,
                        void *thread, nmp_error_t **error);

	void (*thread_yield)(void);
	void (*thread_join)(void *thread);
	void (*thread_exit)(void);
	void (*thread_set_priority)(void *thread, nmp_thread_priority priority);
	void (*thread_self)(void *thread);
	int  (*thread_equal)(void *thread1, void *thread2);

	nmp_private_t* (*private_new)(nmp_destroy_notify destructor);
	void* (*private_get)(nmp_private_t *private_key);
	void (*private_set)(nmp_private_t *private_key, void *data);
};

extern nmp_thread_interfaces_t *nmplib_thread_ops;

#define NMP_THREAD_INVOKE(op, arglist) \
	(*nmplib_thread_ops->op) arglist

#define nmp_mutex_new() NMP_THREAD_INVOKE(mutex_new, ())
#define nmp_mutex_lock(mutex) NMP_THREAD_INVOKE(mutex_lock, (mutex))
#define nmp_mutex_trylock(mutex) NMP_THREAD_INVOKE(mutex_trylock, (mutex))
#define nmp_mutex_unlock(mutex) NMP_THREAD_INVOKE(mutex_unlock, (mutex))
#define nmp_mutex_free(mutex) NMP_THREAD_INVOKE(mutex_free, (mutex))
#define nmp_cond_new() NMP_THREAD_INVOKE(cond_new, ())
#define nmp_cond_signal(cond) NMP_THREAD_INVOKE(cond_signal, (cond))
#define nmp_cond_broadcast(cond) NMP_THREAD_INVOKE(cond_broadcast, (cond))
#define nmp_cond_wait(cond, mutex) NMP_THREAD_INVOKE(cond_wait, ((cond), (mutex)))
#define nmp_cond_timed_wait(cond, mutex, abs_time) \
	NMP_THREAD_INVOKE(cond_timed_wait, ((cond), (mutex), (abs_time)))
#define nmp_cond_free(cond) NMP_THREAD_INVOKE(cond_free, (cond))
#define nmp_thread_yield() NMP_THREAD_INVOKE(thread_yield, ())
#define nmp_private_new(destructor) NMP_THREAD_INVOKE(private_new, (destructor))
#define nmp_private_get(key) NMP_THREAD_INVOKE(private_get, (key))
#define nmp_private_set(key, data) NMP_THREAD_INVOKE(private_set, ((key), (data)))

#define nmp_thread_join(thread_id) NMP_THREAD_INVOKE(thread_join, (thread_id))
#define nmp_thread_self(thread_id) NMP_THREAD_INVOKE(thread_self, (thread_id))
#define nmp_thread_exit() NMP_THREAD_INVOKE(thread_exit, ())

nmp_thread_t *nmp_thread_create_internal(nmp_thread_func func, void *arg, 
	unsigned long stack_size,int joinable, int bound, nmp_thread_priority priority,
	nmp_error_t **error);

#define nmp_thread_create(func, arg, joinable, error) \
	nmp_thread_create_internal(func, arg, 0, joinable, 0, THREAD_PRIORITY_NORMAL, error)

void nmp_thread_set_priority(nmp_thread_t *thread, nmp_thread_priority priority);
nmp_bool_t nmp_thread_equal(nmp_thread_t *thread1, nmp_thread_t *thread2);


#ifdef __cplusplus
}
#endif

#endif	/* __NMP_THREAD_H__ */
