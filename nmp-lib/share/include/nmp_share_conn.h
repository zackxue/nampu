#ifndef __NMP_SHARE_CONNECTION_H__
#define __NMP_SHARE_CONNECTION_H__

#include <glib.h>
#include <netinet/in.h>

G_BEGIN_DECLS

typedef struct _NmpConnection NmpConnection;

enum _NmpConnFlags
{
	CF_TYPE_TCP = 0x01,
	CF_TYPE_UDP = 0x02,
	CF_FLGS_NONBLOCK = 0x100,
	CF_FLGS_CONN_INPROGRESS =0x200,
	CF_FLGS_IO_HEAVY = 0x1000
};

gint nmp_resolve_host(struct sockaddr_in *sin, gchar *host, gint port);

NmpConnection *nmp_connection_new(struct sockaddr *sa,
	guint flags, gint *errp);

gint nmp_connection_listen(NmpConnection *conn);
gint nmp_connection_get_fd(NmpConnection *conn);
gint nmp_connection_set_flags(NmpConnection *conn, guint flgs);
gint nmp_connection_is_blocked(NmpConnection *conn);
NmpConnection *nmp_connection_accept(NmpConnection *listen, gint *errp);
gint nmp_connection_is_ingrogress(NmpConnection *conn, int clear);
gint nmp_connection_connect(NmpConnection *conn, struct sockaddr *sa);
gint nmp_connection_read(NmpConnection *conn, gchar buf[], gsize size);
gint nmp_connection_write(NmpConnection *conn, gchar *buf, gsize size);
void nmp_connection_close(NmpConnection *conn);

void nmp_connection_set_buffer_size(NmpConnection *conn, gint size);
gint nmp_connection_get_buffer_size(NmpConnection *conn);
void nmp_connection_set_heavy(NmpConnection *conn);
gint nmp_connection_is_heavy(NmpConnection *conn);
gint nmp_connection_get_timeout(NmpConnection *conn);
void nmp_connection_set_timeout(NmpConnection *conn, gint millisec);

gchar *nmp_connection_get_peer(NmpConnection *conn);

G_END_DECLS

#endif	//__NMP_SHARE_CONNECTION_H__
