#include <stdio.h>
#include <string.h>
#include "obj.h"
#include "alloc.h"


obj *__obj_new(uint32_t size, obj_fin fin)
{
	obj *p;

	if (size < sizeof(*p))
		return NULL;

	p = tr_alloc(size);
	p->size = size;
	atomic_set(&p->ref_count, 1);
	p->fin = fin;

	return p;
}


#ifdef TR_DEBUG
obj *__obj_new_debug(uint32_t size, obj_fin fin, uint8_t *name)
{
	obj *p;

	p = __obj_new(size, fin);
	if (p)
	{
		strncpy(__str(p->name), __str(name), NAME_SIZE - 1);
		p->name[NAME_SIZE - 1] = 0;
	}

	return p;
}
#endif


static __inline__ void
obj_free(obj *p)
{
	if (p->fin)
	{
		(*p->fin)(p);
	}
#ifdef TR_DEBUG
	printf("####DEATH:obj '%p', born on '%s' freed!!\n", p, p->name);
#endif
	tr_free(p, p->size);
}


void obj_ref(void *_p)
{
	obj *p = (obj*)_p;
	BUG_ON(atomic_get(&p->ref_count) <= 0);

	atomic_inc(&p->ref_count);
}


void obj_unref(void *_p)
{
	obj *p = (obj*)_p;
	BUG_ON(atomic_get(&p->ref_count) <= 0);

	if (atomic_dec_and_test_zero(&p->ref_count))
	{
		obj_free(p);
	}
}


//:~ End
