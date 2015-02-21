/* *
 * This file is part of rtsp-watch lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#ifndef __IO_WATCH_H__
#define __IO_WATCH_H__

#include <glib.h>
#include <rtspparse.h>

#include "nmpev.h"

typedef struct _IO_watch IO_watch;
struct _IO_watch
{
	GEvent base;

    GQueue *output;
    guint8 *write_buffer;
	gsize write_off;
	gsize write_size;
	gsize backlog;
	gint err_code;

	GMutex *mutex;
};


IO_watch *io_watch_new(gsize size, gint fd, gint events);
void io_watch_finalize(GEvent *ev);
gboolean io_watch_write_pending(IO_watch *watch);
GstRTSPResult io_watch_write_data(IO_watch *watch, const guint8 *data, guint size);

//@{Ugly}
GstRTSPResult io_watch_write_message(IO_watch *watch, GstRTSPMessage *message);

#endif	/* __IO_WATCH_H__ */
