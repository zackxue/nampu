/* *
 * This file is part of rtsp-client lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#ifndef __RTSP_METHOD_H__
#define __RTSP_METHOD_H__

#include <rtspwatch.h>
#include "rtsp.h"

GstRTSPResult rtsp_session_options(RTSP_client_session *session);
GstRTSPResult rtsp_session_describe(RTSP_client_session *session);
GstRTSPResult rtsp_session_setup(RTSP_client_session *session);
GstRTSPResult rtsp_session_play(RTSP_client_session *session);
GstRTSPResult rtsp_session_teardown(RTSP_client_session *session);

#endif	/* __RTSP_METHOD_H__ */
