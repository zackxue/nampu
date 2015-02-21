/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#ifndef __TINY_RAIN_LOCAL_DEV_SRC_H__
#define __TINY_RAIN_LOCAL_DEV_SRC_H__

#include "media_src.h"
#include "tr_avs.h"

BEGIN_NAMESPACE

typedef struct __ld_src ld_src;
struct __ld_src
{
	media_src __super;
	int8_t  idx[ST_MAX];
	int32_t ldl; 		//@{live src}
	int32_t break_off;	//@{user killed}
	void *u;	 		//@{user data}
};

void __fill_media_info(ld_src *lds, media_info *msi, media_info_t *mi);
int32_t ld_src_break_off(ld_src *lds);

END_NAMESPACE

#endif	//__TINY_RAIN_LOCAL_DEV_SRC_H__
