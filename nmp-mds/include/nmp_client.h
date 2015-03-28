#ifndef __NMP_CLIENT_H__
#define __NMP_CLIENT_H__

#include <rtspwatch.h>
#include "nmp_media.h"
#include "nmp_media_sinker.h"

#define CLIENT_SESSION_ID_LEN				32
#define DEFAULT_SESSSION_TIMEOUT			15

typedef enum
{
	JPF_RTSP_STAT_NORMAL,
	JPF_RTSP_STAT_KILLED
}JpfClientState;


typedef struct _JpfRtspClient JpfRtspClient;
struct _JpfRtspClient
{
	gint					 ref_count;
	JpfClientState			 state;

	GstRtspWatch			*watch;			/* RTSP watch , gst component */

	GList					*sessions;		/* sessions attached to this client, always only 1 */
	gchar             		*client_ip;		/* client ip */
	guint					port;			/* client port */
	GMutex					*lock;

	gint					ttl;
	gint					ttd;			/* time to death (seconds) */

	gpointer				 client_mng;
};


typedef struct _JpfRtspSession JpfRtspSession;
struct _JpfRtspSession
{
	gchar					sid[CLIENT_SESSION_ID_LEN];
	gint					timeout;

	JpfRtspMedia			*media;
	JpfSinkerWatch			*sinker;
};


JpfRtspSession *nmp_rtsp_create_session(JpfRtspClient *client, 
	JpfRtspMedia *media, GstRTSPTransport *ct);

JpfRtspClient *nmp_rtsp_client_new( void );

void nmp_rtsp_client_attach(JpfRtspClient *client, void *context);
gboolean nmp_rtsp_client_accept(JpfRtspClient *client,
	GEvent *e_listen, gint *errp);

JpfRtspClient *nmp_rtsp_client_ref(JpfRtspClient *client);
void nmp_rtsp_client_unref(JpfRtspClient *client);

void nmp_rtsp_client_send_response(JpfRtspClient *client,
	JpfRtspSession *session, GstRTSPMessage *response);

gint nmp_rtsp_session_attach_media(JpfRtspSession *s, 
	JpfRtspMedia *media, JpfSinkerWatch *w);

JpfRtspSession *nmp_rtsp_client_find_session(JpfRtspClient *client, 
	gchar *session_id);

gint nmp_rtsp_client_play_failed(JpfRtspClient *client, 
	gchar *session_id);

gint nmp_rtsp_client_play_session(JpfRtspClient *client, 
	gchar *session_id);

gint nmp_rtsp_client_delete_session(JpfRtspClient *client, 
	gchar *session_id);

void nmp_rtsp_client_set_illegal(JpfRtspClient *client);

gint nmp_rtsp_client_timeout(gconstpointer c, gconstpointer  u);

void nmp_rtsp_client_alive(JpfRtspClient *client);

gint nmp_rtsp_client_send_message(void *cli, GstRTSPMessage *msg);

#endif	/* __NMP_CLIENT_H__ */
