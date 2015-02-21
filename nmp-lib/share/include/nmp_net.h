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

typedef void (*HmIOFin)(gpointer priv_data, HmNetIO *io, gint err);
typedef gint (*HmIOInit)(gpointer priv_data, HmNetIO *io);
typedef gint (*HmIOReader)(gpointer priv_data, HmNetIO *net_io, gpointer msg);

HmNet *hm_net_new(HmPacketProto *ll_proto,
	HmPayloadProto *hl_proto, gpointer priv_data);

HmNet *hm_net_new_full(guint nloops, gboolean gather,
	HmPacketProto *ll_proto, HmPayloadProto *hl_proto, gpointer priv_data);

void hm_net_release(HmNet *net);

void hm_net_set_reader(HmNet *net, HmIOReader reader);

gpointer hm_net_get_private(HmNet *net);

void hm_net_set_funcs(HmNet *net, HmIOInit init, HmIOFin fin);

HmNetIO *hm_net_create_io(HmNet *net, HmConnection *conn, HmIOEst on_est,
	gint *err);

HmNetIO *hm_net_create_listen_io(HmNet *net, HmConnection *conn,
	gint *err);

HmNetIO *hm_net_create_listen_io_2(HmNet *net, struct sockaddr *sa,
	gint *err);

gint hm_net_write_io(HmNetIO *net_io, gpointer msg);

HmNetIO *hm_net_ref_io(HmNetIO *net_io);
void hm_net_unref_io(HmNetIO *net_io);

void hm_net_set_io_private(HmNetIO *net_io, gpointer priv_data, HmDesFun des);
gpointer hm_net_get_io_private(HmNetIO *net_io);

gboolean hm_net_set_io_ttd(HmNetIO *net_io, gint milli_sec);

gint hm_net_kill_io(HmNet *net, HmNetIO *net_io);

gchar *hm_net_get_io_peer_name(HmNetIO *net_io);
void hm_net_set_heavy_io_load(HmNetIO *net_io);
void hm_net_set_io_block_size(HmNetIO *net_io, gint size);

G_END_DECLS

#endif	//__HM_NET_H__
