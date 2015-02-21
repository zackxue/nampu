#include "rtsp_def.h"
#include "rtsp_impl.h"
#include "tr_log.h"


void
rtsp_method_on_getparm(rtsp_client *rc, rtsp_message *req)
{
	rtsp_message *res;

	res = rtsp_impl_new_generic_response(req, RTSP_STS_OK);
	rtsp_impl_send_message(rc, res);
}


//:~ End
