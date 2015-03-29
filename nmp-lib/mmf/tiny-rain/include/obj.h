/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_OBJ_H__
#define __TINY_RAIN_OBJ_H__

#include "def.h"

BEGIN_NAMESPACE

#define NAME_SIZE		32			

typedef struct __obj obj;
typedef void (*obj_fin)(obj *p);

struct __obj
{
#ifdef TR_DEBUG
	uint8_t		name[NAME_SIZE];
#endif

	uint32_t	size;
	atomic_t	ref_count;
	obj_fin		fin;
};

#ifdef TR_DEBUG
# define obj_new(s, f) __obj_new_debug(s, f, __u8_str(__FUNCTION__))
obj *__obj_new_debug(uint32_t size, obj_fin fin, uint8_t *name);
#else
# define obj_new __obj_new
obj *__obj_new(uint32_t size, obj_fin fin);
#endif

void obj_ref(void *p);
void obj_unref(void *p);

END_NAMESPACE

#endif	//__TINY_RAIN_OBJ_H__
