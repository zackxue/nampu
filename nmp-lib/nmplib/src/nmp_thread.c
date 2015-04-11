#include "nmp_thread.h"
#include "nmp_mem.h"

typedef struct _nmp_real_thread nmp_real_thread_t;
struct _nmp_real_thread
{
	nmp_thread_t        thread_base;
	void                *retval;
	nmp_system_thread_t system_thread;
};


static void *nmp_thread_proxy(void *arg)
{
	nmp_real_thread_t *rt = (nmp_real_thread_t*)arg;

	rt->retval = (*rt->thread_base.func)(rt->thread_base.data);

	return NULL;
}


nmp_thread_t * nmp_thread_create_internal(nmp_thread_func func, void *arg, unsigned long stack_size,
	int joinable, int bound, nmp_thread_priority priority, nmp_error_t **error)
{
	nmp_real_thread_t *result = nmp_new(nmp_real_thread_t, 1);
	nmp_error_t *err = NULL;

	result->thread_base.func = func;
	result->thread_base.data = arg;
	result->thread_base.joinable = joinable;
	result->thread_base.priority = priority;

	NMP_THREAD_INVOKE(thread_create, (nmp_thread_proxy, result, stack_size, joinable,
		bound, priority, &result->system_thread, &err));

	if (!err)
	{
		return (nmp_thread_t*)result;
	}

	if (error)
	{
		*error = err;
	}
	
	nmp_del(result, nmp_real_thread_t, 1);

	return NULL;
}


void nmp_thread_set_priority(nmp_thread_t *thread, nmp_thread_priority priority)
{
}


nmp_bool_t nmp_thread_equal(nmp_thread_t *thread1, nmp_thread_t *thread2)
{
	return FALSE;
}


//:~ End
