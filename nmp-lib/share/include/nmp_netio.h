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

typedef struct _JpfNetIO JpfNetIO;
typedef struct _JpfNet JpfNet;
typedef void (*HmDesFun)(void *private_data);
typedef void (*HmIOEst)(JpfNetIO *io, void *init_data);

JpfNetIO *jpf_net_io_new(HmConnection *conn, JpfPacketProto *ll_proto,
	JpfPayloadProto *hl_proto, gint *err);

JpfNetIO *jpf_net_listen_io_new(HmConnection *conn,
	JpfPacketProto *ll_proto, JpfPayloadProto *hl_proto, gint *err);

void jpf_net_io_attach(JpfNetIO *net_io, GMainContext *context);

JpfNetIO *jpf_net_io_ref(JpfNetIO *net_io);
void jpf_net_io_unref(JpfNetIO *net_io);

void jpf_net_io_kill(JpfNetIO *net_io);
void jpf_net_io_async_kill(JpfNetIO *net_io, gint err);

void jpf_net_io_set_owner(JpfNetIO *net_io, gpointer owner);

void jpf_net_io_set_private(JpfNetIO *net_io, gpointer priv_data, HmDesFun des);
gpointer jpf_net_io_get_private(JpfNetIO *net_io);

gint jpf_net_io_read_message(JpfNetIO *net_io, gpointer msg);
gint jpf_net_io_write_message(JpfNetIO *net_io, gpointer msg);

gint jpf_net_io_add_child_watch(JpfNetIO *net_io, gpointer watch);

void jpf_net_io_establish(JpfNetIO *net_io);
void jpf_net_io_on_establish(JpfNetIO *net_io, gpointer init_data);

gboolean jpf_net_io_set_ttd(JpfNetIO *net_io, gint milli_secs);

void jpf_net_io_set_ester(JpfNetIO *net_io, HmIOEst on_est);

gchar *jpf_net_io_get_peer(JpfNetIO *net_io);
void jpf_net_io_set_heavy_load(JpfNetIO *net_io);
void jpf_net_io_set_block_size(JpfNetIO *net_io, gint size);

G_END_DECLS

#endif	//__NMP_NETIO_H__
