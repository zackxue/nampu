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
 
#ifndef __RTSP_PARSE_H__
#define __RTSP_PARSE_H__

#include "rtspctx.h"
#include "sdp.h"
#include "rtspurl.h"
#include "rtsptransport.h"

typedef struct _GstRtspParser GstRtspParser;
struct _GstRtspParser
{
	GstRTSPContext		*ctx;
	GstRTSPBuilder		builder;
	GstRTSPMessage		message;
	gboolean			msg_complete;
};

GstRtspParser *gst_rtsp_parser_new(gsize rb_size, const gchar *initial_buffer);
void gst_rtsp_parser_free(GstRtspParser *parser);
GstRTSPResult gst_rtsp_parser_recv(GstRtspParser *parser, gint fd, GstRTSPMessage **msg, gint *err);

#endif	/* __RTSP_PARSE_H__ */
