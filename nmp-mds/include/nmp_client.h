#ifndef __NMP_CLIENT_H__
#define __NMP_CLIENT_H__

#include <rtspwatch.h>
#include "nmp_media.h"
#include "nmp_media_sinker.h"

#define CLIENT_SESSION_ID_LEN				32
#define DEFAULT_SESSSION_TIMEOUT			15

typedef enum
{
	NMP_RTSP_STAT_NORMAL,
	NMP_RTSP_STAT_KILLED
}NmpClientState;


typedef struct _NmpRtspClient NmpRtspClient;
struct _NmpRtspClient
{
	gint					 ref_count;
	NmpClientState			 state;

	GstRtspWatch			*watch;			/* RTSP watch , gst component */

	GList					*sessions;		/* sessions attached to this client, always only 1 */
	gchar             		*client_ip;		/* client ip */
	guint					port;			/* client port */
	GMutex					*lock;

	gint					ttl;
	gint					ttd;			/* time to death (seconds) */

	gpointer				 client_mng;
};


typedef struct _NmpRtspSession NmpRtspSession;
struct _NmpRtspSession
{
	gchar					sid[CLIENT_SESSION_ID_LEN];
	gint					timeout;

	NmpRtspMedia			*media;
	NmpSinkerWatch			*sinker;
};


NmpRtspSession *nmp_rtsp_create_session(NmpRtspClient *client, 
	NmpRtspMedia *media, GstRTSPTransport *ct);

NmpRtspClient *nmp_rtsp_client_new( void );

void nmp_rtsp_client_attach(NmpRtspClient *client, void *context);
gboolean nmp_rtsp_client_accept(NmpRtspClient *client,
	GEvent *e_listen, gint *errp);

NmpRtspClient *nmp_rtsp_client_ref(NmpRtspClient *client);
void nmp_rtsp_client_unref(NmpRtspClient *client);

void nmp_rtsp_client_send_response(NmpRtspClient *client,
	NmpRtspSession *session, GstRTSPMessage *response);

gint nmp_rtsp_session_attach_media(NmpRtspSession *s, 
	NmpRtspMedia *media, NmpSinkerWatch *w);

NmpRtspSession *nmp_rtsp_client_find_session(NmpRtspClient *client, 
	gchar *session_id);

gint nmp_rtsp_client_play_failed(NmpRtspClient *client, 
	gchar *session_id);

gint nmp_rtsp_client_play_session(NmpRtspClient *client, 
	gchar *session_id);

gint nmp_rtsp_client_delete_session(NmpRtspClient *client, 
	gchar *session_id);

void nmp_rtsp_client_set_illegal(NmpRtspClient *client);

gint nmp_rtsp_client_timeout(gconstpointer c, gconstpointer  u);

void nmp_rtsp_client_alive(NmpRtspClient *client);

gint nmp_rtsp_client_send_message(void *cli, GstRTSPMessage *msg);

#endif	/* __NMP_CLIENT_H__ */
