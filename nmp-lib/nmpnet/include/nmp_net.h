/*
 * nmp_net.h
 *
 * This file declares net interfaces.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_NET_H__
#define __NMP_NET_H__

#include "nmp_netio.h"


typedef struct _nmp_2proto
{
	nmp_packet_proto_t 	*ll_proto;
	nmp_payload_proto_t	*hl_proto;
	void                *init_data;		/* user data */
}nmp_2proto_t;


typedef void (*nmp_io_fin_func)(nmp_netio_t *io, int err, void *init_data);
typedef int (*nmp_io_init_func)(nmp_netio_t *io, void *init_data);
typedef void (*nmp_io_fin_user)(void *user);

#ifdef __cplusplus
extern "C" {
#endif

nmp_net_t *nmp_net_new(int loops);
void nmp_net_release(nmp_net_t *net);

void nmp_net_set_funcs(nmp_net_t *net, nmp_io_init_func init, nmp_io_fin_func fin);
void nmp_net_set_io_reader(nmp_netio_t *net_io, nmp_io_reader_func reader, void *init_data);
int nmp_net_write_io(nmp_netio_t *net_io, void *msg);

nmp_netio_t *nmp_net_create_io(nmp_net_t *net, nmp_conn_t *conn,
	nmp_2proto_t *proto, nmp_io_est_func on_est, int *err);

nmp_netio_t *nmp_net_create_listen_io(nmp_net_t *net, nmp_conn_t *conn,
	nmp_2proto_t *proto, int *err);

nmp_netio_t *nmp_net_create_listen_io_2(nmp_net_t *net, struct sockaddr *sa,
	nmp_2proto_t *proto, int *err);

nmp_netio_t *nmp_net_ref_io(nmp_netio_t *net_io);
void nmp_net_unref_io(nmp_netio_t *net_io);

void nmp_net_set_io_ttd(nmp_netio_t *net_io, int milli_sec);
int nmp_net_kill_io(nmp_net_t *net, nmp_netio_t *net_io);

void nmp_net_set_io_u(nmp_netio_t *net_io, void *u, nmp_io_fin_user *fu);
void *nmp_net_get_io_u(nmp_netio_t *net_io);

char *nmp_net_get_io_host_name(nmp_netio_t *net_io, char *ip);
char *nmp_net_get_io_peer_name(nmp_netio_t *net_io, char *ip);

//Timer
void *nmp_set_timer(int timeout, int (*)(void*), void *data);
void nmp_del_timer(void *handle);

#ifdef __cplusplus
}
#endif

#endif	//__NMP_NET_H__
