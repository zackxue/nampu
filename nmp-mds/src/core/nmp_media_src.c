#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_media_src.h"
#include "nmp_media_device.h"
#include "nmp_rtp_buffer.h"
#include "nmp_scheduler.h"
#include "nmp_audit.h"
#include "nmp_monotonic_timer.h"


#define READ_ERR    (G_IO_HUP|G_IO_ERR|G_IO_NVAL)
#define READ_COND   (G_IO_IN|READ_ERR)
#define WRITE_ERR   (G_IO_HUP|G_IO_ERR|G_IO_NVAL)
#define WRITE_COND  (G_IO_OUT|WRITE_ERR)

#define MEDIA_SRC_TIMOUT_SECS			8
#define MEDIA_SLEEPING_TIMEOUT_MSEC		10


static __inline__ void
nmp_media_src_destroy_streams(JpfMediaSource *stream);
static __inline__ void
nmp_media_src_recycle_socket(JpfMediaSource *source);
static __inline__ void
nmp_media_src_add_streams_poll(JpfMediaSource *source, gint rtcp);
static __inline__ void
nmp_media_src_remove_streams_poll(JpfMediaSource *source, gint rtcp);


static __inline__ gint64
nmp_media_src_get_time(JpfMediaSource *source)
{
	gint64 ts;
	GTimeVal tv;

	g_source_get_current_time((GSource*)source, &tv);
	ts = ((gint64)tv.tv_sec) * 1000;
	ts += tv.tv_usec / 1000;
	return ts;
}


static __inline__ gint64
nmp_media_src_cal_timeout_delta(JpfMediaSource *source)
{
	gint64 now, timeout_point;

	timeout_point = source->last_check + MEDIA_SRC_TIMOUT_SECS;
	now = nmp_get_monotonic_time();
	return now - timeout_point;
}


static __inline__ gint64
nmp_media_src_cal_sleeping_timeout_delta(JpfMediaSource *source)
{
	gint64 now, timeout_point;

	timeout_point = source->last_check + MEDIA_SLEEPING_TIMEOUT_MSEC;
	now = nmp_media_src_get_time(source);
	return now - timeout_point;	
}


static __inline__ void
nmp_media_src_sync_time(JpfMediaSource *source)
{
	if (!source->blockable)
		source->last_check = nmp_get_monotonic_time();
	else
		source->last_check = nmp_media_src_get_time(source);
}


static __inline__ gint
nmp_media_src_might_sleep(JpfMediaSource *source)
{
	gint err;

	if (!source->blockable)
		return 0;

	if (!source->func_recv)
		return 0;

	err = (*source->func_recv)(source->media, 0, NULL, 0);
	if (err != -EAGAIN)
		return 0;

	if (!source->blocking)
	{
		source->blocking = 1;
		nmp_media_src_remove_streams_poll(source, 0);
		nmp_media_src_sync_time(source);
	}

	return 1;
}


static __inline__ void
nmp_media_src_woken_up(JpfMediaSource *source)
{
	if (source->blocking)
	{
		source->blocking = 0;
		nmp_media_src_add_streams_poll(source, 0);
	}
}


static __inline__ gboolean
nmp_media_src_check_rtp(JpfStreamSource *stream)
{
	if (stream->rtp_read.revents & READ_COND)
		return TRUE;
	return FALSE;
}


static __inline__ void
nmp_media_src_init_rtp_pollfd(GPollFD *fd, JpfMediaSocket *ssock)
{
	fd->fd = ssock->rtp_sock;
	fd->events = READ_COND;
	fd->revents = 0;
}


static __inline__ gint
nmp_media_src_recv_udp_stream_rtp(JpfMediaSource *source, gint i_stream,
	JpfStreamSource *stream)
{
	JpfRtpWrap *wrap_rtp;
	gint rtp_size, buf_size, err;

	if (!(stream->rtp_read.revents & READ_COND))
		return 0;

	wrap_rtp = (JpfRtpWrap*)stream->rtp_buffer.buffer;
	buf_size = sizeof(stream->rtp_buffer.buffer) - WRAP_HEAD_LEN;

	while ((rtp_size = recvfrom(stream->rtp_read.fd, 
		wrap_rtp->rtp, buf_size, 0, NULL, NULL)) > 0)
	{
		nmp_audit_inc(AUDIT_BRATE_SRC, rtp_size);

		if (G_LIKELY(source->func_recv))
		{
			wrap_rtp->length = rtp_size;

			(*source->func_recv)(source->media, i_stream, (gchar*)wrap_rtp, 
				WRAP_HEAD_LEN + rtp_size);

#ifdef CONFIG_RTCP_SUPPORT
			nmp_rtcp_control_update(&stream->rtcp_control,
				wrap_rtp->rtp, rtp_size);
#endif
		}
	}

	err = -errno;
	if (!rtp_size || err != -EAGAIN)
	{
		nmp_print(
			"Udp sock error, while recving udp stream '%d', err:'%d'.",
			i_stream,
			err
		);

		return err;
	}

	stream->rtp_read.revents &= ~READ_COND;
	return 0;
}


static __inline__ gint
nmp_media_src_tcp_est_rtp(JpfMediaSource *source, JpfStreamSource *stream)
{
	gint err;

	err = nmp_media_sock_est_rtp_sock(&stream->sock_info);
	if (!err)
	{
		g_source_remove_poll((GSource*)source, &stream->rtp_read);
		nmp_media_src_init_rtp_pollfd(&stream->rtp_read, &stream->sock_info);
		g_source_add_poll((GSource*)source, &stream->rtp_read);
		stream->rtp_connected = TRUE;
	}
	else
	{
		nmp_print(
			"Media src accept tcp rtp sock failed, err:'%d'", err
		);
	}

	return err;
}


static __inline__ gint
nmp_media_src_recv_tcp(JpfMediaSource *source, JpfStreamSource *stream,
	gchar buf[], gsize size)
{
	gint ret;

#ifdef CONFIG_MEDIA_SRC_USE_BLOCKABLE_IO
	if (stream->data_size)
	{
copy_it:
		ret = stream->data_size > size ? size : stream->data_size;
		memcpy(buf, &stream->data_block[stream->data_offset], ret);
		stream->data_offset += ret;
		stream->data_size -= ret;
		return ret;
	}
	else
	{
		stream->data_offset = 0;
		ret = recv(stream->rtp_read.fd, stream->data_block, stream->block_capacity,
			MSG_DONTWAIT);
		if (ret > 0)
		{
			stream->data_size = ret;
			goto copy_it;
		}
		else
		{
			if (ret == 0)
				return ret;
			return -errno;
		}
	}

#else
	ret = recv(stream->rtp_read.fd, buf, size, MSG_DONTWAIT);
	if (ret >= 0)
	{
		return ret;
	}

	return -errno;
#endif
}


static __inline__ gint
nmp_media_src_recv_tcp_stream_rtp(JpfMediaSource *source, gint i_stream,
	JpfStreamSource *stream)
{
	JpfRtpWrap *wrap_rtp;
	gint to_recv, nr;
	gsize rtp_size, trivial_size;
	JpfSmartBuffer *sb;

	if (!(stream->rtp_read.revents & READ_COND))
		return 0;

	if (G_UNLIKELY(!stream->rtp_connected))
	{
		return nmp_media_src_tcp_est_rtp(source, stream);
	}

	sb = &stream->rtp_buffer;
	trivial_size = WRAP_HEAD_LEN + 4;	/* 4: '$' + 'ch' + len */

	if (G_UNLIKELY(!sb->bytes))
	{
		sb->bytes = WRAP_HEAD_LEN;
		sb->offset = WRAP_HEAD_LEN;
	}

	while (TRUE)
	{
		if (nmp_media_src_might_sleep(source))
		{
			stream->rtp_read.revents &= ~READ_COND;
			return 0;
		}

		if (sb->bytes < trivial_size)
			to_recv = trivial_size - sb->bytes;
		else
		{
			rtp_size = ntohs(*(unsigned short*)(sb->buffer + trivial_size  - 2));
			if (G_UNLIKELY(!rtp_size))
			{
				sb->bytes = WRAP_HEAD_LEN;
				sb->offset = WRAP_HEAD_LEN;
				continue;
			}

			if (G_UNLIKELY(rtp_size >= SMART_BUFFER_SIZE - trivial_size))
			{
				nmp_warning(
					"Rtp size(%u) too large(>SMART_BUFFER_SIZE), while recving tcp stream '%d'.",
					rtp_size, i_stream
				);
				return -E_PACKET;
			}

			to_recv = rtp_size - (sb->bytes - trivial_size);
		}

		nr = nmp_media_src_recv_tcp(source, stream, &sb->buffer[sb->offset], to_recv);
		if (nr <= 0)
		{
			if (nr == 0)
			{
				nmp_print(
					"Tcp connection reset by peer, while recving tcp stream '%d'.",
					i_stream
				);
				return -ECONNRESET;
			}

			if (nr != -EAGAIN)
			{
				nmp_print(
					"Tcp connection error, while recving tcp stream '%d', err:'%d'.",
					i_stream,
					nr
				);
				return nr;
			}

			break;
		}

		sb->offset += nr;
		sb->bytes += nr;

		if (nr < to_recv)
			break;

		if (sb->bytes > trivial_size)
		{
			wrap_rtp = (JpfRtpWrap*)(sb->buffer + trivial_size - WRAP_HEAD_LEN);
			wrap_rtp->length = rtp_size;

			nmp_audit_inc(AUDIT_BRATE_SRC, rtp_size);

			if (G_LIKELY(source->func_recv))
			{
				(*source->func_recv)(source->media, i_stream, (gchar*)wrap_rtp, 
					WRAP_HEAD_LEN + wrap_rtp->length);

#ifdef CONFIG_RTCP_SUPPORT
				nmp_rtcp_control_update(&stream->rtcp_control,
					wrap_rtp->rtp, rtp_size);
#endif
			}

			sb->bytes = WRAP_HEAD_LEN;
			sb->offset = WRAP_HEAD_LEN;
		}
	}

	stream->rtp_read.revents &= ~READ_COND;
	return 0;
}


static __inline__ gint
nmp_media_src_recv_udp_rtp(JpfMediaSource *source)
{
	gint err, n_streams;

	n_streams = source->n_streams;
	while (--n_streams >= 0)
	{
		err = nmp_media_src_recv_udp_stream_rtp(
			source,
			n_streams,
			&source->streams[n_streams]
		);

		if (G_UNLIKELY(err))
			return err;
	}

	return 0;
}


static __inline__ gint
nmp_media_src_recv_tcp_rtp(JpfMediaSource *source)
{
	gint err, n_streams;

	n_streams = source->n_streams;
	while (--n_streams >= 0)
	{
		err = nmp_media_src_recv_tcp_stream_rtp(
			source,
			n_streams,
			&source->streams[n_streams]
		);

		if (G_UNLIKELY(err))
			return err;
	}

	return 0;
}


#ifdef CONFIG_RTCP_SUPPORT

static __inline__  void
nmp_media_src_init_rtcp_cb(JpfRtcpControl *rtcp_control, 
	JpfMediaSocket *ssock)
{
	rtcp_control->rtcp_recv.fd = ssock->rtcp_sock;
	rtcp_control->rtcp_send.fd = ssock->rtcp_sock;
	rtcp_control->rtcp_recv.events = READ_COND;
	rtcp_control->rtcp_send.events = WRITE_COND;
	rtcp_control->last_rtp_seq = 0;
}


static __inline__ void
nmp_media_src_add_poll_rtcp(JpfMediaSource *source, 
	JpfRtcpControl *rtcp_control)
{
	g_source_add_poll((GSource*)source, &rtcp_control->rtcp_recv);
}


static __inline__ void
nmp_media_src_remove_poll_rtcp(JpfMediaSource *source, 
	JpfRtcpControl *rtcp_control)
{
	g_source_remove_poll((GSource*)source, &rtcp_control->rtcp_recv);
}


gint
nmp_media_src_get_rtcp_port(JpfMediaSource *source, gint stream, gint *rtcp)
{
	g_assert(source && rtcp);
	
	if (G_UNLIKELY(stream >= N_STREAMS))
		return -E_INVAL;

	*rtcp = source->streams[stream].sock_info.rtcp_port;
	return 0;
}


static __inline__ gboolean
nmp_media_src_check_rtcp(JpfStreamSource *stream)
{
	JpfRtcpControl *control;

	control = (JpfRtcpControl*)&stream->rtcp_control;
	if (control->rtcp_recv.revents & READ_COND)
		return TRUE;

	if (control->rtcp_send.revents & WRITE_COND)
		return TRUE;

	return FALSE;
}


static __inline__ gint
nmp_media_src_recv_udp_stream_rtcp(JpfMediaSource *source,
	gint i_stream, JpfStreamSource *stream)
{
	JpfRtcpControl *control;
	gint err;

	control = (JpfRtcpControl*)&stream->rtcp_control;
	if (!(control->rtcp_recv.revents & READ_COND))
		return 0;

	err = nmp_rtcp_control_recv(&stream->rtcp_control, 0);
	if (err)
	{
		nmp_print(
			"Rtp sock error, ec:'%d'", err
		);
		return err;
	}

	control->rtcp_recv.revents &= ~READ_COND;
	return 0;
}


static __inline__ gint
nmp_media_src_tcp_est_rtcp(JpfMediaSource *source, JpfStreamSource *stream)
{
	gint err;

	err = nmp_media_sock_est_rtcp_sock(&stream->sock_info);
	if (!err)
	{
		g_source_remove_poll((GSource*)source, &stream->rtcp_control.rtcp_recv);
		nmp_media_src_init_rtcp_cb(&stream->rtcp_control, &stream->sock_info);
		nmp_media_src_add_poll_rtcp(source, &stream->rtcp_control);
		stream->rtp_connected = TRUE;
	}
	else
	{
		nmp_print(
			"Media src accept tcp rtcp sock failed, err:'%d'", err
		);
	}

	return err;
}


static __inline__ gint
nmp_media_src_recv_tcp_stream_rtcp(JpfMediaSource *source,
	gint i_stream, JpfStreamSource *stream)
{
	JpfRtcpControl *control;
	gint err;

	control = (JpfRtcpControl*)&stream->rtcp_control;
	if (!(control->rtcp_recv.revents & READ_COND))
		return 0;

	if (G_UNLIKELY(!stream->rtcp_connected))
	{
		return nmp_media_src_tcp_est_rtcp(source, stream);
	}

	err = nmp_rtcp_control_recv(&stream->rtcp_control, 0);
	if (err)
	{
		nmp_print(
			"Rtcp sock error, ec:'%d'", err
		);
		return err;
	}

	control->rtcp_recv.revents &= ~READ_COND;
	return 0;
}


static __inline__ gint
nmp_media_src_recv_udp_rtcp(JpfMediaSource *source)
{
	gint err, n_streams;

	n_streams = source->n_streams;
	while (--n_streams >= 0)
	{
		err = nmp_media_src_recv_udp_stream_rtcp(
			source,
			n_streams,
			&source->streams[n_streams]
		);

		if (G_UNLIKELY(err))
			return err;
	}

	return 0;
}


static __inline__ gint
nmp_media_src_recv_tcp_rtcp(JpfMediaSource *source)
{
	gint err, n_streams;

	n_streams = source->n_streams;
	while (--n_streams >= 0)
	{
		err = nmp_media_src_recv_tcp_stream_rtcp(
			source,
			n_streams,
			&source->streams[n_streams]
		);

		if (G_UNLIKELY(err))
			return err;
	}

	return 0;
}


#endif


static __inline__ gboolean
nmp_media_src_check_stream(JpfMediaSource *source, JpfStreamSource *stream)
{
	if (nmp_media_src_check_rtp(stream))
	{
		nmp_media_src_sync_time(source);
		return TRUE;
	}

#ifdef CONFIG_RTCP_SUPPORT
	if (nmp_media_src_check_rtcp(stream))
		return TRUE;
#endif

	return FALSE;
}


static gboolean
nmp_media_src_watch_prepare(GSource *source, gint *timeout)
{
	gint64 delta;
	JpfMediaSource *src = (JpfMediaSource*)source;

	if (!src->blockable)
	{
		delta = nmp_media_src_cal_timeout_delta(src);
		if (delta < 0)
		{
			*timeout = (-delta) * 1000;
			return FALSE;
		}
	}
	else
	{
		if (!src->blocking)
		{
			*timeout = -1;
			return FALSE;
		}

		delta = nmp_media_src_cal_sleeping_timeout_delta(src);
		if (delta < 0)
		{
			*timeout = -delta;
			return FALSE;
		}
	}

	*timeout = 0;
	src->err_code = -E_TIMEOUT;
	return TRUE;
}


static gboolean
nmp_media_src_watch_check(GSource *__source)
{
	gint n_streams = 0;
    JpfMediaSource *source;
    source = (JpfMediaSource*)__source;

    while (n_streams < source->n_streams)
    {
		if (nmp_media_src_check_stream(source, &source->streams[n_streams]))
			return TRUE;
    	++n_streams;
    }

    return FALSE;
}


static gboolean
nmp_media_src_watch_dispatch(GSource *__source, GSourceFunc callback,
    gpointer user_data)
{
    JpfMediaSource *source = (JpfMediaSource*)__source;
    JpfMediaSourceFuncs *funcs = source->funcs;

	if (source->err_code == -E_TIMEOUT)
	{
		if (!source->blockable)
		{
			nmp_warning(
				"Media '%p' src '%p' recv stream %d seconds timout!",
				source->media, source, MEDIA_SRC_TIMOUT_SECS
			);
			(*funcs->error)(source);
			return FALSE;			
		}
		else
		{
			source->err_code = 0;
			nmp_media_src_woken_up(source);
		}
		nmp_media_src_sync_time(source);
		return TRUE;
	}
	else
	{
	    if ((*funcs->recv)(source))
	    {
			(*funcs->error)(source);
			return FALSE;
		}
	    return TRUE;
	}
}


static void
nmp_media_src_watch_finalize(GSource *__source)
{
	JpfMediaSource *source = (JpfMediaSource*)__source;

	if (source->media)
	{
		nmp_media_src_destroy_streams(source);
		nmp_rtsp_media_unref(source->media);
	}

	nmp_print(
		"Media source '%p' finalized, Media '%p'!",
		source,
		source->media
	);
}


static GSourceFuncs nmp_media_src_source_funcs =
{
    .prepare        = nmp_media_src_watch_prepare,
    .check          = nmp_media_src_watch_check,
    .dispatch       = nmp_media_src_watch_dispatch,
    .finalize       = nmp_media_src_watch_finalize
};


static gint 
nmp_media_src_recv_udp_packet(JpfMediaSource *source)
{
	gint err;

	err = nmp_media_src_recv_udp_rtp(source);
#ifdef CONFIG_RTCP_SUPPORT
	if (G_LIKELY(!err))
		err = nmp_media_src_recv_udp_rtcp(source);
#endif

	return err;
}


static gint 
nmp_media_src_recv_tcp_packet(JpfMediaSource *source)
{
	gint err;

	err = nmp_media_src_recv_tcp_rtp(source);
#ifdef CONFIG_RTCP_SUPPORT
	if (G_LIKELY(!err))
		err = nmp_media_src_recv_tcp_rtcp(source);
#endif

	return err;
}


static void 
nmp_media_src_error(JpfMediaSource *source)
{
	extern void nmp_rtsp_media_zombie_it(void *media);

	if (source->media)
		nmp_rtsp_media_zombie_it(source->media);

	nmp_media_src_recycle_socket(source);

	nmp_print(
		"Media source '%p' error, stop working", source
	);
}


static JpfMediaSourceFuncs nmp_media_src_udp_watch_ops =
{
	.recv			= nmp_media_src_recv_udp_packet,
	.error			= nmp_media_src_error
};


static JpfMediaSourceFuncs nmp_media_src_tcp_watch_ops =
{
	.recv			= nmp_media_src_recv_tcp_packet,
	.error			= nmp_media_src_error
};


void
nmp_media_src_set_recv_fun(JpfMediaSource *source, 
	JpfMediaRecvFunc function)
{
	g_assert(source != NULL);
	source->func_recv = function;
}


static __inline__ gint
nmp_media_src_init_stream(JpfStreamSource *stream, JpfMediaTrans transport)
{
	gint err;

	memset(stream, 0, sizeof(*stream));
	err = nmp_media_sock_info_init(&stream->sock_info, transport);
	if (!err)
	{
		nmp_media_src_init_rtp_pollfd(&stream->rtp_read, &stream->sock_info);
#ifdef CONFIG_RTCP_SUPPORT
		nmp_media_src_init_rtcp_cb(&stream->rtcp_control, &stream->sock_info);
#endif
	}

#ifdef CONFIG_MEDIA_SRC_USE_BLOCKABLE_IO
	if (transport == MEDIA_TRANS_TCP)
	{
		stream->data_block = g_malloc(MAX_BLOCK_IO_SIZE);
		stream->block_capacity = MAX_BLOCK_IO_SIZE;
	}
#endif

	return err;
}


static __inline__ void
nmp_media_src_stream_say_bye(JpfStreamSource *stream)
{
	if (stream->rtp_said_bye)
		return;
	stream->rtp_said_bye = TRUE;

#ifdef CONFIG_RTCP_SUPPORT
	nmp_rtcp_control_bye(&stream->rtcp_control);
#endif
}


static __inline__ void
nmp_media_src_recycle_stream_sock(JpfStreamSource *stream)
{
	nmp_media_src_stream_say_bye(stream);
	nmp_media_sock_info_reset(&stream->sock_info);
}


static __inline__ void
nmp_media_src_destroy_stream(JpfStreamSource *stream)
{
	nmp_media_src_recycle_stream_sock(stream);

#ifdef CONFIG_MEDIA_SRC_USE_BLOCKABLE_IO
	if (stream->data_block)
	{
		g_free(stream->data_block);
		stream->data_block = NULL;
	}
#endif
}


static __inline__ void
nmp_media_src_destroy_streams(JpfMediaSource *source)
{
	gint n_streams = source->n_streams;

	nmp_audit_dec(AUDIT_CNT_MEDIA_SRC, 1);
	nmp_audit_dec(AUDIT_CNT_STREAM_SRC, n_streams);

	while (--n_streams >= 0)
	{
		nmp_media_src_destroy_stream(&source->streams[n_streams]);
	}
}


static __inline__ void
nmp_media_src_recycle_socket(JpfMediaSource *source)
{
	gint n_streams = source->n_streams;

	while (--n_streams >= 0)
	{
		nmp_media_src_recycle_stream_sock(&source->streams[n_streams]);
	}	
}


static __inline__ gint
nmp_media_src_init_streams(JpfMediaSource *source, gint streams,
	JpfMediaTrans transport)
{
	gint n_streams, err;

	memset(source->streams, 0, sizeof(source->streams));
	for (n_streams = 0; n_streams < streams; ++n_streams)
	{
		if ((err = nmp_media_src_init_stream(
				&source->streams[n_streams], transport)))
		{
			goto init_stream_failed;
		}
	}

	source->n_streams = streams;
	return 0;

init_stream_failed:
	for (; --n_streams >= 0;)
	{
		nmp_media_src_destroy_stream(&source->streams[n_streams]);
	}
	return err;
}


static __inline__ void
nmp_media_src_add_stream_poll(JpfMediaSource *source, JpfStreamSource *stream,
	gint rtcp)
{
	if (!stream->gpfd_added)
	{
		g_source_add_poll((GSource*)source, &stream->rtp_read);
		stream->gpfd_added = TRUE;
	}

#ifdef CONFIG_RTCP_SUPPORT
	if (rtcp)
	{
		if (!stream->rtcp_fd_added)
		{
			nmp_media_src_add_poll_rtcp(source, &stream->rtcp_control);
			stream->rtcp_fd_added = TRUE;
		}
	}
#endif
}


static __inline__ void
nmp_media_src_remove_stream_poll(JpfMediaSource *source, JpfStreamSource *stream,
	gint rtcp)
{
	if (stream->gpfd_added)
	{
		g_source_remove_poll((GSource*)source, &stream->rtp_read);
		stream->gpfd_added = FALSE;
	}

#ifdef CONFIG_RTCP_SUPPORT
	if (rtcp)
	{
		if (stream->rtcp_fd_added)
		{
			nmp_media_src_remove_poll_rtcp(source, &stream->rtcp_control);
			stream->rtcp_fd_added = FALSE;
		}
	}
#endif
}


static __inline__ void
nmp_media_src_add_streams_poll(JpfMediaSource *source, gint rtcp)
{
	gint n_streams = 0;

	for (; n_streams < source->n_streams; ++n_streams)
		nmp_media_src_add_stream_poll(source, &source->streams[n_streams], rtcp);
}


static __inline__ void
nmp_media_src_remove_streams_poll(JpfMediaSource *source, gint rtcp)
{
	gint n_streams = 0;

	for (; n_streams < source->n_streams; ++n_streams)
		nmp_media_src_remove_stream_poll(source, &source->streams[n_streams], rtcp);
}


JpfMediaSource *
nmp_media_src_create_source(gpointer media, gint streams, 
	JpfMediaTrans transport, gint blockable)
{
	JpfMediaSource *source;

	if (G_UNLIKELY(streams > N_STREAMS))
	{
		nmp_warning(
			"Create source for media '%p' failed, streams(%d)>N_STREAMS",
			media, streams
		);
		return NULL;
	}

	source = (JpfMediaSource*)g_source_new(
		&nmp_media_src_source_funcs, sizeof(JpfMediaSource));
	source->media = NULL;

	if (nmp_media_src_init_streams(source, streams, transport))
	{
		nmp_warning(
			"Init media '%p' source streams failed.", media
		);
		g_source_unref((GSource*)source);
		return NULL;
	}

	source->funcs = transport == MEDIA_TRANS_TCP
	                ? &nmp_media_src_tcp_watch_ops
	                : &nmp_media_src_udp_watch_ops;

	source->last_check = 0;
	source->err_code = 0;
	source->blockable = blockable;
	source->blocking = 0;
	source->media = media;
	source->func_recv = NULL;
	source->user_data = NULL;
	nmp_media_src_add_streams_poll(source, 1);

	nmp_audit_inc(AUDIT_CNT_MEDIA_SRC, 1);
	nmp_audit_inc(AUDIT_CNT_STREAM_SRC, streams);

	return source;
}


void
nmp_media_src_attach(JpfMediaSource *source)
{
	G_ASSERT(source != NULL);

	nmp_media_src_sync_time(source);
	nmp_scheduler_add((GSource*)source, WEIGHT_STREAM_IN);
}


gint
nmp_media_src_get_rtp_port(JpfMediaSource *source, gint stream, gint *rtp)
{
	g_assert(source && rtp);

	if (G_UNLIKELY(stream >= N_STREAMS))
		return -E_INVAL;

	*rtp = source->streams[stream].sock_info.rtp_port;
	return 0;
}


//:~ End
