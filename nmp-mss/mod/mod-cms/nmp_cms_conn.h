#ifndef __NMP_CMS_CONN_H__
#define __NMP_CMS_CONN_H__

#include <glib.h>
#include "nmp_message.h"

G_BEGIN_DECLS

// MAX_IP_LEN defined in nmp_msg_struct.h

typedef enum
{
	DISCONNECTED,
	CONNECTING,
	CONNECTED,
	DISCONNECTING
}NmpConnState;


typedef struct __NmpCmsConn NmpCmsConn;
struct __NmpCmsConn
{
	NmpNetIO	*bridge;		/* communication bridge */

	gchar		ip[MAX_IP_LEN];	/* cms ip */
	gint		port;			/* cms port */

	gint		rtsp_port;		/* mss rtsp port */

	GMutex		*lock;
	NmpConnState state;
	guint		state_timer;
	guint 		ttl;
	guint 		ttd;
	guint		hb;
	guint 		unregistered;
	guint		seq_generator;
};

NmpCmsConn *nmp_create_cms_conn( void );
gint nmp_cms_conn_state(NmpCmsConn *conn);
void nmp_cms_conn_set_registered(NmpCmsConn *conn, NmpmssRegisterRes *res);
void nmp_cms_conn_set_register_failed(NmpCmsConn *conn, gint err);

void nmp_cms_conn_is_alive(NmpCmsConn *conn);

void nmp_cms_conn_io_fin(NmpCmsConn *conn, NmpNetIO *bridge);

NmpNetIO *nmp_cms_conn_get_io(NmpCmsConn *conn);


G_END_DECLS

#endif	/* __NMP_CMS_CONN_H__ */
