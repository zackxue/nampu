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
 
#ifndef __RTSP_METHODS_H__
#define __RTSP_METHODS_H__

#include "rtsp.h"

extern void rtsp_handle_options_request(RTSP_Client *client, RTSP_Ps *state);
extern void rtsp_handle_describe_request(RTSP_Client *client, RTSP_Ps *state);
extern void rtsp_handle_setup_request(RTSP_Client *client, RTSP_Ps *state);
extern void rtsp_handle_play_request(RTSP_Client *client, RTSP_Ps *state);
extern void rtsp_handle_getp_request(RTSP_Client *client, RTSP_Ps *state);
extern void rtsp_handle_setp_request(RTSP_Client *client, RTSP_Ps *state);
extern void rtsp_handle_pause_request(RTSP_Client *client, RTSP_Ps *state);
extern void rtsp_handle_teardown_request(RTSP_Client *client, RTSP_Ps *state);

#endif	//__RTSP_METHODS_H__
 