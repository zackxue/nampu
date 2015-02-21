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

#include "rtsp.h"
#include "rc_log.h"

#include "media.h"
#include "rtp.h"
#include "mediautils.h"


#define DEFAULT_TTL				32

typedef struct _sdp_append_pair sdp_append_pair;
struct _sdp_append_pair
{
    GstSDPMedia *media;
    Track *track;
};


static void
rtsp_sdp_track_private_append(gpointer element, gpointer user_data)
{
    sdp_field *field = (sdp_field *)element;
    sdp_append_pair *pair = (sdp_append_pair *)user_data;
    gchar *tmp;

    switch (field->type)
    {
    case empty_field:
    	gst_sdp_media_add_attribute(pair->media, NULL, field->field);	//?
        break;

    case fmtp:
    	tmp = g_strdup_printf("%u %s", pair->track->properties.payload_type,
    		field->field);
    	gst_sdp_media_add_attribute(pair->media, "fmtp", tmp);
        g_free(tmp);
        break;

    case rtpmap:
    	tmp = g_strdup_printf("%u %s", pair->track->properties.payload_type,
    		field->field);
    	gst_sdp_media_add_attribute(pair->media, "rtpmap", tmp);
        g_free(tmp);
        break;

	case pri:
    	tmp = g_strdup_printf("%u %s", pair->track->properties.payload_type,
    		field->field);
    	gst_sdp_media_add_attribute(pair->media, "pri", tmp);
        g_free(tmp);
        break;

    default:
        break;
    }
}


static void
rtsp_sdp_track_descr(gpointer element, gpointer user_data)
{
	Track *track = (Track *)element;
	TrackInfo *t_info = track->info;
	GstSDPMessage *sdp = (GstSDPMessage*)user_data;
	GstSDPMedia *media;
	gchar *track_name, *tmp;
	MediaType type;
	gfloat frame_rate;

	static const gchar *const sdp_media_types[] =
	{
	    [MP_audio] = "audio",
	    [MP_video] = "video",
	    [MP_application] = "application",
	    [MP_data] = "data",
	    [MP_control] = "control"
	};

	type = track->properties.media_type;
	g_assert(type != MP_undef);

	gst_sdp_media_new(&media);
	gst_sdp_media_set_media(media, sdp_media_types[type]);
	gst_sdp_media_set_port_info(media, t_info->rtp_port, 1);
	gst_sdp_media_set_proto(media, "RTP/AVP");

	tmp = g_strdup_printf("%u", track->properties.payload_type);
	gst_sdp_media_add_format(media, tmp);
	g_free(tmp);

	// i=*
	// c=*
	// b=*
	// k=*
	// a=*
	track_name = g_uri_escape_string(t_info->name, NULL, FALSE);
	tmp = g_strdup_printf(SDP_TRACK_SEPARATOR"%s", track_name);
	gst_sdp_media_add_attribute(media, "control", tmp);
	g_free(tmp);
	g_free(track_name);
	
	if ((frame_rate = track->properties.frame_rate) && type == MP_video)
	{
		tmp = g_strdup_printf("%f", frame_rate);
		gst_sdp_media_add_attribute(media, "framerate", tmp);
		g_free(tmp);
	}

	{
	    sdp_append_pair pair =
	    {
	        .media = media,
	        .track = track
	    };
	    g_slist_foreach(track->sdp_fields,
	    	rtsp_sdp_track_private_append, &pair);
	}

	if (t_info->commons_deed[0])
		gst_sdp_media_add_attribute(media, "uriLicense", t_info->commons_deed);

	if (t_info->rdf_page[0])
		gst_sdp_media_add_attribute(media, "uriMetadata", t_info->rdf_page);

	if (t_info->title[0])
		gst_sdp_media_add_attribute(media, "title", t_info->title);

	if (t_info->author[0])
		gst_sdp_media_add_attribute(media, "author", t_info->author);

	gst_sdp_message_add_media(sdp, media);
	gst_sdp_media_free(media);
}


void
rtsp_handle_describe_request(RTSP_Client *client, RTSP_Ps *state)
{
	GstSDPMessage *sdp;
	Resource *r;
	gchar *session_name, *range_e, *str, *content_base;
	guint str_len;
	gdouble drange_e;
	ResourceInfo *res_info;

	r = r_open(state->url->abspath, OM_DESC);
	if (G_UNLIKELY(!r))
	{
		rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_NOT_FOUND,
			state
		);
		rc_log(RC_LOG_ERR, "[SDP] %s is not found.", state->url->abspath);
		return;
	}

	res_info = r->info;
	g_assert(res_info);

	gst_sdp_message_new(&sdp);

	/* v=0 */
	gst_sdp_message_set_version(sdp, "0");

	/* o=- 12345678910111213 1 IN IP4 192.168.1.163 */
	gst_sdp_message_set_origin(sdp, "-", "12345678910111213", "1", "IN", "IP4",
    	state->url->host ?: "0.0.0.0");

	/* s=LibFoy RTSP Session */
	if ((session_name = res_info->description) == NULL)
		session_name = "LibFoy RTSP Session";
	gst_sdp_message_set_session_name(sdp, session_name);

	/* u= */
    if (res_info->descrURI)
        gst_sdp_message_set_uri(sdp, res_info->descrURI);

	/* e= */
    if (res_info->email)
		gst_sdp_message_add_email(sdp, res_info->email);

	/* p= */
	if (res_info->phone)
		gst_sdp_message_add_phone(sdp, res_info->phone);

	/* c= */
	gst_sdp_message_set_connection(
		sdp,
		"IN",
		"IP4",
		res_info->multicast[0] ? res_info->multicast : "0.0.0.0",
		(res_info->multicast[0] &&  res_info->ttl[0]) ? atoi(res_info->ttl) : DEFAULT_TTL,
		0
	);

	/* t= */
	gst_sdp_message_add_time(sdp, "0", "0", NULL);

	/* a= */
	gst_sdp_message_add_attribute(sdp, "type", "broadcast");
	gst_sdp_message_add_attribute(sdp, "tool", "LibFoy");
	gst_sdp_message_add_attribute(sdp, "control", "*");
    if ((drange_e = res_info->duration) > 0 && drange_e != HUGE_VAL)
    {
    	range_e = g_strdup_printf("npt=0-%f", drange_e);
		gst_sdp_message_add_attribute(sdp, "range", range_e);
		g_free(range_e);
	}

    g_list_foreach(r->tracks, rtsp_sdp_track_descr, sdp);
    r_close(r);

	gst_rtsp_message_init_response(state->response, GST_RTSP_STS_OK,
		gst_rtsp_status_as_text(GST_RTSP_STS_OK), state->request);

	gst_rtsp_message_add_header (state->response, GST_RTSP_HDR_CONTENT_TYPE,
		"application/sdp");

	str = gst_rtsp_url_get_request_uri(state->url);
	str_len = strlen(str);
	if (str[str_len - 1] != '/')
	{
		content_base = g_malloc(str_len + 2);
		memcpy(content_base, str, str_len);
		content_base[str_len] = '/';
		content_base[str_len + 1] = '\0';
		g_free(str);
	}
	else
	{
	    content_base = str;
	}

	gst_rtsp_message_add_header(state->response, GST_RTSP_HDR_CONTENT_BASE,
		content_base);
	g_free(content_base);

	str = gst_sdp_message_as_text(sdp);
	gst_rtsp_message_take_body(state->response, (guint8*)str, strlen(str));
	gst_sdp_message_free(sdp);

	rtsp_client_send_response(client, NULL, state->response);
}

//:~ End
