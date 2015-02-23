#ifndef __NMP_CONNECTION_H__
#define __NMP_CONNECTION_H__

#include <netinet/in.h>
#include <stddef.h>

typedef struct _JConnection JConnection;

enum _JConnFlags
{
	CF_TYPE_TCP = 0x01,
	CF_TYPE_UDP = 0x02,
	CF_FLGS_NONBLOCK = 0x100,
	CF_FLGS_CONN_INPROGRESS =0x200
};

#ifdef __cplusplus
extern "C" {
#endif

int j_resolve_host(struct sockaddr_in *sin, char *host, int port);

JConnection *j_connection_new(struct sockaddr *sa,
	unsigned flags, int *errp);

int j_connection_listen(JConnection *conn);
int j_connection_connect(JConnection *conn, struct sockaddr *sa);
int j_connection_get_fd(JConnection *conn);
int j_connection_set_flags(JConnection *conn, unsigned flgs);
int j_connection_is_blocked(JConnection *conn);
JConnection *j_connection_accept(JConnection *listen, int *errp);
int j_connection_is_ingrogress(JConnection *conn, int clear);
int j_connection_read(JConnection *conn, char buf[], size_t size);
int j_connection_write(JConnection *conn, char *buf, size_t size);
void j_connection_close(JConnection *conn);

int j_connection_get_timeout(JConnection *conn);
void j_connection_set_timeout(JConnection *conn, int millisec);

char *j_connection_get_host(JConnection *conn, char *ip);
char *j_connection_get_peer(JConnection *conn, char *ip);

#ifdef __cplusplus
}
#endif

#endif	//__NMP_CONNECTION_H__
