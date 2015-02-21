/*
 * nmp_net.h
 *
 * This file declares net interfaces.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __HM_NET_H__
#define __HM_NET_H__

#include <glib.h>
#include "nmp_netio.h"

G_BEGIN_DECLS

typedef void (*HmIOFin)(gpointer priv_data, JpfNetIO *io, gint err);
typedef gint (*HmIOInit)(gpointer priv_data, JpfNetIO *io);
typedef gint (*HmIOReader)(gpointer priv_data, JpfNetIO *net_io, gpointer msg);

JpfNet *jpf_net_new(JpfPacketProto *ll_proto,
	JpfPayloadProto *hl_proto, gpointer priv_data);

JpfNet *jpf_net_new_full(guint nloops, gboolean gather,
	JpfPacketProto *ll_proto, JpfPayloadProto *hl_proto, gpointer priv_data);

void jpf_net_release(JpfNet *net);

void jpf_net_set_reader(JpfNet *net, HmIOReader reader);

gpointer jpf_net_get_private(JpfNet *net);

void jpf_net_set_funcs(JpfNet *net, HmIOInit init, HmIOFin fin);

JpfNetIO *jpf_net_create_io(JpfNet *net, HmConnection *conn, HmIOEst on_est,
	gint *err);

JpfNetIO *jpf_net_create_listen_io(JpfNet *net, HmConnection *conn,
	gint *err);

JpfNetIO *jpf_net_create_listen_io_2(JpfNet *net, struct sockaddr *sa,
	gint *err);

gint jpf_net_write_io(JpfNetIO *net_io, gpointer msg);

JpfNetIO *jpf_net_ref_io(JpfNetIO *net_io);
void jpf_net_unref_io(JpfNetIO *net_io);

void jpf_net_set_io_private(JpfNetIO *net_io, gpointer priv_data, HmDesFun des);
gpointer jpf_net_get_io_private(JpfNetIO *net_io);

gboolean jpf_net_set_io_ttd(JpfNetIO *net_io, gint milli_sec);

gint jpf_net_kill_io(JpfNet *net, JpfNetIO *net_io);

gchar *jpf_net_get_io_peer_name(JpfNetIO *net_io);
void jpf_net_set_heavy_io_load(JpfNetIO *net_io);
void jpf_net_set_io_block_size(JpfNetIO *net_io, gint size);

G_END_DECLS

#endif	//__HM_NET_H__
