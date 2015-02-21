/* *
 * This file is part of rtsp-server
 *
 * Copyright (C) 2012 fangyi <fangyi@szjxj.net>
 * See COPYING for more details
 *
 * rtsp-server is a stream transporting and controlling module using
 * RTP/RTCP/RTSP protocols, which is designed for jxj platform servers
 * such as mss, proxy-server etc.
 *
 * */

#ifndef __RTSP_SERVER_H__
#define __RTSP_SERVER_H__

#include <netembryo/wsocket.h>
#include <glib.h>
#include <rtspparse.h>

#include "reverse_connector.h"
#include "evsched.h"
#include "macros.h"
#include "stream_api.h"

typedef struct _Listen_watch Listen_watch;
struct _Listen_watch
{
	GEvent	base;

	Sock 	*sock;
	GMutex	*lock;
	GList	*conns;		/* indicate clients */
	POINTER_BACK_TO(RTSP_server)
};

typedef struct _RTSP_server RTSP_server;
struct _RTSP_server
{
	gint 	ref_count;	/* reference counting */

	gchar *host;		/* serving ip */
	gchar *port;		/* serving port */
	Listen_watch *listener;
};


RTSP_server *rtsp_server_new( void );
void rtsp_server_set_port(RTSP_server *server, gint port);
gint rtsp_server_bind_port(RTSP_server *server);
void rtsp_server_free(RTSP_server *server);

Reverse_connector *
rtsp_server_reverse_connect(RTSP_server *server, gchar *dest_ip,
	gint dest_port, gchar *puid, gint ka_freq, gint l4proto,
	Connect_exp cb, gpointer u);

void rtsp_server_release_reverse_connector(RTSP_server *server,
	Reverse_connector *r_cntr);

RTSP_server *rtsp_server_ref(RTSP_server *server);
void rtsp_server_unref(RTSP_server *server);

#endif	/*__RTSP_SERVER_H__ */
