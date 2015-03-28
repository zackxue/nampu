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

#include <glib.h>
#include "nmp_conn.h"
#include "nmp_netproto.h"

G_BEGIN_DECLS

typedef struct _NmpNetIO NmpNetIO;
typedef struct _NmpNet NmpNet;
typedef void (*NmpDesFun)(void *private_data);
typedef void (*NmpIOEst)(NmpNetIO *io, void *init_data);

NmpNetIO *nmp_net_io_new(NmpConnection *conn, NmpPacketProto *ll_proto,
	NmpPayloadProto *hl_proto, gint *err);

NmpNetIO *nmp_net_listen_io_new(NmpConnection *conn,
	NmpPacketProto *ll_proto, NmpPayloadProto *hl_proto, gint *err);

void nmp_net_io_attach(NmpNetIO *net_io, GMainContext *context);

NmpNetIO *nmp_net_io_ref(NmpNetIO *net_io);
void nmp_net_io_unref(NmpNetIO *net_io);

void nmp_net_io_kill(NmpNetIO *net_io);
void nmp_net_io_async_kill(NmpNetIO *net_io, gint err);

void nmp_net_io_set_owner(NmpNetIO *net_io, gpointer owner);

void nmp_net_io_set_private(NmpNetIO *net_io, gpointer priv_data, NmpDesFun des);
gpointer nmp_net_io_get_private(NmpNetIO *net_io);

gint nmp_net_io_read_message(NmpNetIO *net_io, gpointer msg);
gint nmp_net_io_write_message(NmpNetIO *net_io, gpointer msg);

gint nmp_net_io_add_child_watch(NmpNetIO *net_io, gpointer watch);

void nmp_net_io_establish(NmpNetIO *net_io);
void nmp_net_io_on_establish(NmpNetIO *net_io, gpointer init_data);

gboolean nmp_net_io_set_ttd(NmpNetIO *net_io, gint milli_secs);

void nmp_net_io_set_ester(NmpNetIO *net_io, NmpIOEst on_est);

gchar *nmp_net_io_get_peer(NmpNetIO *net_io);
void nmp_net_io_set_heavy_load(NmpNetIO *net_io);
void nmp_net_io_set_block_size(NmpNetIO *net_io, gint size);

G_END_DECLS

#endif	//__NMP_NETIO_H__
