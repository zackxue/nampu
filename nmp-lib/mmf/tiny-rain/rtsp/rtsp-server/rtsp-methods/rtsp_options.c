#include "rtsp_impl.h"


void
rtsp_method_on_option(rtsp_client *rc, rtsp_message *req)
{
	rtsp_message *res;
	const char *str;
	RTSP_METHOD options;

	res = rtsp_impl_new_generic_response(req, RTSP_STS_OK);
	options = RTSP_OPTIONS | RTSP_DESCRIBE | RTSP_SETUP | RTSP_PLAY | 
		RTSP_TEARDOWN | RTSP_SET_PARAMETER | RTSP_GET_PARAMETER;
	str = rtsp_options_as_text(options);
	rtsp_message_add_header(res, RTSP_HDR_PUBLIC, str);
	rtsp_free_string((char*)str);
	rtsp_impl_send_message(rc, res);
}


//:~ End
