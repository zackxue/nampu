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
 
 
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "mediautils.h"
#include "demuxer_module.h"
#include "mediaparser.h"
#include "rc_log.h"
#include "macros.h"
#include "standard_frame.h"
#include "stream_api.h"


static const DemuxerInfo info = {
    "Nampu Media File Demuxer",
    "nmpmf",
    "Nampu",
    "",
    "nmpmf"
};

struct file_ops *jmf_file_ops;

gint register_file_ops(struct file_ops *ops)
{
	if (!ops)
		return -EINVAL;

	jmf_file_ops = ops;
	return 0;
}


static gint file_probe(const char *filename)
{
	if (!jmf_file_ops || !jmf_file_ops->probe)
		return -EINVAL;

	return (*jmf_file_ops->probe)(filename);
}


static gint file_open(char *mrl, struct file_ctx *filp_ctx)
{
	if (!jmf_file_ops || !jmf_file_ops->open)
		return -EINVAL;

	return (*jmf_file_ops->open)(mrl, filp_ctx);
}


static void file_close(struct file_ctx *filp_ctx)
{
	if (!jmf_file_ops || !jmf_file_ops->close)
		return;

	(*jmf_file_ops->close)(filp_ctx);
}


static gint file_read_packet(struct file_ctx *ctx, 
	struct file_packet *pkt)
{
	if (!jmf_file_ops || !jmf_file_ops->read)
		return -EINVAL;

	return (*jmf_file_ops->read)(ctx, pkt);
}


static void file_free_packet(struct file_packet *pkt)
{
	if (!jmf_file_ops || !jmf_file_ops->free_packet)
		return;

	(*jmf_file_ops->free_packet)(pkt);
}


static gint jmf_probe(const char *filename)
{
	if (file_probe(filename))
		return RESOURCE_DAMAGED;

	return RESOURCE_OK;
}


static gint jmf_stm_pub_init(Resource *r, struct file_ctx *fp_ctx)
{
	MediaProperties props_hints;
	TrackInfo trackinfo;
	Track *track;
	gdouble length = HUGE_VAL;
	gint stm;

	/* "set range:ntp=0-" */
	if (fp_ctx->length > 0)
		length = fp_ctx->length;
	r->info->duration = length;

	for (stm = 0; stm < fp_ctx->streams; ++stm)
	{
	    memset(&props_hints, 0, sizeof(MediaProperties));
	    memset(&trackinfo, 0, sizeof(TrackInfo));

		switch (fp_ctx->stms[stm].stm_type)
		{
		case STM_VIDEO:

			/* if MP_video, "a=framerate:50.000000" */
			props_hints.media_type = MP_video;
			props_hints.frame_rate = 50;

			/* m= video 0 RTP/AVP 96 */
			trackinfo.rtp_port = 0;
			trackinfo.id = stm;
			props_hints.payload_type = 96;

			/* clock rate 90000 */
			props_hints.clock_rate = 90000;
			strcpy(props_hints.encoding_name, "JMV");

			break;

		case STM_AUDIO:

			props_hints.media_type = MP_audio;

			/* m=audio 0 RTP/AVP 8 */
			trackinfo.rtp_port = 0;
			trackinfo.id = stm;
			props_hints.payload_type = 8;	/* Maybe changed in jma_init() */

			strcpy(props_hints.encoding_name, "JMA");

			break;
		}

		/* "a=control:stream=0" */
		sprintf(trackinfo.name, "%d", stm);

		/* stored stream */
		props_hints.media_source = MS_stored;

		props_hints.extradata = (uint8_t*)fp_ctx->stms[stm].codec;

		trackinfo.mrl = g_strdup_printf("%s%s%s", r->info->mrl,
			SDP_TRACK_URI_SEPARATOR, trackinfo.name);

	    if (!(track = add_track(r, &trackinfo, &props_hints)))
	    {
	    	rc_log(RC_LOG_ERR, "[LS] add_track() failed.");
			return ERR_ALLOC;
	    }
	}

	r->info->media_source = props_hints.media_source;
	return RESOURCE_OK;
}


static gint jmp_stm_pri_init(Resource *r, struct file_ctx *fp_ctx)
{
	MediaProperties props_hints;
	TrackInfo trackinfo;
	Track *track;
	gdouble length = HUGE_VAL;

    memset(&props_hints, 0, sizeof(MediaProperties));
    memset(&trackinfo, 0, sizeof(TrackInfo));

	/* "set range:ntp=0-" */
	if (fp_ctx->length > 0)
		length = fp_ctx->length;
	r->info->duration = length;

	/* "a=control:stream=0" */
	strcpy(trackinfo.name, "0");

	/* live stream */
	props_hints.media_source = MS_stored;

	/* if MP_video, "a=framerate:50.000000" */
	props_hints.media_type = MP_video;
	props_hints.frame_rate = 50;

	/* m= video 0 RTP/AVP 99 */
	trackinfo.rtp_port = 0;
	props_hints.payload_type = 99;

	/* clock rate */
	props_hints.clock_rate = 90000;
	strcpy(props_hints.encoding_name, "JMV");

	props_hints.extradata = (uint8_t*)fp_ctx->stms[0].codec;

	trackinfo.mrl = g_strdup_printf("%s%s%s", r->info->mrl,
		SDP_TRACK_URI_SEPARATOR, trackinfo.name);

    if (!(track = add_track(r, &trackinfo, &props_hints)))
    {
    	rc_log(RC_LOG_ERR, "[LS] add_track() failed.");
		return ERR_ALLOC;
    }

	r->info->media_source = props_hints.media_source;
    return RESOURCE_OK;	
}


static gint jmf_init(Resource *r)
{
	struct file_ctx *ctx;
	gint ret = RESOURCE_OK;

	ctx = g_new0(struct file_ctx, 1);

	if (file_open(r->info->mrl, ctx))
	{
		g_free(ctx);
		return RESOURCE_DAMAGED;
	}

	switch (ctx->file_type)
	{
	case FILE_STM_PUB:
		ret = jmf_stm_pub_init(r, ctx);
		break;

	case FILE_STM_PRI:
		ret = jmp_stm_pri_init(r, ctx);
		break;

	default:
		ret = RESOURCE_DAMAGED;
		break;
	}

	if (ret != RESOURCE_OK)
	{
		file_close(ctx);
		g_free(ctx);
		return ret;
	}

	r->private_data = ctx;
    return ret;
}


static gint jmf_read_packet(Resource * r)
{
	struct file_ctx *ctx;
	struct file_packet pkt;
	TrackList tr_it;
	gint ret;

	ctx = (struct file_ctx*)r->private_data;

	if (file_read_packet(ctx, &pkt) || !pkt.data || !pkt.size)
		return RESOURCE_EOF;

	for (tr_it = g_list_first(r->tracks);
		tr_it != NULL;
		tr_it = g_list_next(tr_it))
	{
        Track *tr = (Track*)tr_it->data;
        if (pkt.stm_index == tr->info->id)
        {
        	tr->properties.dts = pkt.pts;
			tr->properties.pts = pkt.pts - ctx->ts_base;

            ret = tr->parser->parse(tr, (uint8_t*)pkt.data, pkt.size);
            break;
        }
    }

    file_free_packet(&pkt);
    return ret;
}


static gint jmf_seek(Resource * r, double time_sec)
{
	return 0;
}


static void jmf_uninit(gpointer rgen)
{
	Resource *r = (Resource*)rgen;
	struct file_ctx *ctx;

    ctx = (struct file_ctx*)r->private_data;
    if (ctx)
    {
    	file_close(ctx);
    	g_free(ctx);
    	r->private_data = NULL;
    }
}


FNC_LIB_DEMUXER(jmf);
