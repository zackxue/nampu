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
	NMP_MEDIA_STAT_NEW,
	NMP_MEDIA_STAT_READY,
	NMP_MEDIA_STAT_NULL
}NmpMediaState;


typedef enum
{
	NMP_SESSION_DESCRIBE,
	NMP_SESSION_SETUP,
	NMP_SESSION_PLAY,
	NMP_SESSION_OPTIONS,
	NMP_SESSION_TEARDOWN
}NmpSessionState;


typedef enum
{
	NMP_MT_UDP,
	NMP_MT_TCP
}NmpDeviceMediaType;


enum
{
	MS_LIVE = 0
};

/*
 * "/dev=DM-12345@JXJ-DVS-12345678[%vdu=192.168.1.13&9901%key=7546211]/key=8566142/media=0/channel=01&level=1/stream=0",
*/
typedef struct _NmpMediaUri NmpMediaUri;
struct _NmpMediaUri
{
	gchar			device[MAX_MEDIA_LOCTION_LEN];	/* device identification */
	gchar			mrl[MAX_MEDIA_MRL_LEN];	/* uri in dev */

	gint			type;			/* media=0/1, live/history */
	gint			channel;		/* media source channel */
	gint			rate_level;		/* major, minor, ... */
	gint			sequence;		/* media sequence */

	gchar			*media_base;	/* base url */
};


typedef struct _NmpMediaSession NmpMediaSession;
struct _NmpMediaSession
{
	gchar				sid[MAX_SESSION_ID_LEN];	/* session id */

	gint				cseq;		/* seq waiting for */
	guint				expire;		/* time expire */
	gint				stream;		/* request stream no. */
	NmpSessionState		state;		/* session state */
};


typedef struct _NmpMediaTrack NmpMediaTrack;
struct _NmpMediaTrack
{
	gchar				name[MAX_TRACK_NAME_LEN];
};


typedef struct _NmpRtspMedia NmpRtspMedia;
typedef struct _NmpRtspMediaClass NmpRtspMediaClass;

struct _NmpRtspMedia
{
	gint				ref_count;
	GMutex				*lock;
	NmpMediaState		state;

	NmpMediaUri			*media_uri;	/* media uri information */
	NmpMediaSession		*session;	/* media session */

	NmpMediaSinkers		*sink;		/* media sink element */
	NmpMediaSource		*src;		/* media source element */

	GstSDPMessage		*sdp;		/* sdp info the device announced */

	gint				streams;	/* media streams count */
	NmpMediaTrack		track_map[NSTREAMS];	/* i_stream <=> track map */
	gint				ready_to_play;	/* ready ? at least 1 sinker */

	NmpDeviceMediaType	mt;			/* device media type */
	gint				zombie;		/* enter zombie state */

	guint				timer;		/* media timer */

	gpointer			device;		/* media device */

	NmpPendingQueue		r_pending;	/* requests pending queue */
};


NmpRtspMedia *nmp_rtsp_media_new( void );
void nmp_rtsp_media_ref(NmpRtspMedia *media);
void nmp_rtsp_media_unref(NmpRtspMedia *media);
void nmp_rtsp_media_kill_unref(NmpRtspMedia *media);
gint nmp_rtsp_media_prepare(NmpRtspMedia *media, NmpDeviceMediaType mt);

void nmp_rtsp_media_attach_device(NmpRtspMedia *media, gpointer device);
void nmp_rtsp_media_detach_device(NmpRtspMedia *media, gpointer device);

NmpMediaUri *nmp_rtsp_media_uri_dup(const NmpMediaUri *uri);
NmpSessionState nmp_rtsp_media_get_session_state(NmpRtspMedia *media);
gint nmp_rtsp_media_session_expire(NmpRtspMedia *media);
void nmp_rtsp_media_set_session_id(NmpRtspMedia *media, gchar *sid);
void nmp_rtsp_media_set_session_seq(NmpRtspMedia *media, gint seq);
NmpSessionState nmp_rtsp_media_session_state_next(NmpRtspMedia *media);
gboolean nmp_rtsp_media_has_teardown(NmpRtspMedia *media);

void nmp_rtsp_media_zombie_it(void *media);

gint nmp_rtsp_media_play_waiting(NmpRtspMedia *media);
void nmp_rtsp_media_die_announce(NmpRtspMedia *media, gpointer device);

void nmp_rtsp_media_set_sdp_info(NmpRtspMedia *media, GstSDPMessage *sdp);
void nmp_rtsp_media_get_sdp_info(NmpRtspMedia *media, guint8 **sdp, guint *size);

gint nmp_rtsp_media_make_request(NmpRtspMedia *media,
	GstRTSPMessage **request);

gint nmp_rtsp_media_make_param_request(NmpRtspMedia *media,
	GstRTSPMessage **request, gint name, gpointer value);

gint nmp_rtsp_media_sinker_play(NmpRtspMedia *media, NmpSinkerWatch *sinker);

NmpSinkerWatch *nmp_media_create_sinker(NmpRtspMedia *media, GstRTSPLowerTrans trans,
	gint *err);
gint nmp_media_set_sinker_transport(NmpRtspMedia *media, NmpSinkerWatch *sinker,
	NmpMediaTrack *track, gchar *client_Ip, GstRTSPTransport *ct, gboolean b_nat, void *p);
	
gint nmp_rtsp_media_add_sinker(NmpRtspMedia *media, NmpSinkerWatch *sinker);
void nmp_rtsp_media_del_sinker(NmpRtspMedia *media, NmpSinkerWatch *sinker);

gint nmp_rtsp_media_pending_request_state(NmpRtspMedia *media, NmpMediaState s,
	PQN_DATA_T data_1, PQN_DATA_T data_2, PQN_DATA_T data_3, NmpPQNFunc fun);

void nmp_rtsp_media_deal_pending_request(NmpRtspMedia *media);

gboolean nmp_rtsp_media_destroyed(NmpRtspMedia *media);

G_END_DECLS

#endif /* __NMP_RTSP_MEDIA_H__ */
