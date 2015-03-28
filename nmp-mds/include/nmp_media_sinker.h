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
}NmpSinkWatchState;


typedef struct _NmpSinkerStream NmpSinkerStream;
struct _NmpSinkerStream
{
	NmpMediaSocket		sock_info;		/* stream socket infomation */

	gboolean			connected;		/* data link connected ? */
	gboolean			rtp_over_rtsp;	/* over rtsp ? */
	gboolean			pending;		/* write pending ?*/

	GPollFD				rtp_read;		/* fd for recving */
	GPollFD				rtp_write;		/* fd for sending */
	NmpSmartBuffer		rtp_buffer;		/* for packet buffering */

	gint				rtp_ch;			/* a union is perfect */

	NmpSinkWatchState	state;			/* watch state */
	NmpRingBuffer		*ring_buff;		/* ring buffer for data buffering */

#ifdef CONFIG_RTCP_SUPPORT
	NmpRtcpControl		rtcp_control;	/* rtcp control block */
#endif
};


typedef struct _NmpSinkerWatch NmpSinkerWatch;
typedef struct _NmpSinkerWatchFuncs NmpSinkerWatchFuncs;

struct _NmpSinkerWatchFuncs			/* sink watch functions set */
{
	gint (*send)(NmpSinkerWatch *sinker, NmpSinkerStream *stream);	/* send data out */
	gint (*fill)(NmpSinkerWatch *sinker, NmpSinkerStream *stream,
		gchar *data, gsize size);	/* fill sinker with data from src */
	void (*error)(NmpSinkerWatch *sinker);	/* invoked when watch errors */
};


struct _NmpSinkerWatch
{
	GSource				source;			/* GSource object */
	GstRTSPLowerTrans	trans;			/* transport style */
	NmpSinkerStream		streams[NSTREAMS]; /* streams of media */

	gint				n_streams;

	void				*tcp_sinker;	/* RTP/RTCP Over RTSP , pointer to client,
	we make sure that the lifetime of client longer than sinker, so we don't ref it */

	NmpSinkerWatchFuncs	*funcs;			/* sinker watch functions set */

	gboolean			sink_ready;		/* ready to comsume data */
	GStaticMutex		lock;			/* watch lock */
};


typedef struct _NmpMediaSinkers NmpMediaSinkers;
struct _NmpMediaSinkers
{
	gint				ref_count;
	GPtrArray			*sinkers;		/* sinker watches set */
	gint				n_sinkers;		/* sinkers count */
	gpointer			media;			/* owner, media object */
	gint				blockable;		/* all sinkers are block-able */
	GStaticMutex		lock;			/* protected all above */
};


NmpMediaSinkers *nmp_media_sinkers_new(gpointer media, gint blockable);
NmpMediaSinkers *nmp_media_sinkers_ref(NmpMediaSinkers *sinkers);
void nmp_media_sinkers_unref(NmpMediaSinkers *sinkers);

void nmp_media_sinkers_add(NmpMediaSinkers *sinkers, NmpSinkerWatch *w);
gint nmp_media_sinkers_del(NmpMediaSinkers *sinkers, NmpSinkerWatch *w);

NmpSinkerWatch *nmp_media_sinker_create(GstRTSPLowerTrans trans,
	gint streams, gint *err);

void nmp_media_sinker_ref(NmpSinkerWatch *watch);
void nmp_media_sinker_unref(NmpSinkerWatch *watch);

gint nmp_media_sinker_set_transport(NmpSinkerWatch *sinker, gint i_stream,
	gchar *client_Ip, GstRTSPTransport *ct, gboolean b_nat, void *p);

gint nmp_media_sinkers_recv_packet(NmpMediaSinkers *sinkers, gint stream,
	gchar *data, gsize size);

void nmp_media_sinker_play(NmpSinkerWatch *watch);

gint nmp_media_sinker_watch_write(NmpSinkerWatch *watch, gint stream,
	gchar *data, gsize size);

#endif	/* __NMP_MEDIA_SINKER_H__ */
