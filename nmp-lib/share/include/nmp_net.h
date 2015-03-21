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

#include <glib.h>
#include "nmp_netio.h"

G_BEGIN_DECLS

typedef void (*HmIOFin)(gpointer priv_data, NmpNetIO *io, gint err);
typedef gint (*HmIOInit)(gpointer priv_data, NmpNetIO *io);
typedef gint (*HmIOReader)(gpointer priv_data, NmpNetIO *net_io, gpointer msg);

NmpNet *nmp_net_new(NmpPacketProto *ll_proto,
	NmpPayloadProto *hl_proto, gpointer priv_data);

NmpNet *nmp_net_new_full(guint nloops, gboolean gather,
	NmpPacketProto *ll_proto, NmpPayloadProto *hl_proto, gpointer priv_data);

void nmp_net_release(NmpNet *net);

void nmp_net_set_reader(NmpNet *net, HmIOReader reader);

gpointer nmp_net_get_private(NmpNet *net);

void nmp_net_set_funcs(NmpNet *net, HmIOInit init, HmIOFin fin);

NmpNetIO *nmp_net_create_io(NmpNet *net, HmConnection *conn, HmIOEst on_est,
	gint *err);

NmpNetIO *nmp_net_create_listen_io(NmpNet *net, HmConnection *conn,
	gint *err);

NmpNetIO *nmp_net_create_listen_io_2(NmpNet *net, struct sockaddr *sa,
	gint *err);

gint nmp_net_write_io(NmpNetIO *net_io, gpointer msg);

NmpNetIO *nmp_net_ref_io(NmpNetIO *net_io);
void nmp_net_unref_io(NmpNetIO *net_io);

void nmp_net_set_io_private(NmpNetIO *net_io, gpointer priv_data, HmDesFun des);
gpointer nmp_net_get_io_private(NmpNetIO *net_io);

gboolean nmp_net_set_io_ttd(NmpNetIO *net_io, gint milli_sec);

gint nmp_net_kill_io(NmpNet *net, NmpNetIO *net_io);

gchar *nmp_net_get_io_peer_name(NmpNetIO *net_io);
void nmp_net_set_heavy_io_load(NmpNetIO *net_io);
void nmp_net_set_io_block_size(NmpNetIO *net_io, gint size);

G_END_DECLS

#endif	//__NMP_NET_H__
