#include "nmp_thread.h"
#include "nmp_mem.h"



typedef struct _JRealThread JRealThread;
struct _JRealThread
{
	JThread thread_base;
	void *retval;
	JSystemThread system_thread;
};


static void *j_thread_proxy(void *arg)
{
	JRealThread *rt = (JRealThread*)arg;

	rt->retval = (*rt->thread_base.func)(rt->thread_base.data);

	return NULL;
}


JThread * j_thread_create_internal(JThreadFunc func, void *arg, unsigned long stack_size,
	int joinable, int bound, JThreadPriority priority, JError **error)
{
	JRealThread *result = j_new(JRealThread, 1);
	JError *err = NULL;

	result->thread_base.func = func;
	result->thread_base.data = arg;
	result->thread_base.joinable = joinable;
	result->thread_base.priority = priority;

	J_THREAD_INVOKE(thread_create, (j_thread_proxy, result, stack_size, joinable,
		bound, priority, &result->system_thread, &err));

	if (!err)
	{
		return (JThread*)result;
	}

	if (error)
	{
		*error = err;
	}
	
	j_del(result, JRealThread, 1);

	return NULL;
}


void *j_thread_join(JThread *thread)
{
	return NULL;
}


void j_thread_exit(void *retval)
{
}


void j_thread_set_priority(JThread *thread, JThreadPriority priority)
{
}


JThread *j_thread_self( void )
{
	return NULL;
}


JBool j_thread_equal(JThread *thread1, JThread *thread2)
{
	return FALSE;
}


//:~ End
