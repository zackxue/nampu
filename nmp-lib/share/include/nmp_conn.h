#ifndef __NMP_CONNECTION_H__
#define __NMP_CONNECTION_H__

#include <glib.h>
#include <netinet/in.h>

G_BEGIN_DECLS

typedef struct _HmConnection HmConnection;

enum _HmConnFlags
{
	CF_TYPE_TCP = 0x01,
	CF_TYPE_UDP = 0x02,
	CF_FLGS_NONBLOCK = 0x100,
	CF_FLGS_CONN_INPROGRESS =0x200,
	CF_FLGS_IO_HEAVY = 0x1000
};

gint jpf_resolve_host(struct sockaddr_in *sin, gchar *host, gint port);

HmConnection *jpf_connection_new(struct sockaddr *sa,
	guint flags, gint *errp);

gint jpf_connection_listen(HmConnection *conn);
gint jpf_connection_get_fd(HmConnection *conn);
gint jpf_connection_set_flags(HmConnection *conn, guint flgs);
gint jpf_connection_is_blocked(HmConnection *conn);
HmConnection *jpf_connection_accept(HmConnection *listen, gint *errp);
gint jpf_connection_is_ingrogress(HmConnection *conn, int clear);
gint jpf_connection_connect(HmConnection *conn, struct sockaddr *sa);
gint jpf_connection_read(HmConnection *conn, gchar buf[], gsize size);
gint jpf_connection_write(HmConnection *conn, gchar *buf, gsize size);
void jpf_connection_close(HmConnection *conn);

void jpf_connection_set_buffer_size(HmConnection *conn, gint size);
gint jpf_connection_get_buffer_size(HmConnection *conn);
void jpf_connection_set_heavy(HmConnection *conn);
gint jpf_connection_is_heavy(HmConnection *conn);
gint jpf_connection_get_timeout(HmConnection *conn);
void jpf_connection_set_timeout(HmConnection *conn, gint millisec);

gchar *jpf_connection_get_peer(HmConnection *conn);

G_END_DECLS

#endif	//__NMP_CONNECTION_H__
