#include "rtsp.h"
#include "rc_log.h"
#include "media.h"


void
rtsp_handle_getp_request(RTSP_Client *client, RTSP_Ps *state)
{
	rtsp_client_send_generic_response(
		client,
		GST_RTSP_STS_OK,
		state
	);
}

void
rtsp_handle_setp_request(RTSP_Client *client, RTSP_Ps *state)
{
	gint ret;

	ret = r_ctrl(state->url->abspath, 0, NULL);
	if (!ret)
	{
		rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_OK,
			state
		);
	}
	else
	{
		rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_NOT_FOUND,
			state
		);		
	}
}
