/* *
 * This file is part of rtsp-server
 *
 * Copyright (C) 2012 fangyi <fangyi@szjxj.net>
 * See COPYING for more details
 *
 * rtsp-server is a stream transporting and controlling module using
 * RTP/RTCP/RTSP protocols, which is designed for jxj platform servers
 * such as mss, proxy-server etc.
 *
 * */

#include <stdlib.h>
#include <arpa/inet.h>
#include "mediautils.h"
#include "demuxer_module.h"
#include "mediaparser.h"
#include "rc_log.h"
#include "macros.h"
#include "standard_frame.h"
#include "stream_api.h"

#define MAX_RTP_PAYLOAD		1200
#define CLOCK_RATE	90000
#define PL_SIZE  (MAX_RTP_PAYLOAD - sizeof(EL))
#define PT_TYPE				99
#define ENCODING_NAME		"JPF-GENERIC"
#define MAX_PUID_SIZE		32
#define ALIGN(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#define F_FRAME_MAGIC 0x6a786a2d
#define URL_REGEX \
		"/+dev=[A-Z]{3}-[A-Z]{3}-\\d{8}/+media=\\d+/+channel=\\d+&level=\\d+/+seq=\\d+";


enum
{
	MS_TYPE_LIVE		= 0,
	MS_TYPE_VOD			= 2,
	MS_TYPE_DOWNLOAD	= 3
};

static const DemuxerInfo info = {
    "Nampu Live Stream Demuxer",
    "ls",
    "NMP",
    "",
    "ls"
};

typedef struct _EL EL;
struct _EL                   //@{Encapsulation Layer for network transmission}
{
    uint8_t magic;          //数据检测的幻数：0xAA
    uint8_t frame_no;       //同一帧拆开的片具有相同的frame_no
    uint8_t total_frags;    //总共分成了几片
    uint8_t frag_no;        //当前是第几片,从1开始
};

static Stream_operation ls_regitered_stream_ops;


/**
 * @brief Fake parser description for demuxer_sd tracks
 *
 * This object is used to free the slice in Track::private_data as a
 * mqd_t object.
 */
static MediaParser demuxer_ls_fake_mediaparser = {
    .info = NULL,
    .init = NULL,
    .parse = NULL,
    .uninit = NULL
};


void register_stream_operations(Stream_operation *ops)
{
	if (!ops)
		return;

	ls_regitered_stream_ops = *ops;
}


void set_stream_user_data(gpointer stream, void *u)
{
	BufferQueue_Producer *producer = (BufferQueue_Producer*)stream;
	bq_set_user_data(producer, u);
}


void *get_stream_user_data(gpointer stream)
{
	BufferQueue_Producer *producer = (BufferQueue_Producer*)stream;
	return bq_get_user_data(producer);
}


gint test_stream_blockable(gpointer stream, gsize size)
{
	gint packets;
	BufferQueue_Producer *producer = (BufferQueue_Producer*)stream;

	packets = (size + MAX_RTP_PAYLOAD -1 ) / MAX_RTP_PAYLOAD;
	return bq_test_blockable(producer, packets + 1);
}


gpointer stream_handle_ref(gpointer stream)
{
	BufferQueue_Producer *producer = (BufferQueue_Producer*)stream;

	if (producer)
	{
		bq_producer_ref(producer);
	}
	return stream;
}


void stream_handle_unref(gpointer stream)
{
	BufferQueue_Producer *producer = (BufferQueue_Producer*)stream;
	bq_producer_unref(producer);
}


static int ls_probe(const char *filename)
{
	const gchar *regex = URL_REGEX;

	if (!filename || !regex_string_match(filename, regex))
		return RESOURCE_DAMAGED;

	return RESOURCE_OK;
}


static int ls_init(Resource * r)		/* ls uses 1 track */
{
	gchar *mi_base64, *mi = NULL;
	gsize mi_size = 0;
	Track *track;
	gchar puid[MAX_PUID_SIZE] = {0};
	gdouble length = HUGE_VAL;

	MediaProperties props_hints;
	TrackInfo trackinfo;

	if (r->mode == OM_DESC && ls_regitered_stream_ops.init)
	{
		gint ch, level, rc, media_type = 0;
		gchar *v;

		v = get_param_string(r->info->mrl, "media=\\d+");
		if (v)
		{
			media_type = atoi(v + 6);
			g_free(v);
		}

		v = get_param_string(r->info->mrl, "dev=[A-Z]{3}-[A-Z]{3}-\\d{8}");
		if (v)
		{
			strncpy(puid, v + 4, MAX_PUID_SIZE - 1);
			g_free(v);

			v = get_param_string(r->info->mrl, "channel=\\d+");
			if (v)
			{
				sscanf(v, "channel=%d", &ch);
				g_free(v);

				v = get_param_string(r->info->mrl, "level=\\d+");
				if (v)
				{
					sscanf(v, "level=%d", &level);
					g_free(v);

				    if (media_type == MS_TYPE_DOWNLOAD || media_type == MS_TYPE_VOD)
	  				{
						v = get_param_string(r->info->mrl, "recordType=\\d+&startTime=\\d{14}+&endTime=\\d{14}+");
						if (v)
						{
							rc = (*ls_regitered_stream_ops.init)(puid, media_type, ch, level, v, &mi, &mi_size, &length);
							if (rc)
							{
								g_free(v);
								return ERR_NOT_FOUND;
							}
							g_free(v);
						}
					}
					else if (media_type == MS_TYPE_LIVE)
					{
						rc = (*ls_regitered_stream_ops.init)(puid, media_type, ch, level, v ? v+8 : NULL, &mi, &mi_size, &length);
						if (rc)
						{
							//g_free(v);
							return ERR_NOT_FOUND;
						}

						//g_free(v);
					}
				}
			}
		}
	}

    memset(&props_hints, 0, sizeof(MediaProperties));
    memset(&trackinfo, 0, sizeof(TrackInfo));

	/* "set range:ntp=0-" */
	r->info->duration = length > 0 ? length : HUGE_VAL;

	/* "a=control:stream=0" */
	strcpy(trackinfo.name, "0");

	/* live stream */
	props_hints.media_source = MS_live;

	/* if MP_video, "a=framerate:50.000000" */
	props_hints.media_type = MP_video;
	props_hints.frame_rate = 50;

	/* m= video 0 RTP/AVP 99 */
	trackinfo.rtp_port = 0;
	props_hints.payload_type = PT_TYPE;

	/* clock rate */
	props_hints.clock_rate = CLOCK_RATE;
	strcpy(props_hints.encoding_name, ENCODING_NAME);

	trackinfo.mrl = g_strdup_printf("%s%s%s", r->info->mrl,
		SDP_TRACK_URI_SEPARATOR, trackinfo.name);

    if (!(track = add_track(r, &trackinfo, &props_hints)))
    {
    	rc_log(RC_LOG_ERR, "[LS] add_track() failed.");
		return ERR_ALLOC;
    }

	track->parser = &demuxer_ls_fake_mediaparser;

    if (props_hints.payload_type >= 96)
    {
        char *sdp_value = NULL;
        switch (props_hints.media_type)
        {
        case MP_audio:
            sdp_value = g_strdup_printf ("%s/%d/%d",
                 props_hints.encoding_name,
                 props_hints.clock_rate,
                 props_hints.audio_channels);
            break;

        case MP_video:
            sdp_value = g_strdup_printf ("%s/%d",
                 props_hints.encoding_name,
                 props_hints.clock_rate);
            break;

        default:
            break;
        }

        track_add_sdp_field(track, rtpmap, sdp_value);

		if (mi && mi_size)
		{
			mi_base64 = g_base64_encode((const guchar*)mi, mi_size);
			if (mi_base64)
			{
				sdp_value = g_strdup_printf("decode-header=%s", mi_base64);
				track_add_sdp_field(track, pri, sdp_value);
				g_free(mi_base64);
			};
			g_free(mi);
		}
    }

	r->info->media_source = props_hints.media_source;

    return RESOURCE_OK;
}


static int ls_read_packet(Resource * r)
{
	/* nothing to do for push-style stream mode */
    return RESOURCE_OK;
}

struct seek_block
{
	gint ret;
	gdouble ts;
};

static void seek_track(gpointer element, gpointer user_data)
{
    Track *track = (Track*)element;
    struct seek_block *sb = (struct seek_block*)user_data;

	if (sb->ret)
		return;

	sb->ret = (*ls_regitered_stream_ops.seek)(track->producer, sb->ts);
}


static int ls_seek(Resource *r, gdouble time_sec)
{
	struct seek_block sb;

	if (!ls_regitered_stream_ops.seek)
		return -1;

	sb.ret = 0;
	sb.ts = time_sec;

	g_list_foreach(r->tracks, seek_track, &sb);
	return sb.ret;
}


static void ls_uninit(gpointer ptr)
{
    return;
}


static gint ls_ctrl_track(const gchar *mrl, gint cmd, void *value)
{
	gint channel, level;
	gchar *v;
	gchar puid[MAX_PUID_SIZE] = {0};

	v = get_param_string(mrl, "dev=[A-Z]{3}-[A-Z]{3}-\\d{8}");
	if (v)
	{
		strncpy(puid, v + 4, MAX_PUID_SIZE - 1);
		g_free(v);

		v = get_param_string(mrl, "channel=\\d+");
		if (v)
		{
			sscanf(v, "channel=%d", &channel);
			g_free(v);

			v = get_param_string(mrl, "level=\\d+");
			if (v)
			{
				sscanf(v, "level=%d", &level);
				g_free(v);

				if (ls_regitered_stream_ops.ctrl)
				{
					return (*ls_regitered_stream_ops.ctrl)(puid, channel, level, cmd, value);
				}
			}
		}
	}

	return -1;
}


static void ls_track_start(BufferQueue_Producer *producer, gchar *uri)
{
	gint ch, level, media_type = 0;
	gchar *v;
	gchar puid[MAX_PUID_SIZE] = {0};

	if (!ls_regitered_stream_ops.open)
		return;

	v = get_param_string(uri, "dev=[A-Z]{3}-[A-Z]{3}-\\d{8}");
	if (!v)
		return;

	strncpy(puid, v + 4, MAX_PUID_SIZE - 1);
	g_free(v);

	v = get_param_string(uri, "channel=\\d+");
	if (!v)
		return;

	sscanf(v, "channel=%d", &ch);
	g_free(v);

	v = get_param_string(uri, "level=\\d+");
	if (!v)
		return;

	sscanf(v, "level=%d", &level);
	g_free(v);

	v = get_param_string(uri, "media=\\d+");
	if (v)
	{
		media_type = atoi(v + 6);
		g_free(v);
	}

	//v = get_param_string(uri, "private=.*[\\\\s]");
	if (media_type == MS_TYPE_DOWNLOAD || media_type == MS_TYPE_VOD)
	{
		v = get_param_string(uri, "recordType=\\d+&startTime=\\d{14}+&endTime=\\d{14}+");
		if (v)
		{
			(*ls_regitered_stream_ops.open)(producer, puid, media_type, ch, level, v);
			g_free(v);
		}
	}
	else if (media_type == MS_TYPE_LIVE)
	{
		(*ls_regitered_stream_ops.open)(producer, puid, media_type, ch, level,  v ? v+8 : NULL);
	}

		//(*ls_regitered_stream_ops.open)(producer, puid, media_type, ch, level, v ? v+8 : NULL);
		//g_free(v);
}


static void ls_track_stop(BufferQueue_Producer *producer, gchar *uri)
{
	if (!ls_regitered_stream_ops.close)
		return;

	(*ls_regitered_stream_ops.close)(producer);
}


static void ls_track_play(BufferQueue_Producer *producer, gchar *uri)
{
	if (!ls_regitered_stream_ops.play)
		return;

	(*ls_regitered_stream_ops.play)(producer);
}


static void ls_track_pause(BufferQueue_Producer *producer, gchar *uri)
{
	if (!ls_regitered_stream_ops.pause)
		return;

	(*ls_regitered_stream_ops.pause)(producer);
}


gint ls_write_data(BufferQueue_Producer *producer,
                   gdouble timestamp,
                   gdouble delivery,
                   gdouble duration,
                   gchar *buffer, gsize size)
{
	gulong seq;
	guint8 frag[PL_SIZE + sizeof(EL)];
	gint ret = 0, frags, i = 1;
	MParserBuffer *mp_buffer;
	gsize cur_size;

	frags = (size + PL_SIZE - 1) / PL_SIZE;
	seq = bq_producer_get_total(producer);

	while (size > 0)
	{
		cur_size = size;
		if (cur_size > PL_SIZE)
			cur_size = PL_SIZE;

		((EL*)frag)->magic = 0xAA;
		((EL*)frag)->frame_no = seq;
		((EL*)frag)->total_frags = frags;
		((EL*)frag)->frag_no = i;

		memcpy(frag + sizeof(EL), buffer, cur_size);

		mp_buffer = alloc_mparser_buffer(timestamp,
									   delivery,
									   duration,
									   size - cur_size > 0 ? 0 : 1,
									   MP_video,
									   frag,
									   cur_size + sizeof(EL));

		ret = mparser_buffer_write_buffer(producer, mp_buffer);
		if (ret != E_OK)
		{
			free_mparser_buffer(mp_buffer);
			break;
		}

		++i;
		buffer += cur_size;
		size -= cur_size;
	}

	return ret;
}


gint write_stream_data(gpointer stream, 
                       gulong timestamp,
                       gulong duration,
                       guint8 *data, gsize data_size, guint8 data_type,
                       guint8 *ext, gsize ext_size, guint8 ext_type)
{
	gchar *frame_data;
	gsize frame_size;
	standard_frame_t *frame;
	frame_ext_t *frame_ext;
	gint ret;
	BufferQueue_Producer *producer;
	gdouble ts = ((gdouble)timestamp)/1000.;
	gdouble dur = ((gdouble)duration)/1000.;
	producer = (BufferQueue_Producer*)stream;

	if (!data && !ext)
	{//EOF
		return mparser_buffer_write_eof(producer);
	}

	frame_size = data_size + sizeof(standard_frame_t);

	if (ext)
	{
		frame_size += sizeof(frame_ext_t);
		frame_size += ALIGN(ext_size, 4);
	}

	frame = (standard_frame_t*)g_malloc(frame_size);
	frame->magic = htonl(F_FRAME_MAGIC);
	frame->frame_type = data_type;
	frame->frame_ext = ext ? (ext_type ? ext_type : -1) : 0;
	frame->padding[0] = frame->padding[1] = 0;
	frame->frame_dev = htonl(F_DEV_HIK);
	frame->frame_size = htonl(frame_size);
	frame->time_stamp = htonl(timestamp);

	frame_data = (gchar*)frame->data;

	if (ext)
	{
		frame_ext = (frame_ext_t*)frame_data;
		frame_ext->ext_size = htonl(ext_size);
		memcpy(frame_ext->data, ext, ext_size);

		frame_data += sizeof(frame_ext_t) + ALIGN(ext_size, 4);
	}

	memcpy(frame_data, data, data_size);
	ret = ls_write_data(producer, ts, ts, dur, (gchar*)frame, frame_size);
	g_free(frame);

	return ret;	
}


FNC_LIB_EXT_DEMUXER(ls);

//:~ End
