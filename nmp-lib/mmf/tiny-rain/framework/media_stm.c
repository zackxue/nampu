#include <string.h>
#include "media_stm.h"
#include "alloc.h"
#include "tr_log.h"


void
media_info_init(media_info *mi)
{
	memset(mi, 0, sizeof(*mi));
}


static __inline__ void
__copy_stm(stm_info *dst, stm_info *src)
{
	int32_t fileds;
	stm_into_pri *dst_f, *src_f;

	for (fileds = 0; fileds < src->fields; ++fileds)
	{
		dst_f = &dst->pri_fields[fileds];
		src_f = &src->pri_fields[fileds];
		if (src_f->data)
		{
			dst_f->data = tr_alloc(src_f->size);
			if (!dst_f->data)
			{
				LOG_E("__copy_stm()->tr_alloc() failed, OOM!");
			}
			else
			{
				memcpy(dst_f->data, src_f->data, src_f->size);
			}
		}
	}
}


void
media_info_dup(media_info *dst, media_info *src)
{
	int32_t stm_i;

	memcpy(dst, src, sizeof(*src)); 
	for (stm_i = 0; stm_i < src->n_stms; ++stm_i)
	{
		__copy_stm(&dst->stms[stm_i], &src->stms[stm_i]);
	}
}


static __inline__ void
__clear_stm(stm_info *stm)
{
	int32_t fileds;
	stm_into_pri *f;

	for (fileds = 0; fileds < stm->fields; ++fileds)
	{
		f = &stm->pri_fields[fileds];
		if (f->data)
		{
			tr_free(f->data, f->size);
		}
	}
}


void
media_info_clear(media_info *mi)
{
	int32_t stm_i;

	for (stm_i = 0; stm_i < mi->n_stms; ++stm_i)
	{
		__clear_stm(&mi->stms[stm_i]);
	}

	memset(mi, 0, sizeof(*mi));
}


//:~ End
