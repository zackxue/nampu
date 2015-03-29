/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_MEDIA_H__
#define __TINY_RAIN_MEDIA_H__

#include "def.h"
#include "list.h"
#include "pq.h"
#include "media_src.h"
#include "media_filter.h"
#include "timer.h"

BEGIN_NAMESPACE

typedef enum __wait_type wait_type;
enum __wait_type
{
	W_MEDIA_INFO,
	W_MEDIA_STM
};

typedef struct __media media;
struct __media
{
	obj	__super;
	struct list_head hash_list;

	uint32_t no;
	uint32_t state;
	uint32_t flags;
	uint32_t seq_generator;

	media_uri mrl;
	media_info *info;

	media_filter *filter_src;
	media_src *src;
	struct list_head sinkers;

	pq waiting_for_info;
	pq waiting_for_stms;
	int32_t wfi_err;
	int32_t wfs_err;

	LOCK_T	lock;
	Timer *timer;
};

struct __media_ops
{
	media_info *(*probe)(media *media, uint8_t *mrl);
};

media *get_media(media_uri *mrl);
int32_t media_seq(media *m);

media *media_ref(media *m);
void media_unref(media *m);
void media_kill_unref(media *m);
int32_t media_killed(media *m);

int32_t media_ready_to_start(media *m);
int32_t media_would_block(media *m, int32_t stm_idx, void *data,
	uint32_t size);

int32_t media_insert_sinker(media *m, struct list_head *snk);
int32_t media_remove_sinker(media *m, struct list_head *snk);

int32_t media_attach_source(media *m, media_src *src);

int32_t media_info_get(media *m, media_info *msi);
int32_t media_info_fill(media *m, media_info *msi, int32_t err);
int32_t media_open_end(media *m, int32_t err);

int32_t media_wait(media *m, wait_type wt, void *parm, pq_fun fun);

int32_t media_ctl(media *m, int32_t cmd, void *data);
int32_t media_fill_stream_from_source(media *m, int32_t stm_i,
	void *data, uint32_t size);

int32_t media_play_ctl(media *m, int32_t cmd, void *data);


END_NAMESPACE

#endif	//__TINY_RAIN_MEDIA_H__
