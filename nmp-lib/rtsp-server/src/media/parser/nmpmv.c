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

#include "rc_log.h"
#include "demuxer.h"
#include "mediaparser.h"
#include "mediaparser_module.h"
#include "stream_api.h"

extern struct file_ops *jmf_file_ops;

static const MediaParserInfo info =			/* Jxj Video Parser */
{
    "JMV",
    MP_video
};


struct codec_t
{
	guint32 len1;
	guint32 len2;
	guint32 len3;
	guint32 data[0];
};


static int jmv_init(Track *track)
{
	gchar *sdp_rtpmap, *sdp_fmtp, *sdp_pri;
	struct codec_t *code;
	code = (struct codec_t*)track->properties.extradata;
printf("jmv_init()..\n");
	if (code->len1 || code->len2)
	{
		strcpy(track->properties.encoding_name, "H264");

		sdp_rtpmap = g_strdup_printf("%s/%d",
                 track->properties.encoding_name,
                 track->properties.clock_rate);

        track_add_sdp_field(track, rtpmap, sdp_rtpmap);

		sdp_fmtp = g_strdup_printf(
			"packetization-mode=%d;profile-level-id=%s;sprop-parameter-sets=%s",
			1,
			(gchar*)&code->data[0] + code->len1,
			(gchar*)&code->data[0]
		);

    	track_add_sdp_field(track, fmtp, sdp_fmtp);
	}
	else
	{
		strcpy(track->properties.encoding_name, "JPF-GENERIC");

	    sdp_rtpmap = g_strdup_printf ("%s/%d",
	         track->properties.encoding_name,
	         track->properties.clock_rate);

		track_add_sdp_field(track, rtpmap, sdp_rtpmap);

		sdp_pri = g_strdup_printf("decode-header=%s",
			(gchar*)&code->data[0] + code->len1 + code->len2);

		track_add_sdp_field(track, pri, sdp_pri);		
	}

	return 0;
}


static int jmv_parse(Track *tr, uint8_t *data, size_t len)
{
/*
void mparser_buffer_write(Track *tr,
                          double presentation,
                          double delivery,
                          double duration,
                          gboolean marker,
                          gint mtype,
                          guint8 *data, gsize data_size);
                          */
	gdouble timestamp = tr->properties.dts;
	gdouble delivery = tr->properties.pts / 90000;		/* inc * 90000HZ */
	gdouble duration = .02;	/* 33ms */

	mparser_buffer_write(tr,
                          timestamp,
                          delivery,
                          duration,
                          0,
                          MP_video,
                          data, len);

	return RESOURCE_OK;
}


static void jmv_uninit(Track *tr)
{
}


FNC_LIB_MEDIAPARSER(jmv);
