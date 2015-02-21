#include "rtsp_def.h"
#include "rtsp_impl.h"
#include "tr_log.h"


static __inline__ int32_t
rtsp_method_do_set_parm(rtsp_client *rc, rtsp_message *req)
{
	media *media;
	media_uri mrl;
	const char *url = NULL;

	rtsp_message_parse_request(req, NULL, &url, NULL, NULL);
	if (!url)
	{
		LOG_W(
			"rtsp_method_do_set_parm()->rtsp_message_parse_request() failed."
		);
		return -EINVAL;
	}

	if (rtsp_impl_parse_url(rc, url, &mrl, NULL, 0))
	{
		LOG_I(
			"Invalid request url '%s'.", url
		);
		return -EINVAL;
	}

	media = get_media(&mrl);
	if (!media)
	{
		LOG_W(
			"Get media object failed, url '%s'.",url
		);
		return -ENOMEM;
	}

	media_ctl(media, MEDIA_FORCE_IFRAME, NULL);
	media_unref(media);

	return 0;
}


void
rtsp_method_on_setparm(rtsp_client *rc, rtsp_message *req)
{
	rtsp_message *res;

	rtsp_method_do_set_parm(rc, req);
	res = rtsp_impl_new_generic_response(req, RTSP_STS_OK);
	rtsp_impl_send_message(rc, res);
}


//:~ End
