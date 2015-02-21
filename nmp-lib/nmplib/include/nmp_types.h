#ifndef __J_TYPES_H__
#define __J_TYPES_H__


#ifndef _WIN32
# define __USE_GUN
# include <pthread.h>
#endif

typedef enum
{
#ifdef FALSE
	_FALSE,
	_TRUE
#else
	FALSE,
	TRUE
#endif
}JBool;


typedef struct _JTimeVal JTimeVal;

struct _JTimeVal
{
	long tv_sec;	/* Ãë */
	long tv_usec;	/* Î¢Ãë */
};


typedef struct _JSystemThread JSystemThread;

struct _JSystemThread
{
#ifndef _WIN32
	pthread_t	handle;
#else
	void *handle;
#endif
};


#ifdef __cplusplus
extern "C" {
#endif


#if defined (__GNUC__) && __GNUC__ >= 4 /* since 4.1.2 */

#define ATOMIC_INIT			0

typedef volatile int atomic_t;
# define atomic_set(p, val) ((*(p)) = (val))
# define atomic_get(p) __sync_add_and_fetch(p, 0)
# define atomic_add(p, val)  __sync_add_and_fetch(p, val)
# define atomic_sub(p, val)  __sync_sub_and_fetch(p, val)
# define atomic_inc(p) __sync_add_and_fetch(p, 1)
# define atomic_dec(p) __sync_sub_and_fetch(p, 1)
# define atomic_dec_and_test_zero(p) (__sync_sub_and_fetch(p, 1) == 0)

#else	/* (__GNUC__) && __GNUC__ >= 4 */

#if defined _ARM_
# include <atomic_armv6.h>
# define atomic_set(p, val) _atomic_set(p, val)
# define atomic_get(p) _atomic_add(0, p)
# define atomic_add(p, val) _atomic_add(val, p)
# define atomic_sub(p, val) _atomic_sub(val, p)
# define atomic_inc(p) _atomic_inc(p)
# define atomic_dec(p) _atomic_dec(p)
# define atomic_dec_and_test_zero(p) _atomic_sub_and_test(1, p)

#elif defined _X86_
# warning "ERROR!!, X86 Arch, Gcc Version < 4.0!!!"
#else
# warning "ERROR!!, Unknown Arch, No Atomic Operation Supported!!!"
#endif

#endif	/* (__GNUC__) && __GNUC__ >= 4 */

typedef int (*JCompareCustom)(void *data_orig, void *data_custom);
typedef void (*JVisitCustom)(void *data_orig, void *data_custom);

#ifdef __cplusplus
}
#endif

#endif	/* __J_TYPES_H__ */
