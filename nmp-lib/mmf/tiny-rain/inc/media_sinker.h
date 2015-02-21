/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_MEDIA_SINKER_H__
#define __TINY_RAIN_MEDIA_SINKER_H__

#include "media.h"
#include "session.h"

BEGIN_NAMESPACE

#define list_of_sinker(_l_) list_entry((_l_), media_sinker, list);

typedef struct __media_sinker media_sinker;
typedef struct __media_sinker_ops media_sinker_ops;

struct __media_sinker
{
	obj	__super;

	int32_t state;
	struct list_head list;
	media *medium;
	session *host;
	LOCK_T lock;
	media_sinker_ops *ops;
};

struct __media_sinker_ops
{
	int32_t (*init)(media_sinker *sinker, void *u);
	void	(*fin)(media_sinker *sinker);
	void	(*kill)(media_sinker *sinker);
	int32_t (*consumable)(media_sinker *sinker, int32_t stm_i, uint32_t size);
	int32_t (*consume)(media_sinker *sinker, int32_t stm_i, msg *m, uint32_t seq);
	int32_t (*set_config)(media_sinker *sinker, void *in, void *data);
	int32_t (*get_config)(media_sinker *sinker, void *in, void *data);
};

media_sinker *media_sinker_alloc(uint32_t size, media_sinker_ops *ops, void *u);
media_sinker *media_sinker_ref(media_sinker *sinker);
void media_sinker_unref(media_sinker *sinker);
void media_sinker_kill_unref(media_sinker *sinker);

int32_t media_sinker_ready(media_sinker *sinker);
int32_t media_sinker_consumable(media_sinker *sinker, int32_t stm_i,
	uint32_t size);
int32_t media_sinker_ctl(media_sinker *sinker, int32_t cmd, void *data);

int32_t media_sinker_set_config(media_sinker *sinker, void *in, void *data);
int32_t media_sinker_get_config(media_sinker *sinker, void *in, void *data);
int32_t media_sinker_set_owner(media_sinker *sinker, session *s);
int32_t media_sinker_fill(media_sinker *sinker, int32_t stm_i, msg *m, uint32_t seq);

END_NAMESPACE

#endif	//__TINY_RAIN_MEDIA_SINKER_H__
