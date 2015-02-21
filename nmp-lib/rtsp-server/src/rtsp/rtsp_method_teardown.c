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
rtsp_handle_teardown_request(RTSP_Client *client, RTSP_Ps *state)
{
	GstRTSPResult res;
	gchar *session_id;
	RTSP_session *rtsp_s;

	res = gst_rtsp_message_get_header(state->request,
		GST_RTSP_HDR_SESSION, &session_id, 0);
	if (res != GST_RTSP_OK)
	{
		rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_SESSION_NOT_FOUND,
			state
		);
		return;		
	}

	rtsp_s = rtsp_client_session_find(client, session_id);
	if (G_UNLIKELY(!rtsp_s))
	{//@{Session alreay destroyed, nothing to do}
		goto handle_ok;
	}

	rtsp_client_session_remove(client, rtsp_s);
	rtsp_session_kill(rtsp_s);
	rtsp_session_unref(rtsp_s);

handle_ok:
	rtsp_client_send_generic_response(
		client,
		GST_RTSP_STS_OK,
		state
	);
}

//:~ End
