
#ifndef __MACRO_H__
#define __MACRO_H__

#ifdef __WIN32__
#define inline __inline
#define gettid() GetCurrentThreadId()
#else
#include <sys/syscall.h>  
#define gettid() syscall(__NR_gettid)

#include "jlib.h"

#endif


#define POINTER_BACK_TO(x) \
	void*	owner;

#define SET_OWNER(x, o) \
	(x)->owner = o

#define GET_OWNER(x, TYPE) \
	(TYPE*)((x)->owner)

#ifdef WIN32
#define __OBJECT_REF(x) \
	assert(x); \
	InterlockedIncrement((volatile LONG *)(&(x)->ref_count)); \
	return x;

#define __OBJECT_UNREF(x, FUNC_PREFIX) \
	assert(x); \
	if (!InterlockedDecrement((volatile LONG *)(&(x)->ref_count))) \
	{\
		FUNC_PREFIX##_finalize(x);\
	}
#else
#define __OBJECT_REF(x) \
	assert(x); \
	atomic_add(&(x)->ref_count, 1); \
	return x;

#define __OBJECT_UNREF(x, FUNC_PREFIX) \
	assert(x); \
	if (atomic_dec_and_test_zero(&(x)->ref_count)) \
	{\
		FUNC_PREFIX##_finalize(x);\
	}
#endif
#endif	/* __MACRO_H__ */

