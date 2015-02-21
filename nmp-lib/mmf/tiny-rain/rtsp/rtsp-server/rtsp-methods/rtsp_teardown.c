#include "rtsp_def.h"
#include "rtsp_impl.h"
#include "tr_log.h"


void
rtsp_method_on_teardown(rtsp_client *rc, rtsp_message *req)
{
	rtsp_message *res;
	char *sid = NULL;
	session *s;

	rtsp_message_get_header(req, RTSP_HDR_SESSION, &sid, 0);
	if (sid)
	{
		s = client_find_and_get_s((client*)rc, __u8_str(sid));
		if (s)
		{
			client_kill_unref_s((client*)rc, s);
		}
	}

	res = rtsp_impl_new_generic_response(req, RTSP_STS_OK);
	rtsp_impl_send_message(rc, res);
}


//:~ End
