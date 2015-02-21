/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#ifndef __TINY_RAIN_RTSP_METHODS_H__
#define __TINY_RAIN_RTSP_METHODS_H__

#include "def.h"

BEGIN_NAMESPACE

void rtsp_method_on_option(rtsp_client *rc, rtsp_message *req);
void rtsp_method_on_getparm(rtsp_client *rc, rtsp_message *req);
void rtsp_method_on_setparm(rtsp_client *rc, rtsp_message *req);
void rtsp_method_on_desc(rtsp_client *rc, rtsp_message *req);
void rtsp_method_on_setup(rtsp_client *rc, rtsp_message *req);
void rtsp_method_on_play(rtsp_client *rc, rtsp_message *req);
void rtsp_method_on_teardown(rtsp_client *rc, rtsp_message *req);
void rtsp_method_on_data(rtsp_client *rc, rtsp_message *req);

END_NAMESPACE

#endif	//__TINY_RAIN_RTSP_METHODS_H__