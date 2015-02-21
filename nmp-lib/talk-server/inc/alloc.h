/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_ALLOC_H__
#define __TINY_RAIN_ALLOC_H__

#include "def.h"

void *tr_alloc(uint32_t size);
void *tr_alloc0(uint32_t size);
void  tr_free(void *, uint32_t size);

#endif	//__TINY_RAIN_ALLOC_H__
