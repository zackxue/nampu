#ifndef __NMP_MACROS_H__
#define __NMP_MACROS_H__

#include <assert.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined (__GNUC__) && __GNUC__ >= 4
	# define __ATTR_WARN_UNUSED_RETSULT__ \
		__attribute__((warn_unused_result))
	# define __offsetof__(struct_type, member)	\
		((long)(offsetof(struct_type, member)))
#else
	# define __ATTR_WARN_UNUSED_RETSULT__
	# define __offsetof__(struct_type, member)	\
		((long)(unsigned char*)&((struct_type*)0)->member)
#endif

#if defined (__GNUC__) && __GNUC__ > 2 /* since 2.9 */
	# define NMP_LIKELY(expr) (__builtin_expect(!!(expr), 1))
	# define NMP_UNLIKELY(expr) (__builtin_expect(!!(expr), 0))
#else
	# define NMP_LIKELY(expr) (expr)
	# define NMP_UNLIKELY(expr) (expr)
#endif

#define container_of(ptr, struct_type, member) \
	(struct_type*)(((char*)(ptr)) - __offsetof__(struct_type, member))

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#define ALIGN(size, alignment) (((size) + (alignment) - 1) & ~((alignment) - 1))

#define __export

#define NMP_ASSERT(x) assert(x)
#define BUG() \
do {\
	fprintf(stderr, "BUG() at file:%s line:%d function:%s!\n", __FILE__, __LINE__, __FUNCTION__); \
	char *_______________________p = 0; *_______________________p = 0; \
} while (0)

#define BUG_ON(x) \
do {\
	if (x) \
	{\
		fprintf(stderr, "BUG_ON(%s) at file:%s line:%d function:%s!\n", #x, __FILE__, __LINE__, __FUNCTION__); \
		char *_______________________p = 0; *_______________________p = 0; \
	}\
} while (0)
	
#ifdef __cplusplus
}
#endif


#endif	/* __NMP_MACROS_H__ */
