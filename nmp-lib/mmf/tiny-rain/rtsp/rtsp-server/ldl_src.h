/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#ifndef __TINY_RAIN_LOCAL_DEV_LIVE_SRC_H__
#define __TINY_RAIN_LOCAL_DEV_LIVE_SRC_H__

#include "ld_src.h"

BEGIN_NAMESPACE

typedef struct __ldl_src ldl_src;
struct __ldl_src
{
	ld_src __super;
	uint32_t state;
	LOCK_T ldl_lock;
};

int32_t ldl_facility_init(int32_t tp_threads);
ldl_src *ldl_src_alloc(void *u);
int32_t ldl_src_consume(ldl_src *ldl, frame_t *frm);

END_NAMESPACE

#endif	//__TINY_RAIN_LOCAL_DEV_LIVE_SRC_H__
