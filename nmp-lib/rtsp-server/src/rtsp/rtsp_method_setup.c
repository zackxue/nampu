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

#include <math.h>
#include <string.h>
#include "rtsp.h"
#include "rc_log.h"

#include "media.h"
#include "rtp.h"
#include "mediautils.h"

#define TRY_PORTS_ALLOC				32

static __inline__ GstRTSPResult
rtsp_parse_transport(gchar *transport, GstRTSPTransport **ct)
{
	GstRTSPTransport *ts;
	gchar **transports;
	gint i;
	GstRTSPResult res = GST_RTSP_EINVAL;
	GstRTSPLowerTrans supported;

	g_assert(ct != NULL);

	if (!transport)
		return res;

	res = GST_RTSP_STS_INVALID;
	transports = g_strsplit(transport, ",", 0);

	gst_rtsp_transport_new(&ts);
	gst_rtsp_transport_init(ts);

	supported = GST_RTSP_LOWER_TRANS_UDP |
		GST_RTSP_LOWER_TRANS_UDP_MCAST | GST_RTSP_LOWER_TRANS_TCP;

	for (i = 0; transports[i]; i++)
	{
		res = gst_rtsp_transport_parse(transports[i], ts);
		if (res != GST_RTSP_OK)
		{
			/* no valid transport, search some more */
			rc_log(RC_lOG_ERR, "[SETUP] Could not parse transport %s",
				transports[i]);
			goto next;
		}

		/* we have a transport, see if it's RTP/AVP */
		if (ts->trans != GST_RTSP_TRANS_RTP || 
			ts->profile != GST_RTSP_PROFILE_AVP)
		{
			rc_log(RC_lOG_ERR, "[SETUP] Invalid transport %s",
				transports[i]);
			goto next;
		}

		if (!(ts->lower_transport & supported))
		{
			rc_log(RC_lOG_ERR, "[SETUP] Unsupported transport %s",
				transports[i]);
			goto next;
		}

		/* we have a valid transport */
		rc_log(RC_lOG_INFO, "[SETUP] Found valid transport %s", transports[i]);
		*ct = ts;
		res = GST_RTSP_OK;
		break;

next:
		gst_rtsp_transport_init(ts);
	}

	g_strfreev(transports);
	if (res != GST_RTSP_OK)
		gst_rtsp_transport_free(ts);
	return res;
}


static __inline__ GstRTSPResult
rtsp_transport_unicast(RTSP_Client *client, RTP_transport *transport,
	GstRTSPTransport *ct)
{
    gchar port_buffer[8];
    port_pair ser_ports;
    gint try_it = TRY_PORTS_ALLOC;
    gchar *remote_host;

retry:
	if (--try_it < 0)
	{
		rc_log(RC_lOG_ERR, "[SETUP] alloc ports over N times!");
		return GST_RTSP_ERROR;
	}

    if (get_port_pair(NULL, &ser_ports) != ERR_NOERROR)
    {
        return GST_RTSP_ERROR;
    }

    snprintf(port_buffer, 8, "%d", ser_ports.RTP);
    transport->rtp_sock = Sock_bind(NULL, port_buffer, NULL, UDP, NULL);
	if (!transport->rtp_sock)
	{
		put_port_pair(NULL, &ser_ports);
		goto retry;
	}

    snprintf(port_buffer, 8, "%d", ser_ports.RTCP);
    transport->rtcp_sock = Sock_bind(NULL, port_buffer, NULL, UDP, NULL);
	if (!transport->rtcp_sock)
	{
		put_port_pair(NULL, &ser_ports);
		Sock_close(transport->rtp_sock);
		goto retry;		
	}

	remote_host = ct->destination;
	if (!remote_host)
	{
		remote_host = get_remote_host(client->sock);
	}

    snprintf(port_buffer, 8, "%d", ct->client_port.min);
    Sock_connect(
    	remote_host,
    	port_buffer,
		transport->rtp_sock,
		UDP,
		NULL
	);

    snprintf(port_buffer, 8, "%d", ct->client_port.max);
    Sock_connect(
    	remote_host,
    	port_buffer,
		transport->rtcp_sock,
		UDP,
		NULL
	);

	transport->trans_mode = RTP_OVER_UDP;

    return GST_RTSP_OK;
}


static __inline__ GstRTSPResult
rtsp_transport_ror(RTSP_Client *client, RTP_transport *transport,
	GstRTSPTransport *ct)
{
	transport->passby = g_event_ref((GEvent*)client);
	transport->rtp_ch = ct->interleaved.min;
	transport->rtcp_ch = ct->interleaved.max;
	transport->trans_mode = RTP_OVER_RTSP;
	return GST_RTSP_OK;
}


static __inline__ GstRTSPResult
rtsp_transport_tcp(RTSP_Client *client, RTP_transport *transport,
	GstRTSPTransport *ct)
{
    gchar port_buffer[8];
    port_pair ser_ports;
    Sock *sock;
    gint try_it = TRY_PORTS_ALLOC;
    gchar *remote_host;

	if (ct->client_port.min <= 0)
	{
		//RTP OVER RTSP
		return rtsp_transport_ror(client, transport, ct);
	}

retry:
	if (--try_it < 0)
	{
		rc_log(RC_lOG_ERR, "[SETUP] alloc ports over N times!");
		return GST_RTSP_ERROR;
	}

    if (get_port_pair(NULL, &ser_ports) != ERR_NOERROR)
    {
        return GST_RTSP_ERROR;
    }

    snprintf(port_buffer, 8, "%d", ser_ports.RTP);
    transport->rtp_sock = Sock_bind(NULL, port_buffer, NULL, TCP, NULL);
	if (!transport->rtp_sock)
	{
		put_port_pair(NULL, &ser_ports);
		goto retry;
	}

    snprintf(port_buffer, 8, "%d", ser_ports.RTCP);
    transport->rtcp_sock = Sock_bind(NULL, port_buffer, NULL, TCP, NULL);
	if (!transport->rtcp_sock)
	{
		put_port_pair(NULL, &ser_ports);
		Sock_close(transport->rtp_sock);
		goto retry;		
	}

	remote_host = ct->destination;
	if (!remote_host)
	{
		remote_host = get_remote_host(client->sock);
	}

    snprintf(port_buffer, 8, "%d", ct->client_port.min);
    sock = Sock_connect(
    	remote_host,
    	port_buffer,
		transport->rtp_sock,
		TCP,
		NULL
	);

	if (!sock)
	{
		rc_log(
			RC_lOG_ERR,
			"[SETUP] Connect rtp port: %s:%s failed!",
			remote_host,
			port_buffer
		);

		put_port_pair(NULL, &ser_ports);
		Sock_close(transport->rtcp_sock);
		Sock_close(transport->rtp_sock);
		return GST_RTSP_ERROR;
	}

    snprintf(port_buffer, 8, "%d", ct->client_port.max);
    sock = Sock_connect(
    	remote_host,
    	port_buffer,
		transport->rtcp_sock,
		TCP,
		NULL
	);

	if (!sock)
	{
		rc_log(
			RC_lOG_ERR,
			"[SETUP] Connect rtcp port: %s:%s failed!",
			remote_host,
			port_buffer
		);

		put_port_pair(NULL, &ser_ports);
		Sock_close(transport->rtcp_sock);
		Sock_close(transport->rtp_sock);
		return GST_RTSP_ERROR;
	}

	rtp_over_tcp_prepare(transport);
	transport->trans_mode = RTP_OVER_TCP;

    return GST_RTSP_OK;
}


static __inline__ gboolean
rtsp_check_parsed_transport(RTSP_Client *client, RTP_transport *transport,
	GstRTSPTransport *ct)
{
	memset(transport, 0, sizeof(*transport));

	switch (ct->lower_transport)
	{
	case GST_RTSP_LOWER_TRANS_UDP_MCAST:
		return FALSE;	/* not supported yet */

	case GST_RTSP_LOWER_TRANS_UDP:
		return rtsp_transport_unicast(client, transport, ct) == GST_RTSP_OK;

	case GST_RTSP_LOWER_TRANS_TCP:
		return rtsp_transport_tcp(client, transport, ct) == GST_RTSP_OK;

	default:
		rc_log(RC_LOG_ERR, "[SETUP] Unsupported transport type.");
		return FALSE;
	}
}


static __inline__ void
rtsp_reset_parsed_transport(RTP_transport *transport)
{
	rtp_transport_close(transport);
}


static __inline__ Track *
rtsp_select_requested_track(RTSP_Client *client, RTSP_session *rtsp_s,
	RTSP_Ps *state)
{
    gchar *trackname = NULL, *separator, *path;
    Track *selected_track = NULL;
    gsize resource_uri_length;

    g_assert((rtsp_s->resource && rtsp_s->resource_uri) ||
		(!rtsp_s->resource && !rtsp_s->resource_uri));

    if (!rtsp_s->resource)
    {
		/* try to find "/stream=" */
        separator = strstr(state->url->abspath, SDP_TRACK_URI_SEPARATOR);
        if (!separator)
        {
        	rtsp_client_send_generic_response(
				client,
				GST_RTSP_STS_ONLY_AGGREGATE_OPERATION_ALLOWED,
				state
			);
			return NULL;
        }

        trackname = separator + strlen(SDP_TRACK_URI_SEPARATOR);

        rtsp_s->resource_uri = g_strndup(
        	state->url->abspath,
        	separator - state->url->abspath
        );

        path = g_uri_unescape_string(rtsp_s->resource_uri, "/");
        if (!(rtsp_s->resource = r_open(path, OM_SETUP)))
        {
            fnc_log(FNC_LOG_DEBUG, "Resource for %s not found\n", path);

            g_free(path);
            g_free(rtsp_s->resource_uri);
            rtsp_s->resource_uri = NULL;

        	rtsp_client_send_generic_response(
				client,
				GST_RTSP_STS_NOT_FOUND,
				state
			);

            return NULL;
        }

        g_free(path);
    }
    else
    {
		resource_uri_length = strlen(rtsp_s->resource_uri);

        if (strncmp(state->url->abspath, rtsp_s->resource_uri,
        	resource_uri_length) != 0)
        {
        	rtsp_client_send_generic_response(
				client,
				GST_RTSP_STS_AGGREGATE_OPERATION_NOT_ALLOWED,
				state
			);
            return NULL;
        }

        if (strncmp(state->url->abspath + resource_uri_length,
                     SDP_TRACK_URI_SEPARATOR,
                     strlen(SDP_TRACK_URI_SEPARATOR)) != 0)
        {
        	rtsp_client_send_generic_response(
				client,
				GST_RTSP_STS_ONLY_AGGREGATE_OPERATION_ALLOWED,
				state
			);
            return NULL;
        }

        trackname = state->url->abspath + resource_uri_length
            + strlen(SDP_TRACK_URI_SEPARATOR);
    }

    if (!(selected_track = r_find_track(rtsp_s->resource, trackname)))
    {
    	rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_NOT_FOUND,
			state
		);
    }

    return selected_track;
}


static __inline__ void
rtsp_send_setup_response(RTSP_Client *client, RTSP_session * rtsp_s,
	RTP_session *rtp_s, Track *req_track, RTSP_Ps *state)
{
	gchar *transport = NULL;

	gst_rtsp_message_init_response(state->response, GST_RTSP_STS_OK,
		gst_rtsp_status_as_text(GST_RTSP_STS_OK), state->request);

	switch (rtp_s->transport.trans_mode)
	{
	case RTP_OVER_UDP:
		transport = g_strdup_printf(
			"RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d;ssrc=%08X",
			get_remote_port(rtp_s->transport.rtp_sock),
			get_remote_port(rtp_s->transport.rtcp_sock),
			get_local_port(rtp_s->transport.rtp_sock),
			get_local_port(rtp_s->transport.rtcp_sock),
			rtp_s->ssrc
		);
		break;

	case RTP_OVER_TCP:
		transport = g_strdup_printf(
			"RTP/AVP/TCP;unicast;client_port=%d-%d;server_port=%d-%d;ssrc=%08X",
			get_remote_port(rtp_s->transport.rtp_sock),
			get_remote_port(rtp_s->transport.rtcp_sock),
			get_local_port(rtp_s->transport.rtp_sock),
			get_local_port(rtp_s->transport.rtcp_sock),
			rtp_s->ssrc
		);
		break;		

	case RTP_OVER_RTSP:
		transport = g_strdup_printf(
			"RTP/AVP/TCP;interleaved=%d-%d;ssrc=%08X",
			rtp_s->transport.rtp_ch,
			rtp_s->transport.rtcp_ch,
			rtp_s->ssrc
		);
		break;

	default:
		break;
	}

	if (transport)
	{
		gst_rtsp_message_add_header(state->response, GST_RTSP_HDR_TRANSPORT,
			transport);
		g_free(transport);
	}

	rtsp_client_send_response(client, rtsp_s, state->response);
}


void
rtsp_handle_setup_request(RTSP_Client *client, RTSP_Ps *state)
{
	GstRTSPResult res;
	gchar *transport_str, *sid;
	GstRTSPTransport *ct;
	RTP_transport transport;
	RTSP_session *rtsp_s;
	Track *req_track;
	RTP_session *rtp_s;

	res = gst_rtsp_message_get_header(state->request,
		GST_RTSP_HDR_TRANSPORT, &transport_str, 0);
	if (res != GST_RTSP_OK ||
		rtsp_parse_transport(transport_str, &ct) != GST_RTSP_OK)
	{
		rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_UNSUPPORTED_TRANSPORT,
			state
		);
		return;
	}

	if (!rtsp_check_parsed_transport(client, &transport, ct))
	{
		rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_UNSUPPORTED_TRANSPORT,
			state
		);
		gst_rtsp_transport_free(ct);
		return;
	}

	gst_rtsp_transport_free(ct);

	res = gst_rtsp_message_get_header(state->request,
		GST_RTSP_HDR_SESSION, &sid, 0);
	if (res != GST_RTSP_OK)
	{
		rtsp_s = __rtsp_client_session_new(client);
		if (G_UNLIKELY(!rtsp_s))
		{
			rtsp_client_send_generic_response(
				client,
				GST_RTSP_STS_METHOD_NOT_VALID_IN_THIS_STATE,
				state
			);
			rtsp_reset_parsed_transport(&transport);
			return;
		}
	}
	else
	{
		rtsp_s = rtsp_client_session_find(client, sid);
		if (G_UNLIKELY(!rtsp_s))
		{
			rtsp_client_send_generic_response(
				client,
				GST_RTSP_STS_SESSION_NOT_FOUND,
				state
			);
			rtsp_reset_parsed_transport(&transport);
			return;
		}
	}

	g_mutex_lock(rtsp_s->mutex);	/* Lock direction: rtsp->mutex => client->mutex */

	if (G_UNLIKELY(rtsp_s->killed))
	{
		g_mutex_unlock(rtsp_s->mutex);
		rtsp_session_unref(rtsp_s);

		rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_METHOD_NOT_VALID_IN_THIS_STATE,
			state
		);
		rtsp_reset_parsed_transport(&transport);
		return;
	}

    if (!(req_track = rtsp_select_requested_track(client, rtsp_s, state)))
    {
    	g_mutex_unlock(rtsp_s->mutex);
    	rtsp_session_unref(rtsp_s);
    	rtsp_reset_parsed_transport(&transport);
        return;
    }

	rtp_s = rtp_session_new(client, rtsp_s, &transport, state->url, req_track);

    if (rtsp_s->cur_state == RTSP_SERVER_INIT)
    {
        rtsp_s->cur_state = RTSP_SERVER_READY;
    }

	rtsp_send_setup_response(client, rtsp_s, rtp_s, req_track, state);

	g_mutex_unlock(rtsp_s->mutex);
	rtsp_session_unref(rtsp_s);
}

//:~ End
