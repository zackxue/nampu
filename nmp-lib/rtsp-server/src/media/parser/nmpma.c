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

extern struct file_ops *jmf_file_ops;

static const MediaParserInfo info =			/* Jxj Audio Parser */
{
    "JMA",
    MP_audio
};

#define PCMA_PT	8
#define PCMU_PT 0

struct codec_t
{
	guint32 len1;
	guint32 len2;
	guint32 len3;
	guint32 data[0];
};


static int jma_init(Track *track)
{
	gchar *rtpmap_str, *sdp_rtpmap;
	struct codec_t *code;
	gint pt;
	code = (struct codec_t*)track->properties.extradata;
printf("jma_init()..\n");
	if (code->len3)
	{
		rtpmap_str = (gchar*)&code->data[0] + code->len1 + code->len2;
		sscanf(rtpmap_str, "%d", &pt);
		if (pt == PCMA_PT || pt == PCMU_PT)
			track->properties.payload_type = pt;
		rtpmap_str = strstr(rtpmap_str, " ");	/* skip pt */
		if (rtpmap_str)
		{
			++rtpmap_str;
			sdp_rtpmap = g_strdup(rtpmap_str);
		}
		else
			sdp_rtpmap = g_strdup("PCMX/8000/1");
		strcpy(track->properties.encoding_name, "PCMX");
		track_add_sdp_field(track, rtpmap, sdp_rtpmap);
	}
	else
	{
		strcpy(track->properties.encoding_name, "PCMA");
		sdp_rtpmap = g_strdup_printf("%s/%d/%d",
			track->properties.encoding_name,
			8000 /* sample_rate */, 
			1 /* audio channels */);
		track->properties.payload_type = PCMA_PT;
        track_add_sdp_field(track, rtpmap, sdp_rtpmap);
	}

	return 0;
}


static int jma_parse(Track *tr, uint8_t *data, size_t len)
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
	gdouble duration = .02;

	mparser_buffer_write(tr,
                          timestamp,
                          delivery,
                          duration,
                          0,
                          MP_audio,
                          data, len);

	return RESOURCE_OK;
}


static void jma_uninit(Track *tr)
{
}


FNC_LIB_MEDIAPARSER(jma);
