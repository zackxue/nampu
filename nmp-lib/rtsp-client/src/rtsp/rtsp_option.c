/* *
 * This file is part of rtsp-client lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#include "rtsp_method.h"
#include "media.h"
#include "rtsp.h"
#include "fnc_log.h"


GstRTSPResult rtsp_session_options(RTSP_client_session *session)
{
	GstRTSPMessage *message;
	GstRTSPResult res;
	gchar *uri, *cseq;
	gint nseq;

	uri = g_strdup_printf("rtsp://%s:%s%s", session->url->hostname,
		session->url->port, session->url->path);

	GST_RTSP_CHECK(
		gst_rtsp_message_new_request(&message, 
		GST_RTSP_OPTIONS,
		uri
		),
		no_options_message
	);

	nseq = ++session->rtsp_seq;
	cseq = g_strdup_printf("%d", nseq);

	gst_rtsp_message_add_header(message,
		GST_RTSP_HDR_CSEQ,
		cseq
	);

	gst_rtsp_message_add_header(message,
		GST_RTSP_HDR_ACCEPT,
		"application/sdp"
	);

	res = rtsp_sesseion_send_message(session, message);
	gst_rtsp_message_free(message);

	if (res == GST_RTSP_OK)
	{
		session->wait_for.seq = nseq;
		session->wait_for.method = GST_RTSP_OPTIONS;
	}

	g_free(cseq);
	g_free(uri);

	return res;

no_options_message:
	g_free(uri);
	return GST_RTSP_ENOMEM;
}


//:~ End
