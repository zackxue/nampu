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


typedef struct _JNetIO JNetIO;
typedef struct _JNet JNet;

typedef void (*JNetIOFin)(void *priv);
typedef int (*JIOReader)(JNetIO *net_io, void *msg, void *init_data);
typedef void (*JIOEst)(JNetIO *io, void *init_data);

#ifdef __cplusplus
extern "C" {
#endif

JNetIO *j_net_io_new(JConnection *conn, JPacketProto *ll_proto,
	JPayloadProto *hl_proto, void *init_data, int *err);

JNetIO *j_net_listen_io_new(JConnection *conn,
	JPacketProto *ll_proto, JPayloadProto *hl_proto, void *init_data, int *err);

void j_net_io_attach(JNetIO *net_io, JEventLoop *loop);

JNetIO *j_net_io_ref(JNetIO *net_io);
void j_net_io_unref(JNetIO *net_io);

void j_net_io_kill(JNetIO *net_io);
void j_net_io_async_kill(JNetIO *net_io, int err);

void j_net_io_set_owner(JNetIO *net_io, void *owner);

int j_net_io_read_message(JNetIO *net_io, void *msg);
int j_net_io_write_message(JNetIO *net_io, void *msg);

int j_net_io_add_child_watch(JNetIO *net_io, void *watch);

void j_net_io_establish(JNetIO *net_io);
void j_net_io_on_establish(JNetIO *net_io);

void j_net_io_set_ttd(JNetIO *net_io, int milli_secs);
void j_net_io_set_reader(JNetIO *net_io, JIOReader reader, void *init_data);
void j_net_io_set_ester(JNetIO *net_io, JIOEst on_est);

void j_net_io_on_destroy(JNetIO *net_io, JNetIOFin fin, void *data);

void j_net_io_set_u(JNetIO *net_io, void *user, JNetIOFin user_fin);
void *j_net_io_get_u(JNetIO *net_io);

char *j_net_io_get_host(JNetIO *net_io, char *ip);
char *j_net_io_get_peer(JNetIO *net_io, char *ip);

#ifdef __cplusplus
}
#endif

#endif	//__NMP_NETIO_H__
