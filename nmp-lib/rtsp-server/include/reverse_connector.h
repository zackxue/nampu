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

#ifndef __REVERSE_CONNECTOR_H__
#define __REVERSE_CONNECTOR_H__

#include "rtsp.h"

#define E_CONN_OK		255

typedef struct _Reverse_connector Reverse_connector;
typedef void (*Connect_exp)(Reverse_connector *cntr, gint err, gpointer user_data);

struct _Reverse_connector
{
	RTSP_Client base;
	gboolean connected;
	gchar *dest_ip;
	gchar *dest_port;
	gint ttd;		/* time to die */
	gchar *puid;	/* puid? */
	gint l4proto;
	gint keep_alive_freq;
	Connect_exp exp;
	gpointer user_data; 
};

void rtsp_reverse_cntr_init(Reverse_connector *r_cntr, gchar *ip,
	gchar *port, gchar *puid, gint ka_freq, gint l4proto, Connect_exp cb, gpointer u);

#endif	/* __REVERSE_CONNECTOR_H__ */
