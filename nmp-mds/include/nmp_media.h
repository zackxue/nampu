#ifndef __NMP_RTSP_MEDIA_H__
#define __NMP_RTSP_MEDIA_H__

#include "nmp_media_sinker.h"
#include "nmp_media_src.h"
#include "nmp_pq.h"

G_BEGIN_DECLS

#define MAX_SESSION_ID_LEN			32
#define MAX_MEDIA_LOCTION_LEN		64
#define MAX_TRACK_NAME_LEN			32
#define MAX_MEDIA_MRL_LEN			1024

typedef enum
{
	JPF_MEDIA_STAT_NEW,
	JPF_MEDIA_STAT_READY,
	JPF_MEDIA_STAT_NULL
}JpfMediaState;


typedef enum
{
	JPF_SESSION_DESCRIBE,
	JPF_SESSION_SETUP,
	JPF_SESSION_PLAY,
	JPF_SESSION_OPTIONS,
	JPF_SESSION_TEARDOWN
}JpfSessionState;


typedef enum
{
	JPF_MT_UDP,
	JPF_MT_TCP
}JpfDeviceMediaType;


enum
{
	MS_LIVE = 0
};

/*
 * "/dev=DM-12345@JXJ-DVS-12345678[%vdu=192.168.1.13&9901%key=7546211]/key=8566142/media=0/channel=01&level=1/stream=0",
*/
typedef struct _JpfMediaUri JpfMediaUri;
struct _JpfMediaUri
{
	gchar			device[MAX_MEDIA_LOCTION_LEN];	/* device identification */
	gchar			mrl[MAX_MEDIA_MRL_LEN];	/* uri in dev */

	gint			type;			/* media=0/1, live/history */
	gint			channel;		/* media source channel */
	gint			rate_level;		/* major, minor, ... */
	gint			sequence;		/* media sequence */

	gchar			*media_base;	/* base url */
};


typedef struct _JpfMediaSession JpfMediaSession;
struct _JpfMediaSession
{
	gchar				sid[MAX_SESSION_ID_LEN];	/* session id */

	gint				cseq;		/* seq waiting for */
	guint				expire;		/* time expire */
	gint				stream;		/* request stream no. */
	JpfSessionState		state;		/* session state */
};


typedef struct _JpfMediaTrack JpfMediaTrack;
struct _JpfMediaTrack
{
	gchar				name[MAX_TRACK_NAME_LEN];
};


typedef struct _JpfRtspMedia JpfRtspMedia;
typedef struct _JpfRtspMediaClass JpfRtspMediaClass;

struct _JpfRtspMedia
{
	gint				ref_count;
	GMutex				*lock;
	JpfMediaState		state;

	JpfMediaUri			*media_uri;	/* media uri information */
	JpfMediaSession		*session;	/* media session */

	JpfMediaSinkers		*sink;		/* media sink element */
	JpfMediaSource		*src;		/* media source element */

	GstSDPMessage		*sdp;		/* sdp info the device announced */

	gint				streams;	/* media streams count */
	JpfMediaTrack		track_map[NSTREAMS];	/* i_stream <=> track map */
	gint				ready_to_play;	/* ready ? at least 1 sinker */

	JpfDeviceMediaType	mt;			/* device media type */
	gint				zombie;		/* enter zombie state */

	guint				timer;		/* media timer */

	gpointer			device;		/* media device */

	JpfPendingQueue		r_pending;	/* requests pending queue */
};


JpfRtspMedia *nmp_rtsp_media_new( void );
void nmp_rtsp_media_ref(JpfRtspMedia *media);
void nmp_rtsp_media_unref(JpfRtspMedia *media);
void nmp_rtsp_media_kill_unref(JpfRtspMedia *media);
gint nmp_rtsp_media_prepare(JpfRtspMedia *media, JpfDeviceMediaType mt);

void nmp_rtsp_media_attach_device(JpfRtspMedia *media, gpointer device);
void nmp_rtsp_media_detach_device(JpfRtspMedia *media, gpointer device);

JpfMediaUri *nmp_rtsp_media_uri_dup(const JpfMediaUri *uri);
JpfSessionState nmp_rtsp_media_get_session_state(JpfRtspMedia *media);
gint nmp_rtsp_media_session_expire(JpfRtspMedia *media);
void nmp_rtsp_media_set_session_id(JpfRtspMedia *media, gchar *sid);
void nmp_rtsp_media_set_session_seq(JpfRtspMedia *media, gint seq);
JpfSessionState nmp_rtsp_media_session_state_next(JpfRtspMedia *media);
gboolean nmp_rtsp_media_has_teardown(JpfRtspMedia *media);

void nmp_rtsp_media_zombie_it(void *media);

gint nmp_rtsp_media_play_waiting(JpfRtspMedia *media);
void nmp_rtsp_media_die_announce(JpfRtspMedia *media, gpointer device);

void nmp_rtsp_media_set_sdp_info(JpfRtspMedia *media, GstSDPMessage *sdp);
void nmp_rtsp_media_get_sdp_info(JpfRtspMedia *media, guint8 **sdp, guint *size);

gint nmp_rtsp_media_make_request(JpfRtspMedia *media,
	GstRTSPMessage **request);

gint nmp_rtsp_media_make_param_request(JpfRtspMedia *media,
	GstRTSPMessage **request, gint name, gpointer value);

gint nmp_rtsp_media_sinker_play(JpfRtspMedia *media, JpfSinkerWatch *sinker);

JpfSinkerWatch *nmp_media_create_sinker(JpfRtspMedia *media, GstRTSPLowerTrans trans,
	gint *err);
gint nmp_media_set_sinker_transport(JpfRtspMedia *media, JpfSinkerWatch *sinker,
	JpfMediaTrack *track, gchar *client_Ip, GstRTSPTransport *ct, gboolean b_nat, void *p);
	
gint nmp_rtsp_media_add_sinker(JpfRtspMedia *media, JpfSinkerWatch *sinker);
void nmp_rtsp_media_del_sinker(JpfRtspMedia *media, JpfSinkerWatch *sinker);

gint nmp_rtsp_media_pending_request_state(JpfRtspMedia *media, JpfMediaState s,
	PQN_DATA_T data_1, PQN_DATA_T data_2, PQN_DATA_T data_3, JpfPQNFunc fun);

void nmp_rtsp_media_deal_pending_request(JpfRtspMedia *media);

gboolean nmp_rtsp_media_destroyed(JpfRtspMedia *media);

G_END_DECLS

#endif /* __NMP_RTSP_MEDIA_H__ */
