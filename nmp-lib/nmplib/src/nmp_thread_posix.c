#include <pthread.h>
#include <stdio.h>
#include "nmp_mem.h"
#include "nmp_thread.h"

#define MIN_STACK_SIZE	8192
#define NSEC_PER_SEC	1000000000

#define posix_check_err(err, name) \
do {\
	int error = (err); 							\
	if (error)	 		 		 			\
		fprintf(stderr, "file %s: line %d (%s): error '%d' during '%s'",		\
           __FILE__, __LINE__, __FUNCTION__, error, name);					\
} while (0)

#define posix_check_cmd(cmd) posix_check_err((cmd), #cmd)

static int nmp_thread_priority_map[THREAD_PRIORITY_URGENT + 1] =
{	
};


static nmp_mutex_t *
nmp_mutex_new_posix_impl( void )
{
	nmp_mutex_t *result = (nmp_mutex_t*)nmp_new(pthread_mutex_t, 1);
	posix_check_cmd(
		pthread_mutex_init((pthread_mutex_t*)result,
		NULL)
	);
	return result;
}


static void
nmp_mutex_lock_posix_impl(nmp_mutex_t *mutex)
{
	pthread_mutex_lock((pthread_mutex_t*)mutex);
}


static int
nmp_mutex_trylock_posix_impl(nmp_mutex_t *mutex)
{
	int result;

	result = pthread_mutex_trylock((pthread_mutex_t*)mutex);
	if (result == EBUSY)
		return 0;

	posix_check_err(result, "pthread_mutex_trylock");
	return 1;
}


static void
nmp_mutex_unlock_posix_impl(nmp_mutex_t *mutex)
{
	pthread_mutex_unlock((pthread_mutex_t*)mutex);
}


static void
nmp_mutex_free_posix_impl(nmp_mutex_t *mutex)
{
	posix_check_cmd(
		pthread_mutex_destroy((pthread_mutex_t*)mutex)
	);
	nmp_del(mutex, pthread_mutex_t, 1);
}


static nmp_cond_t *
nmp_cond_new_posix_impl( void )
{
	nmp_cond_t *result = (nmp_cond_t*)nmp_new(pthread_cond_t, 1);
	posix_check_cmd(
		pthread_cond_init((pthread_cond_t*)result,
			NULL)
	);

  return result;
}


static void
nmp_cond_signal_posix_impl(nmp_cond_t *cond)
{
	pthread_cond_signal((pthread_cond_t*)cond);
}


static void
nmp_cond_broadcast_posix_impl(nmp_cond_t *cond)
{
	pthread_cond_broadcast((pthread_cond_t*)cond);
}


static void
nmp_cond_wait_posix_impl(nmp_cond_t *cond, nmp_mutex_t *mutex)
{
	pthread_cond_wait((pthread_cond_t*)cond,
		(pthread_mutex_t*)mutex);
}


static int
nmp_cond_timed_wait_posix_impl(nmp_cond_t *cond, nmp_mutex_t *cond_mutex,
	nmp_timeval_t *abs_time)
{
	struct timespec end_time;
	int result, timed_out;

	if (!cond || !cond_mutex)
		return 0; /* FALSE */

	if (!abs_time)
    {
		result = pthread_cond_wait((pthread_cond_t*)cond,
			(pthread_mutex_t*)cond_mutex);
		timed_out = 0;
    }
	else
	{
		end_time.tv_sec = abs_time->tv_sec;
		end_time.tv_nsec = abs_time->tv_usec * 1000;

		if (end_time.tv_nsec >= NSEC_PER_SEC)
			return 1;

		result = pthread_cond_timedwait((pthread_cond_t*)cond,
			(pthread_mutex_t*)cond_mutex, &end_time);

		timed_out = (result == ETIMEDOUT);
    }

	if (!timed_out)
		posix_check_err(result, "pthread_cond_timedwait");

  return !timed_out;
}


static void
nmp_cond_free_posix_impl(nmp_cond_t *cond)
{
	posix_check_cmd(
		pthread_cond_destroy((pthread_cond_t*)cond)
	);
	nmp_del(cond, pthread_cond_t, 1);
}


static void
nmp_thread_create_posix_impl(nmp_thread_func func, void *arg,
	unsigned long stack_size, int joinable, int bound,
	nmp_thread_priority priority, void *thread, nmp_error_t **error)
{
	pthread_attr_t attr;
	int ret;

	if (!func || priority < THREAD_PRIORITY_LOW
		|| priority > THREAD_PRIORITY_URGENT)
	{
		return;
	}

	posix_check_cmd(pthread_attr_init(&attr));

	if (stack_size)
    {
		stack_size = stack_size > MIN_STACK_SIZE ?
			stack_size : MIN_STACK_SIZE;
		pthread_attr_setstacksize(&attr, stack_size);
    }

	if (bound)
	{
		pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	}

	posix_check_cmd(
		pthread_attr_setdetachstate(&attr,
          joinable ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED)
	);

#if 0	/* not support yet */
    struct sched_param sched;
    posix_check_cmd (pthread_attr_getschedparam(&attr, &sched));
    sched.sched_priority = nmp_thread_priority_map[priority];
    posix_check_cmd_prio(pthread_attr_setschedparam (&attr, &sched));
#endif

	ret = pthread_create(thread, &attr, (void* (*)(void*))func, arg);
	posix_check_cmd(pthread_attr_destroy(&attr));
	posix_check_err(ret, "pthread_create");
}


static void
nmp_thread_yield_posix_impl(void)
{
#if 0
	pthread_yield();
#endif
}


static void
nmp_thread_join_posix_impl(void *thread)
{
	void *ignore;

	posix_check_cmd(
		pthread_join(*(pthread_t*)thread, &ignore)
	);
}


static void
nmp_thread_exit_posix_impl(void)
{
	pthread_exit(NULL);
}


static void
nmp_thread_set_priority_posix_impl(void *thread, nmp_thread_priority priority)
{
	struct sched_param sched;
	int policy;

	if (priority < THREAD_PRIORITY_LOW ||
			priority > THREAD_PRIORITY_URGENT)
	{
		return;
	}

    posix_check_cmd(
    	pthread_getschedparam(*(pthread_t*)thread, &policy, &sched)
    );

    sched.sched_priority = nmp_thread_priority_map[priority];

    posix_check_cmd(
    	pthread_setschedparam(*(pthread_t*)thread, policy,
		&sched)
	);
}


static void
nmp_thread_self_posix_impl(void *thread)
{
	*(pthread_t*)thread = pthread_self();
}


static int
nmp_thread_equal_posix_impl(void *thread1, void *thread2)
{
	return (pthread_equal(*(pthread_t*)thread1, 
		*(pthread_t*)thread2) != 0);
}


static nmp_private_t *
nmp_private_new_posix_impl(nmp_destroy_notify destructor)
{
	nmp_private_t *result = (nmp_private_t*)nmp_new(pthread_key_t, 1);

	posix_check_cmd(
		pthread_key_create((pthread_key_t*)result, destructor)
	);

	return result;
}


static void *
nmp_private_get_posix_impl(nmp_private_t *private_key)
{
	if (!private_key)
		return NULL;

	return pthread_getspecific(*(pthread_key_t*)private_key);
}


static void
nmp_private_set_posix_impl(nmp_private_t *private_key, void *value)
{
	if (!private_key)
		return;

	pthread_setspecific(*(pthread_key_t*)private_key, value);
}


static nmp_thread_interfaces_t nmplib_posix_thread_impl = 
{
	nmp_mutex_new_posix_impl,
	nmp_mutex_lock_posix_impl,
	nmp_mutex_trylock_posix_impl,
	nmp_mutex_unlock_posix_impl,
	nmp_mutex_free_posix_impl,

	nmp_cond_new_posix_impl,
	nmp_cond_signal_posix_impl,
	nmp_cond_broadcast_posix_impl,
	nmp_cond_wait_posix_impl,
	nmp_cond_timed_wait_posix_impl,
	nmp_cond_free_posix_impl,

	nmp_thread_create_posix_impl,
	nmp_thread_yield_posix_impl,
	nmp_thread_join_posix_impl,
	nmp_thread_exit_posix_impl,
	nmp_thread_set_priority_posix_impl,
	nmp_thread_self_posix_impl,
	nmp_thread_equal_posix_impl,

	nmp_private_new_posix_impl,
	nmp_private_get_posix_impl,
	nmp_private_set_posix_impl
};

nmp_thread_interfaces_t *nmplib_thread_ops = &nmplib_posix_thread_impl;

//:~ End
