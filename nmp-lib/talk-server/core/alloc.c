/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#include <stdlib.h>
#include <string.h>
#include "alloc.h"
#include "jlib.h"


void *tr_alloc(uint32_t size)
{
	return j_alloc(size);
}


void *tr_alloc0(uint32_t size)
{
	return j_alloc0(size);
}


void tr_free(void *p, uint32_t size)
{
	j_dealloc(p, size);
}


//:~ End
