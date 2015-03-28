#ifndef __NMP_MEDIA_SINKER_H__
#define __NMP_MEDIA_SINKER_H__

#include "nmp_config.h"
#include "nmp_sock_info.h"
#include "nmp_smart_buffer.h"
#include "nmp_ring_buffer.h"

#ifdef CONFIG_RTCP_SUPPORT
#include "nmp_rtcp_control.h"
#endif

#define NSTREAMS	2

typedef enum
{
	SINKER_WATCH_STAT_EMPTY,
	SINKER_WATCH_STAT_PENDING
}JpfSinkWatchState;


typedef struct _JpfSinkerStream JpfSinkerStream;
struct _JpfSinkerStream
{
	JpfMediaSocket		sock_info;		/* stream socket infomation */

	gboolean			connected;		/* data link connected ? */
	gboolean			rtp_over_rtsp;	/* over rtsp ? */
	gboolean			pending;		/* write pending ?*/

	GPollFD				rtp_read;		/* fd for recving */
	GPollFD				rtp_write;		/* fd for sending */
	JpfSmartBuffer		rtp_buffer;		/* for packet buffering */

	gint				rtp_ch;			/* a union is perfect */

	JpfSinkWatchState	state;			/* watch state */
	JpfRingBuffer		*ring_buff;		/* ring buffer for data buffering */

#ifdef CONFIG_RTCP_SUPPORT
	JpfRtcpControl		rtcp_control;	/* rtcp control block */
#endif
};


typedef struct _JpfSinkerWatch JpfSinkerWatch;
typedef struct _JpfSinkerWatchFuncs JpfSinkerWatchFuncs;

struct _JpfSinkerWatchFuncs			/* sink watch functions set */
{
	gint (*send)(JpfSinkerWatch *sinker, JpfSinkerStream *stream);	/* send data out */
	gint (*fill)(JpfSinkerWatch *sinker, JpfSinkerStream *stream,
		gchar *data, gsize size);	/* fill sinker with data from src */
	void (*error)(JpfSinkerWatch *sinker);	/* invoked when watch errors */
};


struct _JpfSinkerWatch
{
	GSource				source;			/* GSource object */
	GstRTSPLowerTrans	trans;			/* transport style */
	JpfSinkerStream		streams[NSTREAMS]; /* streams of media */

	gint				n_streams;

	void				*tcp_sinker;	/* RTP/RTCP Over RTSP , pointer to client,
	we make sure that the lifetime of client longer than sinker, so we don't ref it */

	JpfSinkerWatchFuncs	*funcs;			/* sinker watch functions set */

	gboolean			sink_ready;		/* ready to comsume data */
	GStaticMutex		lock;			/* watch lock */
};


typedef struct _JpfMediaSinkers JpfMediaSinkers;
struct _JpfMediaSinkers
{
	gint				ref_count;
	GPtrArray			*sinkers;		/* sinker watches set */
	gint				n_sinkers;		/* sinkers count */
	gpointer			media;			/* owner, media object */
	gint				blockable;		/* all sinkers are block-able */
	GStaticMutex		lock;			/* protected all above */
};


JpfMediaSinkers *nmp_media_sinkers_new(gpointer media, gint blockable);
JpfMediaSinkers *nmp_media_sinkers_ref(JpfMediaSinkers *sinkers);
void nmp_media_sinkers_unref(JpfMediaSinkers *sinkers);

void nmp_media_sinkers_add(JpfMediaSinkers *sinkers, JpfSinkerWatch *w);
gint nmp_media_sinkers_del(JpfMediaSinkers *sinkers, JpfSinkerWatch *w);

JpfSinkerWatch *nmp_media_sinker_create(GstRTSPLowerTrans trans,
	gint streams, gint *err);

void nmp_media_sinker_ref(JpfSinkerWatch *watch);
void nmp_media_sinker_unref(JpfSinkerWatch *watch);

gint nmp_media_sinker_set_transport(JpfSinkerWatch *sinker, gint i_stream,
	gchar *client_Ip, GstRTSPTransport *ct, gboolean b_nat, void *p);

gint nmp_media_sinkers_recv_packet(JpfMediaSinkers *sinkers, gint stream,
	gchar *data, gsize size);

void nmp_media_sinker_play(JpfSinkerWatch *watch);

gint nmp_media_sinker_watch_write(JpfSinkerWatch *watch, gint stream,
	gchar *data, gsize size);

#endif	/* __NMP_MEDIA_SINKER_H__ */
