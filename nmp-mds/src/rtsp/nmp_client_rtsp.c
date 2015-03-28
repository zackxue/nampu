#include <string.h>
#include "nmp_media_server.h"
#include "nmp_rtsp.h"
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_path_regex.h"

#define INTEGER_LENGTH		16

#define DEV_REGEX_1	"dev=.*@[A-Z]{3}-[A-Z]{3}-\\d{8}"
#define DEV_REGEX_2 "dev=.*@MSS-\\d{4}"


typedef enum
{
	USER_AGENT_UNKNOWN,
	USER_AGENT_JXJ_CLI,
	USER_AGENT_VLC,
	USER_AGENT_MPLAYER,
}USER_AGENT;


/*
 * "/dev=DM-12345@JXJ-DVS-12345678&key=8566142[%mds=192.168.1.13&port=9901&key=7546211]/media=0/channel=01&level=1/stream=0",
*/


static __inline__ gint
nmp_rtsp_parse_device(const gchar *uri, NmpMediaUri *media_uri)
{
	gint err = -E_RTSPURL;
	gchar *dev, *p;

	dev = nmp_rtsp_param_string(uri, DEV_REGEX_1);
	if (!dev)
	{
		dev = nmp_rtsp_param_string(uri, DEV_REGEX_2);
		if (!dev)
			return err;
	}

	p = strstr(dev, "@");
	if (!p)
	{
		g_free(dev);
		return err;
	}

	strncpy(media_uri->device, ++p, MAX_MEDIA_LOCTION_LEN - 1);
	g_free(dev);
	return 0;
}


static __inline__ gchar *
nmp_rtsp_parse_media_mrl(const gchar *uri, NmpMediaUri *media_uri)
{
	gchar *m, *p;

	if (!uri)
		return NULL;

	m = strstr(uri, "media=");
	if (!m)
		return NULL;

	p = strstr(m, "/");
	if (!p)
		p = strstr(m, "\\");

	if (!p)
		return NULL;

	strncpy(media_uri->mrl, ++p, MAX_MEDIA_MRL_LEN - 1);
	return media_uri->mrl;
}


static __inline__ gint
nmp_rtsp_parse_media_type(const gchar *uri, NmpMediaUri *media_uri,
	void *tags)
{
	gint err = -E_RTSPURL;
	gchar *media;

	media = nmp_rtsp_param_string(uri, "media=\\d+");
	if (!media)
		return err;

	sscanf(media, "media=%d", &media_uri->type);
	g_free(media);

	if (media_uri->type == MS_LIVE)
	{
		media_uri->sequence = 0;
	}
	else
	{
		media_uri->sequence = (gint)tags;
		nmp_rtsp_parse_media_mrl(uri, media_uri);
	}

	return 0;
}


static __inline__ gint
nmp_rtsp_parse_channel(const gchar *uri, NmpMediaUri *media_uri)
{
	gint err = -E_RTSPURL;
	gchar *ch, *le;

	if (media_uri->type != 0)	/* done */
	{
		media_uri->channel = -1;
		media_uri->rate_level = -1;
		return 0;
	}

	ch = nmp_rtsp_param_string(uri, "channel=\\d+");
	if (!ch)
		return err;

	sscanf(ch, "channel=%d", &media_uri->channel);
	g_free(ch);

	if (media_uri->channel < 0 || media_uri->channel > 99)
		return err;

	le = nmp_rtsp_param_string(uri, "level=\\d+");
	if (!le)
		return err;

	sscanf(le, "level=%d", &media_uri->rate_level);
	g_free(le);

	if (media_uri->rate_level < 0 || media_uri->rate_level > 9)
		return err;

	return 0;
}


static __inline__ gint
nmp_rtsp_parse_track(const gchar *uri, NmpMediaTrack *track)
{
	const gchar *s_start, *s_end;

	s_end = uri + strlen(uri);
	s_start = s_end;

	while (s_start >= uri)
	{
		if (*s_start != '/' && *s_start != '\\')
			--s_start;
		else
			break;
	}

	if (s_start < uri || !*s_start)
		return -E_RTSPURL;

	++s_start;

	memset(track, 0, sizeof(*track));
	strncpy(track->name, s_start, MAX_TRACK_NAME_LEN - 1);

	return 0;
}


static __inline__ gint
nmp_rtsp_parse_uri(const gchar *uri, NmpMediaUri *media_uri,
	NmpMediaTrack *track, void *tags)
{
	g_assert(media_uri != NULL);

	if (G_UNLIKELY(!uri))
		return -E_INVAL;

	memset(media_uri, 0, sizeof(*media_uri));

	if (nmp_rtsp_parse_device(uri, media_uri))
	{
		nmp_print(
			"Parse 'dev=' failed in url:'%s'.",
			uri
		);		
		return -E_RTSPURL;
	}

	if (nmp_rtsp_parse_media_type(uri, media_uri, tags))
	{
		nmp_print(
			"Parse 'media=' failed in url:'%s'.",
			uri
		);
		return -E_RTSPURL;
	}

	if (nmp_rtsp_parse_channel(uri, media_uri))
	{
		nmp_print(
			"Parse 'channel&level' failed in url:'%s'.",
			uri
		);

		return -E_RTSPURL;
	}

	if (track)
	{
		if (nmp_rtsp_parse_track(uri, track))
		{
			nmp_print(
				"Parse 'track' failed in url:'%s'.",
				uri
			);

			return -E_RTSPURL;
		}
	}

	return 0;
}


static __inline__ GstRTSPMessage *
nmp_rtsp_message_dup_request(GstRTSPMessage *request)
{
	GstRTSPMessage *msg;
	GstRTSPMethod method;
	const gchar *uri;
	gchar *value;
	GstRTSPVersion version;
	GstRTSPHeaderField field;
	guint8 *body_data;
	guint body_size;

	if (gst_rtsp_message_get_type(request) != 
		GST_RTSP_MESSAGE_REQUEST)
		return NULL;

	if (gst_rtsp_message_parse_request(request,
		&method, &uri, &version) != GST_RTSP_OK)
		return NULL;

	if (gst_rtsp_message_new_request(&msg,
		method, uri) != GST_RTSP_OK)
		return NULL;

	for (field = GST_RTSP_HDR_INVALID + 1;
		field < GST_RTSP_HDR_LAST;
		++field)
	{
		if (gst_rtsp_message_get_header(request,
			field, &value, 0) == GST_RTSP_OK)
		{
			gst_rtsp_message_add_header(msg, field, value);
		}
	}

	if (gst_rtsp_message_get_body(request, 
		&body_data, &body_size) == GST_RTSP_OK)
	{
		gst_rtsp_message_set_body(msg, body_data, body_size);
	}

	return msg;
}


static __inline__ void
nmp_rtsp_client_send_generic_response(NmpRtspClient *client,
	GstRTSPStatusCode code, NmpRtspState *state)
{
	gst_rtsp_message_init_response(state->response, code,
		gst_rtsp_status_as_text(code), state->request);

	nmp_rtsp_client_send_response(client, NULL, state->response);
}


static GstRTSPResult
nmp_rtsp_parse_transport(gchar *transport, GstRTSPTransport **ct)
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
			nmp_warning("could not parse transport %s", transports[i]);
			goto next;
		}

		/* we have a transport, see if it's RTP/AVP */
		if (ts->trans != GST_RTSP_TRANS_RTP || 
			ts->profile != GST_RTSP_PROFILE_AVP)
		{
			nmp_warning("invalid transport %s", transports[i]);
			goto next;
		}

		if (!(ts->lower_transport & supported))
		{
			nmp_warning("unsupported transport %s", transports[i]);
			goto next;
		}

		/* we have a valid transport */
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


static __inline__ USER_AGENT
nmp_rtsp_parse_user_agent(GstRTSPMessage *request)
{
	GstRTSPResult res;
	gchar *user_agent;

	res = gst_rtsp_message_get_header(request, GST_RTSP_HDR_USER_AGENT,
      &user_agent, 0);

	if (res != GST_RTSP_OK)
		return USER_AGENT_UNKNOWN;

	BUG_ON(!user_agent);

	if (strstr(user_agent, "JXJClient"))
		return USER_AGENT_JXJ_CLI;

	if (strstr(user_agent, "LibVLC"))
		return USER_AGENT_VLC;

	if (strstr(user_agent, "MPlayer"))
		return USER_AGENT_MPLAYER;

	return USER_AGENT_UNKNOWN;
}


static __inline__ void
nmp_rtsp_init_desc_response(GstRTSPMessage *response, GstRTSPUrl *url, 
	GstRTSPStatusCode result, GstRTSPMessage *request)
{
	GstRTSPResult res;
	gchar *str, *content_base;
	gchar *cseq;
	gint str_len;
	g_assert(response != NULL && url != NULL);

	gst_rtsp_message_init_response(response, result,
		gst_rtsp_status_as_text(result), response);
	gst_rtsp_message_add_header(response, GST_RTSP_HDR_CONTENT_TYPE,
		"application/sdp");

	res = gst_rtsp_message_get_header(request, GST_RTSP_HDR_CSEQ,
		&cseq, 0);
	if (res == GST_RTSP_OK)
	{
		gst_rtsp_message_add_header(response, GST_RTSP_HDR_CSEQ, cseq);
	}

	str = gst_rtsp_url_get_request_uri(url);
	str_len = strlen(str);

	/* check for trailing '/' and append one */
	if (str[str_len - 1] != '/')
	{
		content_base = g_malloc(str_len + 2);
		memcpy(content_base, str, str_len);
		content_base[str_len] = '/';
		content_base[str_len + 1] = '\0';
		g_free(str);
	}
	else
		content_base = str;

	gst_rtsp_message_add_header(response, GST_RTSP_HDR_CONTENT_BASE,
		content_base);
	g_free(content_base);	
}


static void 
nmp_rtsp_do_pending_desc(gpointer __media, gpointer __client, gpointer __msg)
{
	NmpRtspMedia *media = (NmpRtspMedia*)__media;
	NmpRtspClient *client = (NmpRtspClient*)__client;
	GstRTSPMessage *msg = (GstRTSPMessage*)__msg;
	GstRTSPMethod method;
	GstRTSPVersion version;
	const gchar *url_str = NULL;
	GstRTSPUrl *url = NULL;
	GstRTSPMessage response = { 0 };
	NmpRtspState state = { NULL };

	guint8 *sdp_info = NULL;
	guint sdp_size;

	state.request = msg;
	state.response = &response;

	if (nmp_rtsp_media_has_teardown(media))
	{
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_NOT_FOUND,
			&state
		);
	    goto do_desc_pending_out;
	}

	gst_rtsp_message_parse_request(msg, &method, &url_str, &version);
	if (version != GST_RTSP_VERSION_1_0)
	{
		nmp_rtsp_client_send_generic_response(
			client, 
			GST_RTSP_STS_RTSP_VERSION_NOT_SUPPORTED, 
			&state
		);
	    goto do_desc_pending_out;
	}

	if (gst_rtsp_url_parse(url_str, &url) != GST_RTSP_OK)
	{
    	nmp_rtsp_client_send_generic_response(
    		client,
    		GST_RTSP_STS_BAD_REQUEST,
    		&state
    	);
		goto do_desc_pending_out;
	}

	nmp_rtsp_init_desc_response(
		&response,
		url,
		nmp_rtsp_media_destroyed(media) ? GST_RTSP_STS_NOT_FOUND :
			GST_RTSP_STS_OK,
		msg
	);

	nmp_rtsp_media_get_sdp_info(media, &sdp_info, &sdp_size);
	if (sdp_info)
	{
		gst_rtsp_message_take_body(&response, sdp_info, sdp_size);
	}

	nmp_rtsp_client_send_response(client, NULL, &response);
	gst_rtsp_url_free(url);
	
do_desc_pending_out:
	nmp_rtsp_client_unref(client);
	nmp_rtsp_media_unref(media);
	gst_rtsp_message_free(msg);	
}


static void 
nmp_rtsp_do_pending_setup(gpointer __media, gpointer __client, gpointer __msg)
{
	NmpRtspMedia *media = (NmpRtspMedia*)__media;
	NmpRtspClient *client = (NmpRtspClient*)__client;
	GstRTSPMessage *msg = (GstRTSPMessage*)__msg;
	GstRTSPMethod method;
	GstRTSPVersion version;
	const gchar *url_str = NULL;
	GstRTSPUrl *url = NULL;
	GstRTSPResult res;
	gchar *transport, *trans_string, *sid;
	GstRTSPTransport *ct;
	GstRTSPMessage response = { 0 };
	NmpRtspState state = { NULL };
	NmpRtspSession *session;
	NmpMediaUri media_uri;
	NmpMediaTrack track;

	state.request = msg;
	state.response = &response;

	if (nmp_rtsp_media_has_teardown(media))
	{
		nmp_rtsp_client_send_generic_response(
			client, 
			GST_RTSP_STS_NOT_FOUND, 
			&state
		);
	    goto do_setup_pending_out;		
	}

	gst_rtsp_message_parse_request(msg, &method, &url_str, &version);
	if (version != GST_RTSP_VERSION_1_0)
	{
		nmp_rtsp_client_send_generic_response(
			client, 
			GST_RTSP_STS_RTSP_VERSION_NOT_SUPPORTED,
			&state
		);
	    goto do_setup_pending_out;
	}

	if (gst_rtsp_url_parse(url_str, &url) != GST_RTSP_OK)
	{
    	nmp_rtsp_client_send_generic_response(
    		client,
    		GST_RTSP_STS_BAD_REQUEST,
    		&state
    	);
		goto do_setup_pending_out;
	}

	if (nmp_rtsp_parse_uri(url->abspath, &media_uri,
		&track, client))
	{
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_BAD_REQUEST,
			&state
		);
		goto do_setup_pending_out;
	}

	if (nmp_rtsp_media_destroyed(media))
	{
    	nmp_rtsp_client_send_generic_response(
    		client,
    		GST_RTSP_STS_CONTINUE,
    		&state
    	);
    	gst_rtsp_url_free(url);	
		goto do_setup_pending_out;
	}

	res = gst_rtsp_message_get_header(msg, GST_RTSP_HDR_TRANSPORT,
      &transport, 0);
	if (res != GST_RTSP_OK ||
		nmp_rtsp_parse_transport(transport, &ct) != GST_RTSP_OK)
	{
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_UNSUPPORTED_TRANSPORT,
			&state
		);
		gst_rtsp_url_free(url);	
		goto do_setup_pending_out;
	}

	res = gst_rtsp_message_get_header(msg, GST_RTSP_HDR_SESSION, &sid, 0);
	if (res != GST_RTSP_OK)
	{
		session = nmp_rtsp_create_session(client, media, ct);
	 	if (G_UNLIKELY(!session))
	 	{
			nmp_rtsp_client_send_generic_response(
				client,
				GST_RTSP_STS_INTERNAL_SERVER_ERROR,
				&state
			);
			gst_rtsp_transport_free(ct);
			gst_rtsp_url_free(url);
			goto do_setup_pending_out;
	 	}
	}
	else
	{
		session = nmp_rtsp_client_find_session(client, sid);
		if (G_UNLIKELY(!session))
		{
			nmp_rtsp_client_send_generic_response(
				client,
				GST_RTSP_STS_SESSION_NOT_FOUND,
				&state
			);
			gst_rtsp_transport_free(ct);
			gst_rtsp_url_free(url);	
			goto do_setup_pending_out;	
		}
	}

	if (nmp_media_set_sinker_transport(media, session->sinker, 
		&track, client->client_ip, ct, FALSE, client))
	{
		nmp_rtsp_client_delete_session(client, session->sid);
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_UNSUPPORTED_TRANSPORT,
			&state
		);
		gst_rtsp_transport_free(ct);
		gst_rtsp_url_free(url);	
		goto do_setup_pending_out;	
	}

	trans_string = gst_rtsp_transport_as_text(ct);
	gst_rtsp_message_init_response(
		&response, GST_RTSP_STS_OK,
		gst_rtsp_status_as_text(GST_RTSP_STS_OK),
		msg
	);
	gst_rtsp_message_add_header(&response, GST_RTSP_HDR_TRANSPORT,
		trans_string);
	nmp_rtsp_client_send_response(client, session, &response);

	g_free(trans_string);
	gst_rtsp_transport_free(ct);
	gst_rtsp_url_free(url);

do_setup_pending_out:
	nmp_rtsp_client_unref(client);
	nmp_rtsp_media_unref(media);
	gst_rtsp_message_free(msg);
}


static __inline__ gint
nmp_rtsp_media_pending_request(NmpRtspMedia *media, NmpRtspClient *client,
	GstRTSPMessage *request, NmpMediaState state)
{
	NmpPQNFunc cfun = NULL;	/* callback function */
	GstRTSPMessage *msg;
	gint ret;
	gchar *r_msg;

	switch (state)
	{
	case NMP_SESSION_DESCRIBE:
		cfun = nmp_rtsp_do_pending_desc;
		r_msg = "DESCRIBE";
		break;

	case NMP_SESSION_SETUP:
		cfun = nmp_rtsp_do_pending_setup;
		r_msg = "SETUP";
		break;

	default:
		return -E_NOTSUPPORT;
	}

	msg = nmp_rtsp_message_dup_request(request);
	if (msg)
	{
		nmp_rtsp_media_ref(media);
		nmp_rtsp_client_ref(client);

		ret = nmp_rtsp_media_pending_request_state(
			media, state, media, client, msg, cfun);
		if (ret)
		{
			nmp_rtsp_client_unref(client);
			nmp_rtsp_media_unref(media);
			gst_rtsp_message_free(msg);
		}
		else
		{
			nmp_print(
				"Client '%p' pending '%s' request.",
				client,
				r_msg
			);			
		}

		return ret;
	}

	nmp_warning(
		"Dup rtsp '%s' msg failed for client '%p'.",
		r_msg, client
	);

	return -E_NOMEM;	/* OOM? */
}


gint
nmp_rtsp_play_session(NmpRtspClient *client, gchar *session)
{
	g_assert(client != NULL && session != NULL);

	return nmp_rtsp_client_play_session(client, session);
}


static gint
nmp_rtsp_teardown_session(NmpRtspClient *client, gchar *session)
{
	g_assert(client != NULL && session != NULL);

	return nmp_rtsp_client_delete_session(client, session);
}


static GstRTSPStatusCode
nmp_rtsp_handle_options_request(NmpRtspClient *client, 
	NmpRtspState *state)
{
	GstRTSPMethod options;
	gchar *str;

	options = GST_RTSP_DESCRIBE |
		GST_RTSP_OPTIONS |
		GST_RTSP_PAUSE |
		GST_RTSP_PLAY |
		GST_RTSP_SETUP |
		GST_RTSP_GET_PARAMETER | GST_RTSP_SET_PARAMETER | GST_RTSP_TEARDOWN;

	str = gst_rtsp_options_as_text(options);

	gst_rtsp_message_init_response(
		state->response, 
		GST_RTSP_STS_OK,
		gst_rtsp_status_as_text(GST_RTSP_STS_OK),
		state->request
	);

	gst_rtsp_message_add_header(state->response, GST_RTSP_HDR_PUBLIC, str);
	g_free(str);

	nmp_rtsp_client_send_response(client, NULL, state->response);
	return GST_RTSP_STS_OK;
}


static GstRTSPStatusCode
nmp_rtsp_handle_describe_request(NmpRtspClient *client, NmpRtspState *state)
{
	NmpMediaUri media_uri;
	NmpMediaServer *server;
	NmpMediaDevice *device;
	NmpRtspMedia *media;
	guint8 *sdp_info;
	gint rc, have_a_try = 1;
	guint sdp_size;
	GstRTSPStatusCode res = GST_RTSP_STS_OK; 

	if (nmp_rtsp_parse_uri(state->url->abspath, &media_uri,
		NULL, client))
	{
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_BAD_REQUEST,
			state
		);
		return GST_RTSP_STS_BAD_REQUEST;
	}

	server = nmp_media_server_get();

	device = nmp_rtsp_device_mng_find_and_get_dev(
		server->device_mng, media_uri.device);
	if (G_UNLIKELY(!device))
	{
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_NOT_FOUND, state
		);

		nmp_print(
			"Device '%s' is not found, requested by client '%p'",
			media_uri.device,
			client
		);

		return GST_RTSP_STS_NOT_FOUND;
	}

	media = nmp_rtsp_device_find_media(device, &media_uri);

	if (G_UNLIKELY(!media))
	{
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_INTERNAL_SERVER_ERROR,
			state
		);
		nmp_rtsp_device_unref(device);

		nmp_warning(
			"Create media object failed, fixme!"
		);

		return GST_RTSP_STS_INTERNAL_SERVER_ERROR;
	}

try_it_again:
	nmp_rtsp_media_get_sdp_info(media, &sdp_info, &sdp_size);
	if (sdp_info)
	{
		nmp_rtsp_init_desc_response(
			state->response, state->url, GST_RTSP_STS_OK, state->request
		);

		gst_rtsp_message_take_body(state->response, sdp_info, sdp_size);
		nmp_rtsp_client_send_response(client, NULL, state->response);
	}
	else
	{
		rc = nmp_rtsp_media_pending_request(
			media, client, state->request, NMP_SESSION_DESCRIBE);
		if (rc)
		{
			if (rc == -E_NOPENDING && have_a_try)
			{
				have_a_try = 0;
				goto try_it_again;
			}

			if (rc ==  -E_MKILLED)
			{
				nmp_rtsp_client_send_generic_response(
					client,
					GST_RTSP_STS_CONTINUE,
					state
				);
				res = GST_RTSP_STS_CONTINUE;				
			}
			else
			{
				nmp_rtsp_client_send_generic_response(
					client,
					GST_RTSP_STS_INTERNAL_SERVER_ERROR,
					state
				);
				res = GST_RTSP_STS_INTERNAL_SERVER_ERROR;
			}
		}
	}

	nmp_rtsp_media_unref(media);
	nmp_rtsp_device_unref(device);

	return res;
}


static GstRTSPStatusCode
nmp_rtsp_handle_setup_request(NmpRtspClient *client, NmpRtspState *state)
{
	NmpMediaUri media_uri;
	GstRTSPResult res;
	gchar *transport, *trans_string, *sid;
	GstRTSPTransport *ct;
	NmpMediaDevice *device;
	NmpRtspMedia *media;
	NmpRtspSession *session;
	gint rc;
	GstRTSPStatusCode rtsp_result;
	NmpMediaTrack track;

	if (nmp_rtsp_parse_uri(state->url->abspath, &media_uri,
		&track, client))
	{
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_BAD_REQUEST,
			state
		);

		return GST_RTSP_STS_BAD_REQUEST;
	}

	res = gst_rtsp_message_get_header(
		state->request, GST_RTSP_HDR_TRANSPORT, &transport, 0
	);
	if (res != GST_RTSP_OK)
	{
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_UNSUPPORTED_TRANSPORT,
			state
		);

		return GST_RTSP_STS_UNSUPPORTED_TRANSPORT;
	}

	res = nmp_rtsp_parse_transport(transport, &ct);
	if (res != GST_RTSP_OK)
	{
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_UNSUPPORTED_TRANSPORT,
			state
		);

		return GST_RTSP_STS_UNSUPPORTED_TRANSPORT;
	}

	device = nmp_rtsp_device_mng_find_and_get_dev(
		nmp_media_server_get()->device_mng,
		media_uri.device
	);
	if (G_UNLIKELY(!device))
	{
		gst_rtsp_transport_free(ct);
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_NOT_FOUND,
			state
		);

		return GST_RTSP_STS_NOT_FOUND;
	}

	media = nmp_rtsp_device_find_media(device, &media_uri);
	if (G_UNLIKELY(!media))
	{
		nmp_rtsp_device_unref(device);
		gst_rtsp_transport_free(ct);
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_INTERNAL_SERVER_ERROR,
			state
		);

		return GST_RTSP_STS_INTERNAL_SERVER_ERROR;
	}

	rtsp_result = GST_RTSP_STS_OK;

	rc = nmp_rtsp_media_pending_request(
		media, client, state->request, NMP_SESSION_SETUP
	);
	if (!rc)
	{
		//挂入等待队列
		goto do_setup_finish;
	}
	else
	{
		if (rc == -E_MKILLED)
		{
			nmp_rtsp_client_send_generic_response(
				client,
				GST_RTSP_STS_CONTINUE,
				state
			);
			rtsp_result = GST_RTSP_STS_CONTINUE;
			goto do_setup_finish;
		}

		if (rc != -E_NOPENDING)
		{
			nmp_rtsp_client_send_generic_response(
				client,
				GST_RTSP_STS_INTERNAL_SERVER_ERROR,
				state
			);
			rtsp_result = GST_RTSP_STS_INTERNAL_SERVER_ERROR;
			goto do_setup_finish;
		}
	}

	res = gst_rtsp_message_get_header(
		state->request, GST_RTSP_HDR_SESSION, &sid, 0
	);
	if (res != GST_RTSP_OK)
	{
		session = nmp_rtsp_create_session(client, media, ct);
	 	if (G_UNLIKELY(!session))
	 	{
			nmp_rtsp_client_send_generic_response(
				client,
				GST_RTSP_STS_INTERNAL_SERVER_ERROR,
				state
			);
			rtsp_result = GST_RTSP_STS_INTERNAL_SERVER_ERROR;
	 		goto do_setup_finish;
	 	}
	}
	else
	{
		session = nmp_rtsp_client_find_session(client, sid);
		if (G_UNLIKELY(!session))
		{
			nmp_rtsp_client_send_generic_response(
				client,
				GST_RTSP_STS_SESSION_NOT_FOUND,
				state
			);
			rtsp_result = GST_RTSP_STS_SESSION_NOT_FOUND;
			goto do_setup_finish;
		}
	}

	if (nmp_media_set_sinker_transport(media, session->sinker, 
		&track, client->client_ip, ct, FALSE, client))
	{
		nmp_rtsp_client_delete_session(client, session->sid);
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_CONTINUE,
			state
		);
		rtsp_result = GST_RTSP_STS_CONTINUE;
		goto do_setup_finish;
	}

	trans_string = gst_rtsp_transport_as_text(ct);
	gst_rtsp_message_init_response(
		state->response,
		GST_RTSP_STS_OK,
		gst_rtsp_status_as_text(GST_RTSP_STS_OK),
		state->request
	);
	gst_rtsp_message_add_header(
		state->response,
		GST_RTSP_HDR_TRANSPORT,
		trans_string
	);
	nmp_rtsp_client_send_response(client, session, state->response);
	g_free(trans_string);

do_setup_finish:
	nmp_rtsp_media_unref(media);
	nmp_rtsp_device_unref(device);
	gst_rtsp_transport_free(ct);
	return rtsp_result;
}


static GstRTSPStatusCode
nmp_rtsp_handle_play_request(NmpRtspClient *client,  NmpRtspState *state)
{
	gchar *session;
	GstRTSPResult res;

	res = gst_rtsp_message_get_header(state->request, 
		GST_RTSP_HDR_SESSION, &session, 0);
	if (res != GST_RTSP_OK)
	{
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_BAD_REQUEST,
			state
		);
		return GST_RTSP_STS_BAD_REQUEST;
	}

	if (nmp_rtsp_client_play_failed(client, session))
	{
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_SESSION_NOT_FOUND,
			state
		);
		return GST_RTSP_STS_SESSION_NOT_FOUND;
	}

	gst_rtsp_message_init_response(
		state->response,
		GST_RTSP_STS_OK,
		gst_rtsp_status_as_text(GST_RTSP_STS_OK),
		state->request
	);

	nmp_rtsp_client_send_response(client, NULL, state->response);

	if (nmp_rtsp_play_session(client, session))
	{
		nmp_warning(
			"Play session '%s' failed.", session
		);
	}

	return GST_RTSP_STS_OK;
}


static GstRTSPStatusCode
nmp_rtsp_handle_teardown_request(NmpRtspClient *client, 
	NmpRtspState *state)
{
	gchar *session;
	GstRTSPResult res;

	res = gst_rtsp_message_get_header(state->request,
		GST_RTSP_HDR_SESSION, &session, 0);
	if (res != GST_RTSP_OK)
	{
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_BAD_REQUEST,
			state
		);
		return GST_RTSP_STS_BAD_REQUEST;		
	}

	if (nmp_rtsp_teardown_session(client, session))
	{
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_SESSION_NOT_FOUND,
			state
		);
		return GST_RTSP_STS_SESSION_NOT_FOUND;		
	}

	gst_rtsp_message_init_response(
		state->response,
		GST_RTSP_STS_OK,
		gst_rtsp_status_as_text(GST_RTSP_STS_OK),
		state->request
	);
	nmp_rtsp_client_send_response(client, NULL, state->response);

	return GST_RTSP_STS_OK;
}



static GstRTSPStatusCode
nmp_rtsp_handle_parameter_request(NmpRtspClient *client, 
	NmpRtspState *state)
{
	nmp_rtsp_client_send_generic_response(
		client,
    	GST_RTSP_STS_OK,
    	state
    );
    return GST_RTSP_STS_OK;
}



static GstRTSPStatusCode
nmp_rtsp_handle_request(NmpRtspClient *client, GstRTSPMessage *request)
{
	GstRTSPMethod method;
	GstRTSPVersion version;
	const gchar *url_str;
	GstRTSPUrl *url;
	NmpRtspState state = { NULL };
	GstRTSPMessage response = { 0 };
	GstRTSPStatusCode rtsp_code = GST_RTSP_STS_OK;

	state.request = request;
	state.response = &response;

	gst_rtsp_message_parse_request(request, &method, &url_str, &version);
	if (version != GST_RTSP_VERSION_1_0)
	{
		nmp_rtsp_client_send_generic_response(
			client, 
			GST_RTSP_STS_RTSP_VERSION_NOT_SUPPORTED,
			&state
		);
		nmp_warning("Client '%p', Bad rtsp version.", client);
	    return GST_RTSP_STS_RTSP_VERSION_NOT_SUPPORTED;
	}

	state.method = method;

	if (gst_rtsp_url_parse(url_str, &url) != GST_RTSP_OK)
	{
    	nmp_rtsp_client_send_generic_response(
    		client,
    		GST_RTSP_STS_BAD_REQUEST,
    		&state
    	);
    	nmp_warning("Parse client '%p' url failed.", client);
		return GST_RTSP_STS_BAD_REQUEST;
	}

	state.url = url;

	switch (method)
	{
	case GST_RTSP_OPTIONS:
		rtsp_code = nmp_rtsp_handle_options_request(client, &state);
		break;

	case GST_RTSP_DESCRIBE:
		rtsp_code = nmp_rtsp_handle_describe_request(client, &state);
		break;

	case GST_RTSP_SETUP:
		rtsp_code = nmp_rtsp_handle_setup_request(client, &state);
		break;

	case GST_RTSP_PLAY:
		rtsp_code = nmp_rtsp_handle_play_request(client, &state);
		break;

	case GST_RTSP_TEARDOWN:
		rtsp_code = nmp_rtsp_handle_teardown_request(client, &state);
		break;

	case GST_RTSP_PAUSE:
		break;

	case GST_RTSP_SET_PARAMETER:
	case GST_RTSP_GET_PARAMETER:
		rtsp_code = nmp_rtsp_handle_parameter_request(client, &state);
		break;

	case GST_RTSP_ANNOUNCE:
	case GST_RTSP_RECORD:
	case GST_RTSP_REDIRECT:
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_NOT_IMPLEMENTED,
			&state
		);
		break;

	case GST_RTSP_INVALID:
	default:
		nmp_rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_BAD_REQUEST,
			&state
		);
		break;
	}

	gst_rtsp_url_free(url);
	return rtsp_code;
}


static GstRTSPResult
nmp_rtsp_message_received(GstRtspWatch *watch, 
	GstRTSPMessage *message, gpointer user_data)	//user_data => client
{
	GstRTSPResult result = GST_RTSP_OK;
	NmpRtspClient *client = (NmpRtspClient*)user_data;

	switch (message->type)
	{
	case GST_RTSP_MESSAGE_REQUEST:
		nmp_rtsp_client_alive(client);
		result = nmp_rtsp_handle_request(client, message);
		break;

	case GST_RTSP_MESSAGE_RESPONSE:
		break;

	case GST_RTSP_MESSAGE_DATA:
		break;

	default:
		break;
	}

	return result;
}


static GstRTSPResult
nmp_rtsp_message_sent(GstRtspWatch *watch, guint cseq,
	gpointer user_data)
{

	return GST_RTSP_OK;
}


static GstRTSPResult
nmp_rtsp_conn_closed(GstRtspWatch *watch, gpointer user_data)
{
	NmpRtspClient *client = (NmpRtspClient*)user_data;
	NmpClientMng *client_mng = client->client_mng;

	nmp_print(
		"Client '%p' '%s:%d' reset connection.",
		client,
		client->client_ip ?: "--",
		client->port
	);

	BUG_ON(client_mng == NULL);

	nmp_rtsp_client_mng_remove_client(client_mng, client);
	nmp_rtsp_client_set_illegal(client);
	return GST_RTSP_OK;
}


static GstRTSPResult
nmp_rtsp_conn_error(GstRtspWatch *watch, GstRTSPResult result,
	gpointer user_data)
{
	NmpRtspClient *client = (NmpRtspClient*)user_data;
	NmpClientMng *client_mng = client->client_mng;
	gchar *err_string = gst_rtsp_strresult(result);

	nmp_print(
		"Client '%p' '%s:%d' connection r/w Error:'%s'.",
		client,
		client->client_ip ?: "--",
		client->port,
		err_string
	);

	BUG_ON(client_mng == NULL);

	g_free(err_string);
	nmp_rtsp_client_mng_remove_client(client_mng, client);
	nmp_rtsp_client_set_illegal(client);

	return GST_RTSP_OK;
}


GstRtspWatchFuncs nmp_rtsp_client_watch_funcs = 
{
	.message_received = nmp_rtsp_message_received,
	.message_sent = nmp_rtsp_message_sent,
	.closed = nmp_rtsp_conn_closed,
	.error = nmp_rtsp_conn_error
};


//:~ End
