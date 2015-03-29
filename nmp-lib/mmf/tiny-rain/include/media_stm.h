/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_MEDIA_STM_H__
#define __TINY_RAIN_MEDIA_STM_H__

#include "def.h"

BEGIN_NAMESPACE

#define RANGE_MAX				64
#define MAX_DESCP_LEN			32
#define MAX_EMAIL_LEN			32
#define MAX_PHONE_LEN			32
#define ENCODING_NAME_MAX		16
#define MAX_STM_PRI_FIELDS		3
#define PTS_OF_STM				2

#define VALID_STM_TYPE(stm_type) ((stm_type) > ST_NONE && (stm_type) < ST_MAX)

enum
{
	ST_NONE = -1,
	ST_VIDEO,
	ST_AUDIO,
	ST_MAX
};

enum
{
	FIELD_EMPTY = 0,
	FIELD_FMTP,
	FILED_PRI
};

enum
{
	G711A = 0,
	G711U
};

enum
{
	H264 = 0,
	MPEG4
};

typedef struct __stm_info_pri stm_into_pri;
struct __stm_info_pri
{
	int32_t field_type;
	void *data;
	uint32_t size;
};

typedef struct __stm_info stm_info;
struct __stm_info
{
	int32_t stm_type;
	uint32_t clock_rate;
	uint32_t bit_rate;
	uint32_t sample_rate;
	uint32_t bits_per_sample;
	uint32_t frame_rate;
	uint32_t fmt;	/* H264, G711A/G711U */
	uint8_t encoding_name[ENCODING_NAME_MAX];

	union {
		struct {
			int32_t audio_channels;
		}audio;

		struct {
			uint32_t pixel_width;
			uint32_t pixel_height;
		}video;
	};

	uint32_t fields;
	stm_into_pri pri_fields[MAX_STM_PRI_FIELDS];
};

typedef struct __pt_map pt_map;
struct __pt_map
{
	uint8_t *proto;
	int32_t pt[PTS_OF_STM];
};

static pt_map __proto_pt_maps[] =
{
	{__u8_str("RTP/AVP"), {96, 97}},	/* H264, MPEG4 */
	{__u8_str("RTP/AVP"), {8, 0}}	    /* G711A, G711U */
};

static __inline__ int32_t
__stm_pt(int32_t type, int32_t fmt)
{
	return __proto_pt_maps[type].pt[fmt];
}

static __inline__ uint8_t*
__stm_proto(int32_t type)
{
	return __proto_pt_maps[type].proto;
}

static __inline__ uint8_t*
__stm_name(int32_t type)
{
	uint8_t *name[] = {__u8_str("video"), __u8_str("audio")};
	return name[type];
}

typedef struct __media_info media_info;
struct __media_info
{
	stm_info stms[ST_MAX];
	uint32_t n_stms;

	uint8_t range[RANGE_MAX];
	uint8_t descp[MAX_DESCP_LEN];
	uint8_t email[MAX_EMAIL_LEN];
	uint8_t phone[MAX_PHONE_LEN];
};

void media_info_init(media_info *mi);
void media_info_dup(media_info *dst, media_info *src);
void media_info_clear(media_info *mi);

END_NAMESPACE

#endif	//__TINY_RAIN_MEDIA_STM_H__
