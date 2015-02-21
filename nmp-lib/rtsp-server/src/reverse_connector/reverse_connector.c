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

#include <sys/types.h>
#include <sys/socket.h>

#include "reverse_connector.h"

#define KEEP_ALIVE_CHECK_COUNT			3

static __inline__ gint
make_rtsp_options_request(Reverse_connector *r_cntr, GstRTSPMessage **request)
{
	RTSP_Client *cli;
	gchar *url;
	GstRTSPResult res;

	cli = (RTSP_Client*)r_cntr;

	url = g_strdup_printf(
		"rtsp://%s:%s/PUID=%s/keepalive=%d/l4proto=%d",
		r_cntr->dest_ip,
		r_cntr->dest_port,
		r_cntr->puid,
		r_cntr->keep_alive_freq,
		r_cntr->l4proto
	);

	GST_RTSP_CHECK(
		gst_rtsp_message_new_request(
			request,
			GST_RTSP_OPTIONS,
			url
		),
		no_option_message
	);

	g_free(url);
	return 0;

no_option_message:
	g_free(url);
	return -1;	
}


static __inline__ gboolean
rtsp_reverse_cntr_keepalive(Reverse_connector *r_cntr)
{
	GstRTSPMessage *request;
	RTSP_Client *cli;

	cli = (RTSP_Client*)r_cntr;

	if (!make_rtsp_options_request(r_cntr, &request))
	{
		rtsp_client_send_request(cli, request);
		gst_rtsp_message_free(request);
	}

	return TRUE;
}


static __inline__ gboolean
__rtsp_reverse_cntr_rw_connect(GEvent *ev, gint revents, void *user_data)
{
	Reverse_connector *r_cntr;
    gint err = 0;
    socklen_t len = sizeof(err);
    gboolean ret;

	r_cntr = (Reverse_connector*)ev;

	if (getsockopt(g_event_fd(ev), SOL_SOCKET, SO_ERROR, &err, &len) || err)
	{
		if (!err)
		{
			err = errno;
		}
		ret = rtsp_client_default_rw_cb(ev, EV_TIMER, user_data);

		if (r_cntr->exp)
		{
			(*r_cntr->exp)(r_cntr, err, r_cntr->user_data);
		}

		return ret;
	}

	r_cntr->connected = TRUE;
	g_event_add_events_sync(ev, EV_READ);
	g_event_remove_events_sync(ev, EV_WRITE);
	rtsp_reverse_cntr_keepalive(r_cntr);

	if (r_cntr->exp)
	{
		(*r_cntr->exp)(r_cntr, E_CONN_OK, r_cntr->user_data);
	}

	return TRUE;
}


static gboolean
rtsp_reverse_cntr_default_rw(GEvent *ev, gint revents, void *user_data)
{
	Reverse_connector *r_cntr;
    gboolean ret;

	r_cntr = (Reverse_connector*)ev;

	ret = rtsp_client_default_rw_cb(ev, revents, user_data);
	if (!ret)
	{
		if (r_cntr->exp)
		{
			(*r_cntr->exp)(r_cntr, rtsp_client_errno((RTSP_Client*)ev),
				r_cntr->user_data);
		}
	}

	return ret;
}


static __inline__ gboolean
rtsp_reverse_cntr_to(GEvent *ev, gint revents, void *user_data)
{
	Reverse_connector *r_cntr;
	RTSP_Client *cli;

	r_cntr = (Reverse_connector*)ev;
	cli = (RTSP_Client*)r_cntr;

	if (--r_cntr->ttd <= 0)
	{
		return rtsp_reverse_cntr_default_rw(ev, EV_TIMER, user_data);
	}

	return rtsp_reverse_cntr_keepalive(r_cntr);
}


static gboolean
rtsp_reverse_cntr_rw(GEvent *ev, gint revents, void *user_data)
{
	Reverse_connector *r_cntr;
	RTSP_Client *cli;

	r_cntr = (Reverse_connector*)ev;
	cli = (RTSP_Client*)r_cntr;

	if (revents & (EV_READ | EV_WRITE))
	{
		if (r_cntr->connected)
		{
			r_cntr->ttd = KEEP_ALIVE_CHECK_COUNT;
			return rtsp_reverse_cntr_default_rw(ev, revents, user_data);
		}
		return __rtsp_reverse_cntr_rw_connect(ev, revents, user_data);
	}

	if (revents & EV_TIMER)
		return rtsp_reverse_cntr_to(ev, EV_TIMER, user_data);

	return rtsp_reverse_cntr_default_rw(ev, revents, user_data);
}


static __inline__ void
__rtsp_reverse_cntr_fin(GEvent *ev)
{
	Reverse_connector *r_cntr;

	r_cntr = (Reverse_connector*)ev;
	g_free(r_cntr->puid);
	g_free(r_cntr->dest_port);
	g_free(r_cntr->dest_ip);
}


static void
rtsp_reverse_cntr_fin(GEvent *ev)
{
	Reverse_connector *r_cntr;
	RTSP_Client *cli;

	r_cntr = (Reverse_connector*)ev;
	cli = (RTSP_Client*)r_cntr;

	__rtsp_reverse_cntr_fin(ev);
	rtsp_client_default_finalize(ev);
}


void
rtsp_reverse_cntr_init(Reverse_connector *r_cntr, gchar *ip,
	gchar *port, gchar *puid, gint ka_freq, gint l4proto, Connect_exp cb, gpointer u)
{
	RTSP_Client *cli;

	cli = (RTSP_Client*)r_cntr;
	cli->rw_func = rtsp_reverse_cntr_rw;
	cli->fin_func = rtsp_reverse_cntr_fin;

	r_cntr->connected = FALSE;
	r_cntr->dest_ip = g_strdup(ip);
	r_cntr->dest_port = g_strdup(port);
	r_cntr->ttd = KEEP_ALIVE_CHECK_COUNT;
	r_cntr->puid = g_strdup(puid);
	r_cntr->l4proto = l4proto;
	r_cntr->keep_alive_freq = ka_freq;
	r_cntr->exp = cb;
	r_cntr->user_data = u;
}

//:~ End
