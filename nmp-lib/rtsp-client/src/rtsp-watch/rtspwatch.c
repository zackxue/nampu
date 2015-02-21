/* *
 * This file is part of rtsp-watch lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#include <string.h>
#include "rtspwatch.h"
#include "nmpevsched.h"

#define RTSP_BUFFER_SIZE		4096
#define RTSP_DEF_CONN_WEIGHT	1


static __inline__ gboolean
gst_rtsp_msg_read(GstRtspWatch *w)
{
	GstRTSPMessage *message = NULL;
	GstRTSPResult res;

	while (TRUE)
	{
		res = gst_rtsp_parser_recv(w->parser, g_event_fd(w),
			&message, &w->err_code);
		if (res == GST_RTSP_OK)
		{
			if (w->funcs.message_received)
			{
				(*w->funcs.message_received)(w, message, w->user_data);
			}

			continue;
		}

		else if (res == GST_RTSP_EINTR)
		{
			return TRUE;
		}

		else if (res == GST_RTSP_EEOF)
		{
			if (w->funcs.closed)
			{
				(*w->funcs.closed)(w, w->user_data);
			}

			break;
		}

		else
		{
			if (w->funcs.error)
			{
				(*w->funcs.error)(w, res, w->user_data);
			}

			break;
		}
	}

    return FALSE;
}


static __inline__ gboolean
gst_rtsp_msg_write(GstRtspWatch *w)
{
	return io_watch_write_pending((IO_watch*)w);
}


static gboolean
gst_rtsp_incoming_cb(GEvent *ev, int revents, void *user_data)
{
	GstRtspWatch *w = (GstRtspWatch*)ev;

	if (revents & EV_READ)
	{
		if (!gst_rtsp_msg_read(w))
			return FALSE;
	}

	if (revents & EV_WRITE)
	{
		if (!gst_rtsp_msg_write(w))
			return FALSE;		
	}

	return TRUE;
}


static void
gst_rtsp_watch_finalize(GEvent *ev)
{
	GstRtspWatch *w = (GstRtspWatch*)ev;

	if (w->fin)
	{
		(*w->fin)(w->user_data);
	}

	if (w->parser)
	{
		gst_rtsp_parser_free(w->parser);
	}

	io_watch_finalize(ev);
}


GstRtspWatch *
gst_rtsp_watch_new_2(gint fd, GstRtspWatchFuncs *funcs, void *user_data,
	GstRtspWatchFin fin)
{
	GstRtspWatch *w;

	w = (GstRtspWatch*)io_watch_new(sizeof(GstRtspWatch),
		fd, EV_READ|EV_ERROR);

	w->parser = gst_rtsp_parser_new(RTSP_BUFFER_SIZE, NULL);
	w->err_code = 0;

	memset(&w->funcs, 0, sizeof(GstRtspWatchFuncs));

	if (funcs)
	{
		w->funcs = *funcs;
	}

	w->user_data = user_data;
	w->fin = fin;

	g_event_set_callback((GEvent*)w, gst_rtsp_incoming_cb,
		w, gst_rtsp_watch_finalize);

	return w;
}


void
gst_rtsp_watch_attach_2(GstRtspWatch *w, void *ctx)
{
	g_scheduler_add((GEvent*)w, RTSP_DEF_CONN_WEIGHT);
}


GstRTSPResult
gst_rtsp_watch_send_message_2(GstRtspWatch *watch, GstRTSPMessage *message)
{
	GString *str;
	guint size;

	g_return_val_if_fail(watch != NULL, GST_RTSP_EINVAL);
	g_return_val_if_fail(message != NULL, GST_RTSP_EINVAL);

	str = gst_rtsp_message_to_string(message);
	size = str->len;

	return io_watch_write_data((IO_watch*)watch,
		 (guint8 *)g_string_free(str, FALSE), size);
}


//:~ End
