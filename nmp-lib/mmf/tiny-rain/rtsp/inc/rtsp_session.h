/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_RTSP_SESSION_H__
#define __TINY_RAIN_RTSP_SESSION_H__

#include "session.h"
#include "rtsp_client.h"

BEGIN_NAMESPACE

typedef struct __rtsp_session rtsp_session;
struct __rtsp_session
{
	session __super;

	int32_t timeout;
	rtsp_state state_machine;
};

END_NAMESPACE

#endif	//__TINY_RAIN_RTSP_SESSION_H__
