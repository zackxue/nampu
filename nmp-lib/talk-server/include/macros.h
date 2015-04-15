
#ifndef __MACRO_H__
#define __MACRO_H__

#include <sys/syscall.h>  
#define gettid() syscall(__NR_gettid)

#include "nmplib.h"

#define POINTER_BACK_TO(x) \
	void*	owner;

#define SET_OWNER(x, o) \
	(x)->owner = o

#define GET_OWNER(x, TYPE) \
	(TYPE*)((x)->owner)

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
    
#endif	/* __MACRO_H__ */

