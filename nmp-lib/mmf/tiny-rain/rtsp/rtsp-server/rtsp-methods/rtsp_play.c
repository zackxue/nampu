#include "rtsp_def.h"
#include "rtsp_impl.h"
#include "tr_log.h"

#include "rtsp_utc.h"

void
rtsp_method_on_play(rtsp_client *rc, rtsp_message *req)
{
	rtsp_message *res;
	char *sid = NULL;
    char *range = NULL;
    char *scale = NULL;
	RTSP_STATUS_CODE rsc;
	int32_t err;
	session *s;

	rtsp_message_get_header(req, RTSP_HDR_SESSION, &sid, 0);

	if (!sid)
	{
		LOG_W(
			"rtsp_impl_on_play(): No session id found."
		);		
		err = -EINVAL;
		goto play_err;
	}

	rtsp_message_get_header(req, RTSP_HDR_RANGE, &range, 0);
	rtsp_message_get_header(req, RTSP_HDR_SCALE, &scale, 0);

	s = client_find_and_get_s((client*)rc, __u8_str(sid));
	if (!s)
	{
		LOG_W(
			"rtsp_impl_on_play(): Session not found."
		);		
		err = -EINVAL;
		goto play_err;		
	}

	res = rtsp_impl_new_generic_response(req, RTSP_STS_OK);

  if(range)
    rtsp_message_add_header(res, RTSP_HDR_RANGE, range);

	rtsp_impl_send_message(rc, res);

	session_ready(s);
    
    if(range)
    {
        char *begin = strstr(range, "clock=");
        
        if(begin)
        {
            begin += strlen("clock=");
            uint32_t utc = rtsp_str2utc(begin);
            LOG_W(
                "rtsp_impl_on_play(): found clock=%s, utc:%u", begin, utc
            );
            
            session_ctl(s, MEDIA_PLAY_SEEK, (void*)utc);
        }
    }
    if(scale)
    {
        float fscale = atof(scale);
       
        LOG_W(
            "rtsp_impl_on_play(): found scale=%s, float:%.1f", scale, fscale
        );
       
        session_ctl(s, MEDIA_PLAY_SCALE, (int)fscale);
    }
    
    session_unref(s);
	return;

play_err:
	rsc = rtsp_impl_trans_status_code(TR_RTSP_PLAY, err);
	res = rtsp_impl_new_generic_response(req, rsc);
	rtsp_impl_send_message(rc, res);
}

void
rtsp_method_on_pause(rtsp_client *rc, rtsp_message *req)
{
	rtsp_message *res;
	char *sid = NULL;
	RTSP_STATUS_CODE rsc;
	int32_t err;
	session *s;

	rtsp_message_get_header(req, RTSP_HDR_SESSION, &sid, 0);

	if (!sid)
	{
		LOG_W(
			"rtsp_impl_on_pause(): No session id found."
		);		
		err = -EINVAL;
		goto pause_err;
	}

	s = client_find_and_get_s((client*)rc, __u8_str(sid));
	if (!s)
	{
		LOG_W(
			"rtsp_impl_on_pause(): Session not found."
		);		
		err = -EINVAL;
		goto pause_err;		
	}

	res = rtsp_impl_new_generic_response(req, RTSP_STS_OK);
	rtsp_impl_send_message(rc, res);

    session_ctl(s, MEDIA_PLAY_PAUSE, 0);
    session_unref(s);
	return;

pause_err:
	rsc = rtsp_impl_trans_status_code(TR_RTSP_PAUSE, err);
	res = rtsp_impl_new_generic_response(req, rsc);
	rtsp_impl_send_message(rc, res);
}



//:~ End
