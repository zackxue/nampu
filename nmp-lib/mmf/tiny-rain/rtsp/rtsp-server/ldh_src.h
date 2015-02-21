/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#ifndef __TINY_RAIN_LOCAL_DEV_HIS_SRC_H__
#define __TINY_RAIN_LOCAL_DEV_HIS_SRC_H__

#include "ld_src.h"
#include "list.h"
#include "timer.h"

BEGIN_NAMESPACE

typedef struct __ldh_src ldh_src;
struct __ldh_src
{
	ld_src __super;
	uint32_t state;
	uint32_t flags;
	struct list_head frm_list;	/* pre-read */
	uint32_t frm_count;
	LOCK_T ldh_lock;
	void *next_frame;
	int32_t err;
	uint32_t last_ts;
    int32_t  scale;
	Timer *timer;
};

int32_t ldh_facility_init(int32_t tp_threads);
ldh_src *ldh_src_alloc(void *u);
frame_t *ldh_alloc_frame(uint32_t data_size, uint32_t nal_count);
void ldh_free_frame(frame_t *frame);
void ldh_src_set_download_mode(ldh_src *ldh);

END_NAMESPACE

#endif	//__TINY_RAIN_LOCAL_DEV_HIS_SRC_H__
