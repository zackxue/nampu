#ifndef __NMP_CONNECTION_H__
#define __NMP_CONNECTION_H__

#include <netinet/in.h>
#include <stddef.h>

typedef struct _nmp_connection nmp_conn_t;

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

nmp_conn_t *nmp_conn_new(struct sockaddr *sa,
	unsigned flags, int *errp);

int nmp_conn_listen(nmp_conn_t *conn);
int nmp_conn_connect(nmp_conn_t *conn, struct sockaddr *sa);
int nmp_conn_get_fd(nmp_conn_t *conn);
int nmp_conn_set_flags(nmp_conn_t *conn, unsigned flgs);
int nmp_conn_is_blocked(nmp_conn_t *conn);
nmp_conn_t *nmp_conn_accept(nmp_conn_t *listen, int *errp);
int nmp_conn_is_ingrogress(nmp_conn_t *conn, int clear);
int nmp_conn_read(nmp_conn_t *conn, char buf[], size_t size);
int nmp_conn_write(nmp_conn_t *conn, char *buf, size_t size);
void nmp_conn_close(nmp_conn_t *conn);

int nmp_conn_get_timeout(nmp_conn_t *conn);
void nmp_conn_set_timeout(nmp_conn_t *conn, int millisec);

char *nmp_conn_get_host(nmp_conn_t *conn, char *ip);
char *nmp_conn_get_peer(nmp_conn_t *conn, char *ip);

#ifdef __cplusplus
}
#endif

#endif	//__NMP_CONNECTION_H__
