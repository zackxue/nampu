/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#include <stdlib.h>
#include <string.h>
#include "alloc.h"
#include "nmplib.h"


void *tr_alloc(uint32_t size)
{
	return nmp_alloc(size);
}


void *tr_alloc0(uint32_t size)
{
	return nmp_alloc0(size);
}


void tr_free(void *p, uint32_t size)
{
	nmp_dealloc(p, size);
}


//:~ End
