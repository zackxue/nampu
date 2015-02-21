/*
 * nmp_netio.h
 *
 * This file declares net io interfaces.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __HM_NETIO_H__
#define __HM_NETIO_H__

#include <glib.h>
#include "nmp_conn.h"
#include "nmp_netproto.h"

G_BEGIN_DECLS

typedef struct _HmNetIO HmNetIO;
typedef struct _HmNet HmNet;
typedef void (*HmDesFun)(void *private_data);
typedef void (*HmIOEst)(HmNetIO *io, void *init_data);

HmNetIO *hm_net_io_new(HmConnection *conn, HmPacketProto *ll_proto,
	HmPayloadProto *hl_proto, gint *err);

HmNetIO *hm_net_listen_io_new(HmConnection *conn,
	HmPacketProto *ll_proto, HmPayloadProto *hl_proto, gint *err);

void hm_net_io_attach(HmNetIO *net_io, GMainContext *context);

HmNetIO *hm_net_io_ref(HmNetIO *net_io);
void hm_net_io_unref(HmNetIO *net_io);

void hm_net_io_kill(HmNetIO *net_io);
void hm_net_io_async_kill(HmNetIO *net_io, gint err);

void hm_net_io_set_owner(HmNetIO *net_io, gpointer owner);

void hm_net_io_set_private(HmNetIO *net_io, gpointer priv_data, HmDesFun des);
gpointer hm_net_io_get_private(HmNetIO *net_io);

gint hm_net_io_read_message(HmNetIO *net_io, gpointer msg);
gint hm_net_io_write_message(HmNetIO *net_io, gpointer msg);

gint hm_net_io_add_child_watch(HmNetIO *net_io, gpointer watch);

void hm_net_io_establish(HmNetIO *net_io);
void hm_net_io_on_establish(HmNetIO *net_io, gpointer init_data);

gboolean hm_net_io_set_ttd(HmNetIO *net_io, gint milli_secs);

void hm_net_io_set_ester(HmNetIO *net_io, HmIOEst on_est);

gchar *hm_net_io_get_peer(HmNetIO *net_io);
void hm_net_io_set_heavy_load(HmNetIO *net_io);
void hm_net_io_set_block_size(HmNetIO *net_io, gint size);

G_END_DECLS

#endif	//__HM_NETIO_H__
