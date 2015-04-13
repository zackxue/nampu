/*
 * nmp_netio.h
 *
 * This file declares net io interfaces.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_NETIO_H__
#define __NMP_NETIO_H__

#include "nmplib.h"
#include "nmp_conn.h"
#include "nmp_netproto.h"


typedef struct _nmp_netio nmp_netio_t;
typedef struct _nmp_net nmp_net;

typedef void (*nmp_netio_fin_func)(void *priv);
typedef int (*nmp_io_reader_func)(nmp_netio_t *net_io, void *msg, void *init_data);
typedef void (*nmp_io_est_func)(nmp_netio_t *io, void *init_data);

#ifdef __cplusplus
extern "C" {
#endif

nmp_netio_t *nmp_net_io_new(nmp_conn_t *conn, nmp_packet_proto_t *ll_proto,
	nmp_payload_proto_t *hl_proto, void *init_data, int *err);

nmp_netio_t *nmp_net_listen_io_new(nmp_conn_t *conn,
	nmp_packet_proto_t *ll_proto, nmp_payload_proto_t *hl_proto, void *init_data, int *err);

void nmp_net_io_attach(nmp_netio_t *net_io, nmp_event_loop_t *loop);

nmp_netio_t *nmp_net_io_ref(nmp_netio_t *net_io);
void nmp_net_io_unref(nmp_netio_t *net_io);

void nmp_net_io_kill(nmp_netio_t *net_io);
void nmp_net_io_async_kill(nmp_netio_t *net_io, int err);

void nmp_net_io_set_owner(nmp_netio_t *net_io, void *owner);

int nmp_net_io_read_message(nmp_netio_t *net_io, void *msg);
int nmp_net_io_write_message(nmp_netio_t *net_io, void *msg);

int nmp_net_io_add_child_watch(nmp_netio_t *net_io, void *watch);

void nmp_net_io_establish(nmp_netio_t *net_io);
void nmp_net_io_on_establish(nmp_netio_t *net_io);

void nmp_net_io_set_ttd(nmp_netio_t *net_io, int milli_secs);
void nmp_net_io_set_reader(nmp_netio_t *net_io, nmp_io_reader_func reader, void *init_data);
void nmp_net_io_set_ester(nmp_netio_t *net_io, nmp_io_est_func on_est);

void nmp_net_io_on_destroy(nmp_netio_t *net_io, nmp_netio_fin_func fin, void *data);

void nmp_net_io_set_u(nmp_netio_t *net_io, void *user, nmp_netio_fin_func user_fin);
void *nmp_net_io_get_u(nmp_netio_t *net_io);

char *nmp_net_io_get_host(nmp_netio_t *net_io, char *ip);
char *nmp_net_io_get_peer(nmp_netio_t *net_io, char *ip);

#ifdef __cplusplus
}
#endif

#endif	//__NMP_NETIO_H__
