/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_MEDIA_SRC_H__
#define __TINY_RAIN_MEDIA_SRC_H__

#include "obj.h"
#include "media_stm.h"

BEGIN_NAMESPACE

enum {
   MEDIA_FORCE_IFRAME = 1   /* live stream */
  ,MEDIA_PLAY_SEEK          /* histroy stream */
  ,MEDIA_PLAY_PAUSE
  ,MEDIA_PLAY_SCALE
};

#define MAX_URL_LEN			256
#define get_media_src_u(p) (((media_src*)p)->user)

typedef struct __media_uri media_uri;
typedef int32_t (*url_equal)(media_uri *url_1, media_uri *url_2);

struct __media_uri
{
	uint32_t mrl_type;	//@{indicate how to parse $mrl}
	uint32_t mrl_ind1;  //@{indicator ... channel}
	uint32_t mrl_ind2;  //@{indicator ... level}
	uint32_t mrl_ind3;  //@{indicator ... client}
	uint32_t mrl_ind4;  //@{indicator ... media}
	uint8_t  mrl[MAX_URL_LEN];
	url_equal equal;
};

typedef struct __media_src media_src;
typedef struct __media_src_ops media_src_ops;

struct __media_src
{
	obj	__super;

	uint32_t state;
	void *medium;
	void *user;
	LOCK_T lock;
	media_src_ops *ops;
};

struct __media_src_ops
{
	int32_t (*init)(media_src *src);
	void	(*fin)(media_src *src);
	void	(*kill)(media_src *src);

	int32_t (*ctl)(media_src *src, int32_t cmd, void *data);

	int32_t (*probe)(media_src *src, media_uri *mrl, media_info *msi);
	int32_t (*open)(media_src *src, media_uri *mrl);
	int32_t (*play)(media_src *src);
	int32_t (*pause)(media_src *src);
	int32_t (*lseek)(media_src *src, uint32_t ts);
};

media_src *media_src_alloc(uint32_t size, media_src_ops *ops, void *u);
media_src *media_src_ref(media_src *src);
void media_src_unref(media_src *src);
void media_src_kill_unref(media_src *src);

int32_t media_src_ctl(media_src *src, int32_t cmd, void *data);
int32_t media_src_probe(media_src *src, media_uri *mrl, media_info *msi);
int32_t media_src_open(media_src *src, media_uri *mrl);
int32_t media_src_play(media_src *src);
int32_t media_src_pause(media_src *src);
int32_t media_src_lseek(media_src *src, uint32_t ts);

int32_t media_src_would_block(media_src *src, int32_t stm_idx, void *data,
	uint32_t size);
int32_t media_src_fill_info(media_src *src, media_info *msi, int32_t err);
int32_t media_src_open_end(media_src *src, int32_t err);

int32_t media_src_produce(media_src *src, int32_t stm_index,
	void *data, uint32_t size);

END_NAMESPACE

#endif	//__TINY_RAIN_MEDIA_SRC_H__

