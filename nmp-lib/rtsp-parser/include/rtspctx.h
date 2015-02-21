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

#ifndef __RTSP_CONTEXT_H__
#define __RTSP_CONTEXT_H__

#include <glib.h>
#include "rtspdefs.h"
#include "rtspmsg.h"

typedef struct _GstRTSPBuilder GstRTSPBuilder;
typedef struct _GstRTSPContext GstRTSPContext;

/* a structure for constructing RTSPMessages */
struct _GstRTSPBuilder			/* code from gstreamer */
{
	gint state;
	GstRTSPResult status;
	guint8 buffer[4096];
	guint offset;

	guint line;
	guint8 *body_data;
	glong body_len;
};

void build_init(GstRTSPBuilder *builder);

void build_reset(GstRTSPBuilder *builder);

GstRTSPContext *gst_rtsp_ctx_new(gsize rb_size, const gchar * initial_buffer);
void gst_rtsp_ctx_free(GstRTSPContext *rtsp_ctx);

GstRTSPResult
gst_rtsp_ctx_recv(GstRTSPContext *rtsp_ctx, gint rfd, GstRTSPBuilder *builder,
	GstRTSPMessage *message, gint *err);

#endif /* __RTSP_CONTEXT_H__ */
