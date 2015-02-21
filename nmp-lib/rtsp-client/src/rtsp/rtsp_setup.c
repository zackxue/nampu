/* *
 * This file is part of rtsp-client lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#include "rtsp_method.h"
#include "media.h"
#include "rtsp.h"
#include "fnc_log.h"


GstRTSPResult rtsp_session_setup(RTSP_client_session *session)
{
	GstRTSPMessage *message;
	GstRTSPResult res;
	gchar *uri, *cseq, *transport;
	gint nseq;
	const GstSDPMedia *m;
	RTSP_media *medium;
	RTP_Session *rtp_s;
	gint rtp_port, rtcp_port;

	medium = session->medium;
	m = gst_sdp_message_get_media(medium->sdp, medium->request_itrack++);

	rtp_s = rtsp_media_create_rtp_session(medium, m);
	if (!rtp_s)
	{
		fnc_log(FNC_LOG_ERR, "[FC] Create RTP session failed!");
		return GST_RTSP_ERROR;				
	}

	uri = g_strdup_printf("%s%s", medium->content_base,
		gst_sdp_media_get_attribute_val(m, "control"));

	GST_RTSP_CHECK(
		gst_rtsp_message_new_request(&message, 
		GST_RTSP_SETUP,
		uri
		),
		no_setup_message
	);

	nseq = ++session->rtsp_seq;
	cseq = g_strdup_printf("%d", nseq);

	gst_rtsp_message_add_header(message,
		GST_RTSP_HDR_CSEQ,
		cseq
	);

	rtp_session_port(rtp_s, &rtp_port, &rtcp_port);

	if (!session->rtp_over_rtsp)
	{
		transport = g_strdup_printf(		/* udp */
			"RTP/AVP;unicast;client_port=%d-%d",
			rtp_port,
			rtcp_port
		);
	}
	else
	{
		transport = g_strdup_printf(		/* rtp over rtsp */
			"RTP/AVP/TCP;unicast;interleaved=%d-%d",
			rtp_port,
			rtcp_port
		);
	}

	gst_rtsp_message_add_header(message,
		GST_RTSP_HDR_TRANSPORT,
		transport
	);

	if (session->session_id)
	{
		gst_rtsp_message_add_header(message,
			GST_RTSP_HDR_SESSION,
			session->session_id	
		);
	}

	res = rtsp_sesseion_send_message(session, message);
	gst_rtsp_message_free(message);

	if (res == GST_RTSP_OK)
	{
		session->wait_for.seq = nseq;
		session->wait_for.method = GST_RTSP_SETUP;
	}

	g_free(transport);
	g_free(cseq);
	g_free(uri);

	return res;

no_setup_message:
	g_free(uri);
	return GST_RTSP_ENOMEM;
}


//:~ End
