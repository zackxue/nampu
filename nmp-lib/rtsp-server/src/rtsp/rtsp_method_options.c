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


#include "rtsp.h"
#include "rc_log.h"

void
rtsp_handle_options_request(RTSP_Client *client, RTSP_Ps *state)
{
	GstRTSPMethod options;
	gchar *str;

	rtsp_client_remove_dead_session(client);
	options = GST_RTSP_DESCRIBE |
		GST_RTSP_OPTIONS |
		GST_RTSP_PAUSE |
		GST_RTSP_PLAY |
		GST_RTSP_SETUP |
		GST_RTSP_GET_PARAMETER | GST_RTSP_SET_PARAMETER | GST_RTSP_TEARDOWN;

	str = gst_rtsp_options_as_text(options);

	gst_rtsp_message_init_response(
		state->response, 
		GST_RTSP_STS_OK,
		gst_rtsp_status_as_text(GST_RTSP_STS_OK),
		state->request
	);

	gst_rtsp_message_add_header(state->response, GST_RTSP_HDR_PUBLIC, str);
	g_free(str);

	rtsp_client_send_response(client, NULL, state->response);
}

//:~ End
