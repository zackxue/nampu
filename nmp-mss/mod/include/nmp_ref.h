/* *
 * Copyright (C) 2012 fangyi <fangyi@szjxj.net>
*/

#ifndef __NMP_REF_H__
#define __NMP_REF_H__


#define MAX_REF_COUNT_VALUE (50)	/* Maybe enough */
#define REF_DEBUG_TEST(pobj)	\
do {\
	g_assert((pobj) && g_atomic_int_get(&(pobj)->ref_count) > 0); \
	g_assert(g_atomic_int_get(&(pobj)->ref_count) < MAX_REF_COUNT_VALUE); \
} while (0)

#define __OBJECT_REF(x) \
	REF_DEBUG_TEST(x); \
	g_atomic_int_add(&(x)->ref_count, 1); \
	return x;

#define __OBJECT_UNREF(x, FUNC_PREFIX) \
	REF_DEBUG_TEST(x); \
	if (g_atomic_int_dec_and_test(&(x)->ref_count)) \
	{\
		FUNC_PREFIX##_finalize(x);\
	}

#endif	/* __NMP_REF_H__ */
