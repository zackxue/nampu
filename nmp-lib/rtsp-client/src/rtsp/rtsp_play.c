/* *
 * This file is part of rtsp-client lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#include "rtsp_method.h"
#include "media.h"
#include "rtsp.h"
#include "fnc_log.h"


GstRTSPResult rtsp_session_play(RTSP_client_session *session)
{
	GstRTSPMessage *message;
	GstRTSPResult res;
	gchar *uri, *cseq/*, *range*/;
	gint nseq;
	RTSP_media *medium;

	medium = session->medium;
	uri = g_strdup_printf("%s", medium->content_base);

	GST_RTSP_CHECK(
		gst_rtsp_message_new_request(&message, 
		GST_RTSP_PLAY,
		uri
		),
		no_play_message
	);

	nseq = ++session->rtsp_seq;
	cseq = g_strdup_printf("%d", nseq);

	gst_rtsp_message_add_header(message,
		GST_RTSP_HDR_CSEQ,
		cseq
	);

	gst_rtsp_message_add_header(message,
		GST_RTSP_HDR_RANGE,
		"npt=0.000-"
	);

	gst_rtsp_message_add_header(message,
		GST_RTSP_HDR_SESSION,
		session->session_id
	);

	res = rtsp_sesseion_send_message(session, message);
	gst_rtsp_message_free(message);

	if (res == GST_RTSP_OK)
	{
		session->wait_for.seq = nseq;
		session->wait_for.method = GST_RTSP_SETUP;
	}

	g_free(cseq);
	g_free(uri);
	return res;

no_play_message:
	g_free(uri);
	return GST_RTSP_ENOMEM;
}


//:~ End
