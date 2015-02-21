#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "rtp_watch.h"

#define MAX_DATA_PENDING		(4096*32)

typedef struct IO_rec
{
	guint8	*data;
	guint	size;	
}IO_rec;


RTP_watch *
rtp_watch_new(gint fd, gint events)
{
	RTP_watch *w;

	w = (RTP_watch*)g_event_new(sizeof(RTP_watch),
		fd, events);

	w->output = g_queue_new();
	w->write_buffer = NULL;
	w->write_off = 0;
	w->write_size = 0;
	w->backlog = 0;
	w->err_code = 0;
	w->mutex = g_mutex_new();

	return w;
}


static void
rtp_watch_free_rec(gpointer data, gpointer user_data)
{
	IO_rec *rec = (IO_rec*)data;

	g_free(rec->data);
	g_slice_free(IO_rec, rec);
}


static __inline__ void
rtp_watch_destroy_output_queue(GQueue *output)
{
	if (!output)
		return;
	g_queue_foreach(output, rtp_watch_free_rec, NULL);
	g_queue_free(output);	
}


void
rtp_watch_finalize(GEvent *ev)
{
	RTP_watch *watch = (RTP_watch*)ev;

	g_free(watch->write_buffer);
	rtp_watch_destroy_output_queue(watch->output);
	g_mutex_free(watch->mutex);
}


static __inline__ GstRTSPResult
rtp_watch_write_bytes(gint fd, const guint8 *buffer, guint *idx,
	guint size, gint *err)
{
	guint left;

	if (G_UNLIKELY (*idx > size))
	{
		*err = EINVAL;
		return GST_RTSP_ERROR;
	}

	left = size - *idx;

	while (left)
	{
		gint r;

		r = send(fd, &buffer[*idx], left, MSG_DONTWAIT);
		if (G_UNLIKELY(r == 0))
			return GST_RTSP_EINTR;
		else if (G_UNLIKELY(r < 0))
		{
			*err = errno;
			if (*err == EAGAIN)
				return GST_RTSP_EINTR;

			if (*err != EINTR)
				return GST_RTSP_ESYS;

			*err = 0;
		}
		else
		{
			left -= r;
			 *idx += r;
		}
  	}

	return GST_RTSP_OK;
}


static __inline__ GstRTSPResult
rtp_watch_write_data(RTP_watch *watch, const guint8 *data, guint size)
{
	GstRTSPResult res;
	IO_rec *rec;
	guint off = 0;
	gboolean pending = FALSE;

	g_mutex_lock(watch->mutex);

  	if (!watch->write_buffer && !watch->output->length)
	{
		res = rtp_watch_write_bytes(g_event_fd(watch), data, &off, size,
			&watch->err_code);
		if (res != GST_RTSP_EINTR)
		{
			g_free((gpointer)data);
			goto done;
    	}
  	}

	if (watch->backlog >=  MAX_DATA_PENDING)
	{
		res = GST_RTSP_EINTR;
		g_free((gpointer)data);
		goto done;
	}

	rec = g_slice_new(IO_rec);
	if (off == 0)
	{
		rec->data = (guint8*)data;
		rec->size = size;
	}
	else
	{
		rec->data = g_memdup(data + off, size - off);
		rec->size = size - off;
		g_free((gpointer)data);
	}

	g_queue_push_head(watch->output, rec);
	watch->backlog += rec->size;
	pending = TRUE;
	res = GST_RTSP_OK;

done:
	g_mutex_unlock(watch->mutex);

	if (pending)
		g_event_add_events((GEvent*)watch, EV_WRITE);

	return res;
}


GstRTSPResult
rtp_watch_write_message(RTP_watch *watch, GstRTSPMessage *message)
{
	GString *str;
	guint size;

	g_return_val_if_fail (watch != NULL, GST_RTSP_EINVAL);
	g_return_val_if_fail (message != NULL, GST_RTSP_EINVAL);

	str = gst_rtsp_message_to_string(message);
	size = str->len;

	return rtp_watch_write_data(watch, (guint8 *)g_string_free(str, FALSE), size);
}


gboolean
rtp_watch_write_pending(RTP_watch *watch)
{
	GstRTSPResult res;
	IO_rec *rec;

	g_mutex_lock(watch->mutex);

	do {
		if (!watch->write_buffer)
		{
			rec = g_queue_pop_tail(watch->output);
			if (rec == NULL)
			{
				g_mutex_unlock(watch->mutex);
				g_event_remove_events_sync((GEvent*)watch, EV_WRITE);
				return TRUE;
			}

			watch->write_buffer = rec->data;
			watch->write_off = 0;
			watch->write_size = rec->size;

			if (watch->backlog < rec->size)
				watch->backlog = rec->size;

			watch->backlog -= rec->size;
			g_slice_free(IO_rec, rec);
		}

		res = rtp_watch_write_bytes(g_event_fd(watch), watch->write_buffer,
			&watch->write_off, watch->write_size, &watch->err_code);

		g_mutex_unlock(watch->mutex);

		if (res != GST_RTSP_OK)
		{
			if (res == GST_RTSP_EINTR)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}

		g_mutex_lock(watch->mutex);

		g_free(watch->write_buffer);
		watch->write_buffer = NULL;

	} while (TRUE);

	g_mutex_unlock(watch->mutex);

	return TRUE;
}


gboolean
rtp_watch_would_block(RTP_watch *watch)
{
	gboolean block = FALSE;

	g_mutex_lock(watch->mutex);
	block = (watch->backlog >=  MAX_DATA_PENDING/2);
	g_mutex_unlock(watch->mutex);

	return block;
}

//:~ End
