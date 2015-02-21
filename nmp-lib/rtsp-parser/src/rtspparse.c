/* *
 * This file is part of RTSP protocol parser library.
 *
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTIES OR REPRESENTATIONS; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.See the GNU General Public License
 * for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * */
#include "rtspparse.h"
#include "rtspctx.h"
#include "rtspmsg.h"
#include "rtspdefs.h"


GstRtspParser *
gst_rtsp_parser_new(gsize rb_size, const gchar *initial_buffer)
{
	GstRtspParser *parser;

	parser = g_new0(GstRtspParser, 1);
	parser->ctx = gst_rtsp_ctx_new(rb_size, initial_buffer);
	build_init(&parser->builder);
	return parser;
}


GstRTSPResult
gst_rtsp_parser_recv(GstRtspParser *parser, gint fd, GstRTSPMessage **msg, gint *err)
{
	GstRTSPResult res;

	if (parser->msg_complete)
	{
		build_reset(&parser->builder);
		gst_rtsp_message_unset(&parser->message);
		parser->msg_complete = FALSE;
	}

	res = gst_rtsp_ctx_recv(parser->ctx, fd, &parser->builder,
		&parser->message, err);
	if (res == GST_RTSP_OK)
	{
		if (msg)
			*msg = &parser->message;
		parser->msg_complete = TRUE;
	}

	return res;
}


void gst_rtsp_parser_free(GstRtspParser *parser)
{
	g_assert(parser != NULL);

	gst_rtsp_message_unset(&parser->message);
	build_reset(&parser->builder);
	gst_rtsp_ctx_free(parser->ctx);
	g_free(parser);
}

//:~ End
