#ifndef __RTP_WATCH_H__
#define __RTP_WATCH_H__

#include <glib.h>
#include <rtspparse.h>

#include "evsched.h"
#include "macros.h"

typedef struct _RTP_watch RTP_watch;
struct _RTP_watch
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


RTP_watch *rtp_watch_new(gint fd, gint events);
void rtp_watch_finalize(GEvent *ev);
GstRTSPResult rtp_watch_write_message(RTP_watch *watch, GstRTSPMessage *message);
gboolean rtp_watch_write_pending(RTP_watch *watch);
gboolean rtp_watch_would_block(RTP_watch *watch);

#endif	/* __RTP_WATCH_H__ */
