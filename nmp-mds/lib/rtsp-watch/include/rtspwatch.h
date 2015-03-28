/* *
 * This file is part of rtsp-watch lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#ifndef __RTSP_WATCH_2_H__
#define __RTSP_WATCH_2_H__

#include <rtspparse.h>
#include "io_watch.h"

typedef struct __GstRtspWatch GstRtspWatch;
typedef struct __GstRtspWatchFuncs GstRtspWatchFuncs;

struct __GstRtspWatchFuncs
{
	GstRTSPResult (*message_received)(GstRtspWatch *watch,
		GstRTSPMessage *message, gpointer user_data);

	GstRTSPResult (*message_sent)(GstRtspWatch *watch, guint id,
		gpointer user_data);

	GstRTSPResult (*closed)(GstRtspWatch *watch, gpointer user_data);

	GstRTSPResult (*error)(GstRtspWatch *watch, GstRTSPResult result,
		gpointer user_data);
};


typedef void (*GstRtspWatchFin)(void *user_data);

struct __GstRtspWatch
{
	IO_watch		base;
	GstRtspParser	*parser;

	gint			err_code;

	GstRtspWatchFuncs funcs;
	gpointer		user_data;
	GstRtspWatchFin fin;	/* On destroy */
};


GstRtspWatch *gst_rtsp_watch_new_2(gint fd, GstRtspWatchFuncs *funcs,
	void *user_data, GstRtspWatchFin fin);

void gst_rtsp_watch_attach_2(GstRtspWatch *w, void *ctx);

GstRTSPResult
gst_rtsp_watch_send_message_2(GstRtspWatch *watch, GstRTSPMessage *message);

gboolean
gst_rtsp_watch_is_full(GstRtspWatch *watch);

#endif	/* __RTSP_WATCH_2_H__ */
