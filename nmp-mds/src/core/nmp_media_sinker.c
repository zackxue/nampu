#include <string.h>
#include <arpa/inet.h>
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_media_sinker.h"
#include "nmp_rtp_buffer.h"
#include "nmp_scheduler.h"
#include "nmp_audit.h"

#define nmp_mem_kmalloc(size)	g_malloc(size)
#define nmp_mem_kzalloc(size)	g_malloc0(size)
#define nmp_mem_kfree(ptr, size)	g_free(ptr)

#define READ_ERR    (G_IO_HUP | G_IO_ERR | G_IO_NVAL)
#define READ_COND   (G_IO_IN | READ_ERR)
#define WRITE_ERR   (G_IO_HUP | G_IO_ERR|G_IO_NVAL)
#define WRITE_COND  (G_IO_OUT | WRITE_ERR)

typedef struct _JpfBuf JpfBuf;
struct _JpfBuf
{
	gchar	*start;
	gint	size;
	gint	stream;
};


static void
nmp_media_sinker_recv(gpointer data, gpointer user_data);
static __inline__ gint
nmp_media_sinker_recv_data(JpfSinkerWatch *sinker, gint stream,
	gchar *data, gsize size);
static void
nmp_media_sinker_detach(JpfSinkerWatch *watch);

static JpfSinkerWatchFuncs nmp_default_sinker_watch_funcs;
static JpfSinkerWatchFuncs nmp_tcp_sinker_watch_funcs;
static JpfSinkerWatchFuncs nmp_udp_sinker_watch_funcs;


JpfMediaSinkers *
nmp_media_sinkers_new(gpointer media, gint blockable)
{
	JpfMediaSinkers *sinkers;

	sinkers = nmp_mem_kmalloc(sizeof(JpfMediaSinkers));
	if (G_UNLIKELY(!sinkers))
		return NULL;

	sinkers->sinkers = g_ptr_array_new();
	if (G_UNLIKELY(!sinkers->sinkers))
	{
		nmp_mem_kfree(sinkers, sizeof(JpfMediaSinkers));
		return NULL;
	}

	sinkers->ref_count = 1;
	sinkers->n_sinkers = 0;
	sinkers->media = media;
	sinkers->blockable = blockable;
	g_static_mutex_init(&sinkers->lock);

	return sinkers;
}


static void
nmp_media_sinkers_unref_sinker(gpointer data, gpointer user_data)
{
	JpfSinkerWatch *sinker;
	g_assert(data != NULL);

	sinker = (JpfSinkerWatch*)data;
	nmp_media_sinker_detach(sinker);
	nmp_media_sinker_unref(sinker);
}


static __inline__ void
nmp_media_sinkers_release(JpfMediaSinkers *sinkers)
{
	g_assert(sinkers != NULL);

	g_static_mutex_free(&sinkers->lock);

	g_ptr_array_foreach(sinkers->sinkers,
		nmp_media_sinkers_unref_sinker,
		NULL
	);

	g_ptr_array_free(sinkers->sinkers, TRUE);
}


JpfMediaSinkers *
nmp_media_sinkers_ref(JpfMediaSinkers *sinkers)
{
	g_assert(sinkers != NULL &&
		g_atomic_int_get(&sinkers->ref_count) > 0);
	g_atomic_int_add(&sinkers->ref_count, 1);
	return sinkers;
}


void
nmp_media_sinkers_unref(JpfMediaSinkers *sinkers)
{
	g_assert(sinkers != NULL &&
		g_atomic_int_get(&sinkers->ref_count) > 0);
	if (g_atomic_int_dec_and_test(&sinkers->ref_count))
	{
		nmp_media_sinkers_release(sinkers);
	}
}


static __inline__ void
__nmp_media_sinkers_add(JpfMediaSinkers *sinkers, JpfSinkerWatch *w)
{
	g_ptr_array_add(sinkers->sinkers, w);
	nmp_media_sinker_ref(w);
	++sinkers->n_sinkers;
	BUG_ON(sinkers->n_sinkers >1 && sinkers->blockable);
}


void
nmp_media_sinkers_add(JpfMediaSinkers *sinkers, JpfSinkerWatch *w)
{
	G_ASSERT(sinkers != NULL && w != NULL);
	
	g_static_mutex_lock(&sinkers->lock);
	__nmp_media_sinkers_add(sinkers, w);
	g_static_mutex_unlock(&sinkers->lock);
}


static __inline__ gint
__nmp_media_sinkers_del(JpfMediaSinkers *sinkers, JpfSinkerWatch *w)
{
	gboolean found;

	found = g_ptr_array_remove(sinkers->sinkers, w);
	if (G_LIKELY(found))
	{
		--sinkers->n_sinkers;
		g_static_mutex_unlock(&sinkers->lock);

		nmp_media_sinker_detach(w);
		nmp_media_sinker_unref(w);

		g_static_mutex_lock(&sinkers->lock);
	}

	return sinkers->n_sinkers;
}


gint
nmp_media_sinkers_del(JpfMediaSinkers *sinkers, JpfSinkerWatch *w)
{
	gint n_sinkers;
	G_ASSERT(sinkers != NULL && w != NULL);

	g_static_mutex_lock(&sinkers->lock);
	n_sinkers = __nmp_media_sinkers_del(sinkers, w);
	g_static_mutex_unlock(&sinkers->lock);

	return n_sinkers;
}


static __inline__ gint
__nmp_media_sinkers_recv_packet(JpfMediaSinkers *sinkers, gint stream,
	gchar *data, gsize size)
{
	JpfBuf buf;
	JpfSinkerWatch *s;

	buf.start = data;
	buf.size = size;
	buf.stream = stream;

	if (sinkers->blockable)
	{
		if (sinkers->n_sinkers)
		{
			s = g_ptr_array_index(sinkers->sinkers, 0);
			BUG_ON(!s);
			return nmp_media_sinker_recv_data(s, stream, data, size);
		}
		return 0;
	}

	if (data)
	{
		g_ptr_array_foreach(
			sinkers->sinkers,
			nmp_media_sinker_recv,
			&buf
		);
	}

	return 0;
}


gint
nmp_media_sinkers_recv_packet(JpfMediaSinkers *sinkers, gint stream,
	gchar *data, gsize size)
{
	gint err = 0;
	G_ASSERT(sinkers != NULL);

	g_static_mutex_lock(&sinkers->lock);
	if (sinkers->n_sinkers > 0)
		err = __nmp_media_sinkers_recv_packet(sinkers, stream, data, size);
	g_static_mutex_unlock(&sinkers->lock);

	return err;
}


static __inline__ void
nmp_media_sinker_init_stream_rtp_pollfd(JpfSinkerStream *stream)
{
	stream->rtp_read.fd = stream->sock_info.rtp_sock;
	stream->rtp_read.events = READ_COND;
	stream->rtp_read.revents = 0;

	stream->rtp_write.fd = stream->sock_info.rtp_sock;
	stream->rtp_write.events = WRITE_COND;
	stream->rtp_write.revents = 0;
}


#ifdef CONFIG_RTCP_SUPPORT

static __inline__ void
nmp_media_sinker_init_stream_rtcp_pollfd(JpfSinkerStream *stream)
{
	stream->rtcp_control.rtcp_recv.fd = stream->sock_info.rtcp_sock;
	stream->rtcp_control.rtcp_recv.events = READ_COND;
	stream->rtcp_control.rtcp_recv.revents = 0;

	stream->rtcp_control.rtcp_send.fd = stream->sock_info.rtcp_sock;
	stream->rtcp_control.rtcp_send.events = WRITE_COND;
	stream->rtcp_control.rtcp_send.revents = 0;	
}

#endif


static __inline__ void
nmp_media_sinker_init_stream_pollfd(JpfSinkerStream *stream)
{
	nmp_media_sinker_init_stream_rtp_pollfd(stream);
#ifdef CONFIG_RTCP_SUPPORT
	nmp_media_sinker_init_stream_rtcp_pollfd(stream);
#endif
}


static __inline__ gint
nmp_media_sinker_init_stream(JpfSinkerStream *stream,
	GstRTSPLowerTrans trans, gint i_stream)
{
	gint err;

	memset(stream, 0, sizeof(*stream));
	stream->rtp_over_rtsp = TRUE;
	stream->pending = FALSE;

	if (trans == GST_RTSP_LOWER_TRANS_UDP)
	{
		stream->ring_buff = nmp_ring_buffer_new(
			&rtp_buffer_ops, 
			i_stream ? 8192 : 819200
		);

		if (G_UNLIKELY(!stream->ring_buff))
		{
			return -E_NOMEM;
		}

		err = nmp_media_sock_info_init(&stream->sock_info, MEDIA_TRANS_UDP);	//更换trans类型
		if (G_UNLIKELY(err))
		{
			nmp_ring_buffer_release(stream->ring_buff);
			return err;
		}

		nmp_media_sinker_init_stream_pollfd(stream);

		stream->rtp_over_rtsp = FALSE;
		return 0;
	}

	if (trans == GST_RTSP_LOWER_TRANS_TCP)
	{
		stream->ring_buff = NULL;
		nmp_print(
			"Client transport: 'tcp over rtsp'."
		);
		return 0;
	}

	nmp_print(
		"'transport' isn't supported yet. "
	);

	return -E_NOTSUPPORT;
}


static __inline__ gint
nmp_media_sinker_stream_set_transport(JpfSinkerStream *stream,
	gchar *client_Ip, GstRTSPTransport *ct, gboolean b_nat)
{
	gint err = 0;

	if (stream->rtp_over_rtsp)
	{
		stream->rtp_ch = ct->interleaved.min;
#ifdef CONFIG_RTCP_SUPPORT
#endif
		return err;
	}

	if (!b_nat)
	{
		err = nmp_media_sock_connect(&stream->sock_info, client_Ip, ct);
		if (G_UNLIKELY(err))
			return err;

		stream->connected = TRUE;
#ifdef CONFIG_RTCP_SUPPORT
		stream->rtcp_control.rtcp_connected = TRUE;
#endif
	}
	else
	{
		nmp_media_sock_get_server_ports(&stream->sock_info, ct);
	}

	return err;
}


static __inline__ void
nmp_media_sinker_destroy_stream(JpfSinkerStream *stream)
{
	if (!stream->rtp_over_rtsp)
		nmp_media_sock_info_reset(&stream->sock_info);

	if (stream->ring_buff)
		nmp_ring_buffer_release(stream->ring_buff);
}


static __inline__ void
nmp_media_sinker_pending(JpfSinkerWatch *watch, JpfSinkerStream *stream)
{
	if (!stream->pending)
	{
		g_source_add_poll((GSource*)watch, &stream->rtp_write);
		stream->pending = TRUE;
	}
}

static __inline__ void
nmp_media_sinker_add_stream_rpoll(JpfSinkerWatch *watch, JpfSinkerStream *stream)
{
	if (!stream->rtp_over_rtsp && !stream->connected)
	{
		g_source_add_poll((GSource*)watch, &stream->rtp_read);
#ifdef CONFIG_RTCP_SUPPORT
		g_source_add_poll((GSource*)watch, &stream->rtcp_control.rtcp_recv);
#endif
	}
}


static __inline__ gboolean
nmp_media_sinker_check_stream(JpfSinkerStream *stream)
{
	if (stream->rtp_write.revents & WRITE_COND)
		return TRUE;
	return FALSE;
}


static __inline__ gint
nmp_media_sinker_init_streams(JpfSinkerWatch *watch, GstRTSPLowerTrans trans,
	gint streams, gint *p_err)
{
	gint n_streams, err;

	for (n_streams = 0; n_streams < streams; ++n_streams)
	{
		err = nmp_media_sinker_init_stream(
			&watch->streams[n_streams], trans, n_streams);
		if (G_UNLIKELY(err))
		{
			nmp_warning(
				"Init sinker stream '%d' failed, code:%d.",
				n_streams,
				err	
			);
			goto init_stream_failed;
		}
	}

	watch->n_streams = streams;
	return 0;

init_stream_failed:
	for (; --n_streams >= 0;)
	{
		nmp_media_sinker_destroy_stream(&watch->streams[n_streams]);
	}
	watch->n_streams = 0;

	if (p_err)
	{
		*p_err = err;
	}

	return err;
}


static __inline__ void
nmp_media_sinker_destroy_streams(JpfSinkerWatch *sinker)
{
	gint n_streams = sinker->n_streams;

	nmp_audit_dec(AUDIT_CNT_MEDIA_SNK, 1);
	nmp_audit_dec(AUDIT_CNT_STREAM_SNK, n_streams);

	for (; --n_streams >= 0; )
	{
		nmp_media_sinker_destroy_stream(&sinker->streams[n_streams]);
	}
}


gint
nmp_media_sinker_set_transport(JpfSinkerWatch *sinker, gint i_stream, 
	gchar *client_Ip, GstRTSPTransport *ct, gboolean b_nat, void *p)
{
	JpfSinkerStream *stream;
	gint ret;
	g_assert(sinker != NULL && ct != NULL);

	if (i_stream < 0 || i_stream >= sinker->n_streams)
	{
		nmp_warning(
			"Set stream '%d' transport failed, total streams '%d'.",
			i_stream, sinker->n_streams
		);
		return -E_INVAL;
	}

	stream = &sinker->streams[i_stream];
	ret = nmp_media_sinker_stream_set_transport(stream, client_Ip, ct,
		b_nat);
	if (G_LIKELY(!ret))
	{
		nmp_media_sinker_add_stream_rpoll(sinker, stream);
	}

	if (ct->lower_transport == GST_RTSP_LOWER_TRANS_UDP)
		sinker->funcs = &nmp_udp_sinker_watch_funcs;
	else if (ct->lower_transport == GST_RTSP_LOWER_TRANS_TCP)
	{
		sinker->tcp_sinker = p;
		sinker->funcs = &nmp_tcp_sinker_watch_funcs;
	}

	return ret;
}


static __inline__ gboolean
__nmp_media_sinker_check(JpfSinkerWatch *sinker)
{
	JpfSinkerStream *stream;
	gint n_streams = sinker->n_streams;

	for (; --n_streams >= 0; )
	{
		stream = &sinker->streams[n_streams];
		if (nmp_media_sinker_check_stream(stream))
			return TRUE;
	}

	return FALSE;
}


static __inline__ gboolean
nmp_media_sinker_check(JpfSinkerWatch *sinker)
{
	gboolean ret;

	g_static_mutex_lock(&sinker->lock);
	ret = __nmp_media_sinker_check(sinker);	
	g_static_mutex_unlock(&sinker->lock);

	return ret;
}


static __inline__ gint
nmp_media_sinker_stream_pull(JpfSinkerStream *stream, gint *exit_loop)
{
	gint ret;

	*exit_loop = 0;

	if (stream->rtp_buffer.bytes > 0)
		return stream->rtp_buffer.bytes;

	ret = nmp_ring_buffer_get_data(stream->ring_buff,
		stream->rtp_buffer.buffer, SMART_BUFFER_SIZE, exit_loop);
	BUG_ON(ret < 0);

	stream->rtp_buffer.offset = 0;
	stream->rtp_buffer.bytes = ret;

	return ret;
}


static __inline__ gint
nmp_media_sinker_process_stream(JpfSinkerWatch *sinker,
	JpfSinkerStream *stream)
{
	JpfSinkerWatchFuncs *funcs;

	if (stream->state != SINKER_WATCH_STAT_PENDING)
	{
		BUG_ON(stream->pending);
		return 0;
	}

	funcs = sinker->funcs;
	BUG_ON(!funcs || !funcs->send);

	return (*funcs->send)(sinker, stream);
}


static __inline__ gint
__nmp_media_sinker_process(JpfSinkerWatch *sinker)
{
	JpfSinkerStream *stream;
	gint err, n_streams = sinker->n_streams;

	for (; --n_streams >= 0; )
	{
		stream = &sinker->streams[n_streams];
		err = nmp_media_sinker_process_stream(sinker, stream);
		if (err)
		{
			return err;
		}
	}

	return 0;
}


static __inline__ gint
nmp_media_sinker_process(JpfSinkerWatch *sinker)
{
	gint ret;

	g_static_mutex_lock(&sinker->lock);
	ret = __nmp_media_sinker_process(sinker);	
	g_static_mutex_unlock(&sinker->lock);

	return ret;
}


static __inline__ gint
nmp_media_sinker_stream_udp_fill(JpfSinkerStream *stream, gchar *data,
	gsize size)
{
	gchar *rtp_orig;
	gint err, ec, n_write;

	if (G_UNLIKELY(!stream->connected || !data))
		return 0;

	if (stream->state != SINKER_WATCH_STAT_PENDING)
	{
		rtp_orig = data + WRAP_HEAD_LEN;
		n_write = size - WRAP_HEAD_LEN;

		/* try to write directly, hope it should be successfull.
		 * if data is partially written(unlikely), resend the whole. */
		err = write(stream->rtp_write.fd, rtp_orig, n_write);
		if (err != n_write)
		{
			ec = -errno;
			if (err < 0 && ec != -EAGAIN)
				return ec;

			BUG_ON(size > SMART_BUFFER_SIZE);
			memcpy(stream->rtp_buffer.buffer, data, size);
			stream->rtp_buffer.offset = 0;
			stream->rtp_buffer.bytes = size;
			stream->state = SINKER_WATCH_STAT_PENDING;
			return -EAGAIN;
		}

		nmp_audit_inc(AUDIT_BRATE_SINKER, err);
		return 0;
	}
	else
	{
		if ((err = nmp_ring_buffer_fill_data(stream->ring_buff, data, size)))
			return err; 

		return -EAGAIN;
	}	
}


static __inline__ gint
__nmp_media_sinker_recv_data(JpfSinkerWatch *sinker, gint i_stream, 
	gchar *data, gsize size)
{
	JpfSinkerWatchFuncs *funcs;
	JpfSinkerStream *stream;

	if (G_UNLIKELY(!sinker->sink_ready))
		return 0;

	stream = &sinker->streams[i_stream];
	funcs = sinker->funcs;
	BUG_ON(i_stream >= NSTREAMS);
	BUG_ON(!funcs || !funcs->fill);	

	return (*funcs->fill)(sinker, stream, data, size);
}


static __inline__ gint
nmp_media_sinker_recv_data(JpfSinkerWatch *sinker, gint stream,
	gchar *data, gsize size)
{
	gint err;
	G_ASSERT(sinker != NULL);

	g_static_mutex_lock(&sinker->lock);
	err = __nmp_media_sinker_recv_data(sinker, stream, data, size);
	g_static_mutex_unlock(&sinker->lock);

	return err;
}


static void
nmp_media_sinker_recv(gpointer data, gpointer user_data)
{
	JpfSinkerWatch *s = (JpfSinkerWatch*)data;
	JpfBuf *buf = (JpfBuf*)user_data;

	nmp_media_sinker_recv_data(s, buf->stream, buf->start, buf->size);
}


__export void
nmp_media_sinker_ref(JpfSinkerWatch *watch)
{
	G_ASSERT(watch != NULL);

	g_source_ref((GSource*)watch);
}


__export void
nmp_media_sinker_unref(JpfSinkerWatch *watch)
{
	G_ASSERT(watch != NULL);

	g_source_unref((GSource*)watch);
}


static __inline__ void
__nmp_media_sinker_play(JpfSinkerWatch *watch)
{
	watch->sink_ready = TRUE;
}


void
nmp_media_sinker_play(JpfSinkerWatch *watch)
{
	g_assert(watch != NULL);

	g_static_mutex_lock(&watch->lock);
	__nmp_media_sinker_play(watch);
	g_static_mutex_unlock(&watch->lock);
}


static void
nmp_media_sinker_detach(JpfSinkerWatch *watch)
{
	g_assert(watch != NULL);

	nmp_scheduler_del((GSource*)watch);	/* remove watch from loop */
}


static gboolean
nmp_media_sinker_watch_prepare(GSource *source, gint *timeout)
{
    *timeout = -1;

    return FALSE;
}


static gboolean
nmp_media_sinker_watch_check(GSource *__source)
{
	JpfSinkerWatch *sinker = (JpfSinkerWatch*)__source;

	return nmp_media_sinker_check(sinker);
}


static gboolean
nmp_media_sinker_watch_dispatch(GSource *__source, 
	GSourceFunc callback, gpointer user_data)
{
	JpfSinkerWatch *sinker = (JpfSinkerWatch*)__source;
	gint err;

	if ((err = nmp_media_sinker_process(sinker)))
	{
		nmp_error(
			"error dispatch!!!\n"
		);
		return FALSE;
	}

	return TRUE;
}


static void
nmp_media_sinker_watch_finalize(GSource *__source)
{
	JpfSinkerWatch *sinker = (JpfSinkerWatch*)__source;

	nmp_media_sinker_destroy_streams(sinker);
}


static GSourceFuncs nmp_sinker_source_funcs = 
{
    .prepare        = nmp_media_sinker_watch_prepare,
    .check          = nmp_media_sinker_watch_check,
    .dispatch       = nmp_media_sinker_watch_dispatch,
    .finalize       = nmp_media_sinker_watch_finalize
};


JpfSinkerWatch *
nmp_media_sinker_create(GstRTSPLowerTrans trans, gint streams,
	gint *err)
{
	JpfSinkerWatch *watch;

	if (G_UNLIKELY(streams > NSTREAMS))
	{
		if (err)
		{
			*err = -E_INVAL;
		}
		return NULL;
	}

	watch = (JpfSinkerWatch*)g_source_new(
		&nmp_sinker_source_funcs, sizeof(JpfSinkerWatch));
	if (G_UNLIKELY(!watch))
	{
		if (err)
		{
			*err = -E_NOMEM;
		}
		return NULL;
	}

	if (nmp_media_sinker_init_streams(watch, trans, streams, err))
	{
		g_source_unref((GSource*)watch);
		return NULL;
	}

	watch->trans = trans;
	watch->tcp_sinker = NULL;
	watch->funcs = &nmp_default_sinker_watch_funcs;
	watch->sink_ready = FALSE;
	g_static_mutex_init(&watch->lock);
	nmp_scheduler_add((GSource*)watch, WEIGHT_STREAM_OUT);

	nmp_audit_inc(AUDIT_CNT_MEDIA_SNK, 1);
	nmp_audit_inc(AUDIT_CNT_STREAM_SNK, streams);

	return watch;
}


static __inline__ gint
nmp_sinker_stream_buffer_udp_write(JpfSinkerWatch *sinker,
	JpfSinkerStream *stream)
{
	gchar *buf;
	gsize size;
	gint ret;

	buf =  stream->rtp_buffer.buffer + WRAP_HEAD_LEN;
	size = stream->rtp_buffer.bytes - WRAP_HEAD_LEN;

	if (size > 0)
	{
		ret = write(stream->rtp_write.fd, buf, size);
		if (ret != size)
		{
			if (ret < 0)
			{
				return -errno;
			}
			return -EAGAIN;
		}
		nmp_audit_inc(AUDIT_BRATE_SINKER, ret);
	}

	stream->rtp_buffer.offset = 0;
	stream->rtp_buffer.bytes = 0;
	return 0;
}


static gint
nmp_sinker_stream_udp_send(JpfSinkerWatch *sinker, JpfSinkerStream *stream)
{
	gint ret, exit_loop;

	if (!(stream->rtp_write.revents&WRITE_COND))
		return 0;

	stream->rtp_write.revents &= ~WRITE_COND;

	while (nmp_media_sinker_stream_pull(stream, &exit_loop))
	{
		ret = nmp_sinker_stream_buffer_udp_write(sinker, stream);
		if (!ret)
			continue;
		else
		{
			if (ret == -EAGAIN)
				return 0;
			return ret;
		}

		if (exit_loop)
			break;
	}

	if (stream->pending)
	{
		stream->pending = FALSE;
		g_source_remove_poll((GSource*)sinker, &stream->rtp_write);
	}

	stream->state = SINKER_WATCH_STAT_EMPTY;
	return 0;
}


static gint
nmp_sinker_stream_udp_fill(JpfSinkerWatch *sinker,
	JpfSinkerStream *stream, gchar *data, gsize size)
{
	gint err;

	err = nmp_media_sinker_stream_udp_fill(stream, data, size);
	if (err == -EAGAIN)
	{
		nmp_media_sinker_pending(sinker, stream);
	}

	return err;	
}



static gint
nmp_sinker_stream_tcp_send(JpfSinkerWatch *sinker, JpfSinkerStream *stream)
{
	return 0;
}


static gint
nmp_sinker_stream_tcp_fill(JpfSinkerWatch *sinker,
	JpfSinkerStream *stream, gchar *data, gsize size)
{
	extern gint nmp_rtsp_client_send_message(void *cli, GstRTSPMessage *msg);
	extern gboolean nmp_rtsp_client_would_block(void *cli);

	GstRTSPMessage *msg;
	gchar *rtp_pkt;
	guint16 rtp_size;

	if (!data)
	{
		if (nmp_rtsp_client_would_block(sinker->tcp_sinker))
			return -EAGAIN;
		return 0;
	}

	if (G_UNLIKELY(!sinker->tcp_sinker))
		return 0;

	gst_rtsp_message_new_data(&msg, stream->rtp_ch);

	rtp_pkt = data + WRAP_HEAD_LEN;
	rtp_size = size - WRAP_HEAD_LEN;

	gst_rtsp_message_set_body(msg, (const guint8*)rtp_pkt, rtp_size);
	if (!nmp_rtsp_client_send_message(sinker->tcp_sinker, msg))
		nmp_audit_inc(AUDIT_BRATE_SINKER, rtp_size + 4);
	gst_rtsp_message_free(msg);

	return 0;
}


static gint
nmp_sinker_stream_default_send(JpfSinkerWatch *sinker,
	JpfSinkerStream *stream)
{
	return -E_NOTSUPPORT;
}


static gint
nmp_sinker_stream_default_fill(JpfSinkerWatch *sinker,
	JpfSinkerStream *stream, gchar *data, gsize size)
{
	return -E_NOTSUPPORT;
}


static void 
nmp_media_sinker_watch_error(JpfSinkerWatch *watch)
{
	//在LOOP对此watch进行读写失败或连接关闭时调用。
	//处理的动作为：从LOOP中删除此SOURCE，关闭session, 从media中删除此sinker, 如果需要，关闭media.
	//不能在finalize()中调用。
}


static JpfSinkerWatchFuncs nmp_udp_sinker_watch_funcs =		/* RTP(UDP) */
{
	.send = nmp_sinker_stream_udp_send,
	.fill = nmp_sinker_stream_udp_fill,
	.error = nmp_media_sinker_watch_error
};


static JpfSinkerWatchFuncs nmp_tcp_sinker_watch_funcs =		/* RTP over RTSP */
{
	.send = nmp_sinker_stream_tcp_send,
	.fill = nmp_sinker_stream_tcp_fill,
	.error = nmp_media_sinker_watch_error
};

static JpfSinkerWatchFuncs nmp_default_sinker_watch_funcs =	/* default */
{
	.send = nmp_sinker_stream_default_send,
	.fill = nmp_sinker_stream_default_fill
};


//: ~End
