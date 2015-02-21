#include "rtsp_def.h"
#include "rtsp_impl.h"
#include "tr_log.h"
#include "version.h"
#include "alloc.h"

#define MAX_SDP_SIZE			4096
#define DEFAULT_TTL				32
#define MAX_ATTR_LEN			1024

//#define TEST_ONVIF

static __inline__ void
sdp_media_add_rtpmap(sdp_media *m, stm_info *stm, int32_t pt)
{
	char attr[MAX_ATTR_LEN];

	switch (stm->stm_type)
	{
	case ST_VIDEO:
		snprintf(attr, MAX_ATTR_LEN, "%d %s/%u", pt, __str(stm->encoding_name),
			stm->sample_rate);
		sdp_media_add_attribute(m, "rtpmap", attr);
#ifdef TEST_ONVIF
		sdp_media_add_attribute(m, "x-onvif-track", "VIDEO001");
#endif
        break;

	case ST_AUDIO:
		snprintf(attr, MAX_ATTR_LEN, "%d %s/%u/%d", pt, __str(stm->encoding_name),
			stm->sample_rate, stm->audio.audio_channels);
		sdp_media_add_attribute(m, "rtpmap", attr);
		snprintf(attr, MAX_ATTR_LEN, "%d", 1000/stm->frame_rate);
		sdp_media_add_attribute(m, "ptime", attr);
		
#ifdef TEST_ONVIF
        sdp_media_add_attribute(m, "x-onvif-track", "AUDIO001");
#endif
		break;

	default:
		break;
	}
}


static __inline__ void
__sdp_media_append_fields(sdp_media *m, stm_into_pri *field, int32_t pt)
{
	char attr[MAX_ATTR_LEN];

	switch (field->field_type)
	{
	case FIELD_EMPTY:
    	sdp_media_add_attribute(m, "", __str(field->data));
		break;

	case FIELD_FMTP:
    	snprintf(attr, MAX_ATTR_LEN, "%d %s", pt, __str(field->data));
    	sdp_media_add_attribute(m, "fmtp", attr);
        break;

	case FILED_PRI:
    	snprintf(attr, MAX_ATTR_LEN, "%d %s", pt, __str(field->data));
    	sdp_media_add_attribute(m, "pri", attr);
        break;

	default:
		break;
	}
}


static __inline__ void
sdp_media_append_fields(sdp_media *media, stm_info *stm, int32_t pt)
{
	int32_t i;

	for (i = 0; i < stm->fields; ++i)
	{
		__sdp_media_append_fields(media, &stm->pri_fields[i], pt);
	}
}


static __inline__ sdp_message *
rtsp_method_mi_to_sdp(media_info *mi)
{
#define _MAX_RANGE	64
	sdp_message *sdp;
	char max_range[_MAX_RANGE];
	int32_t stm_i, stm_type;
	sdp_media *media;

	sdp_message_new(&sdp);
	sdp_message_set_version(sdp, "0");
#if 1
    sdp_message_set_origin(sdp, "-", "12345678910111213", "1", "IN", "IP4", "0.0.0.0");
#else
    sdp_message_set_origin(sdp, "-", "12345678910111213", "1", "IN", "IP4", "192.168.1.39");
#endif
    sdp_message_set_session_name(sdp, (const char *)mi->descp);
#if 0
    sdp_message_add_email(sdp, (const char *)mi->email);
	sdp_message_add_phone(sdp, (const char *)mi->phone);
#endif
    sdp_message_set_connection(sdp, "IN", "IP4", "0.0.0.0", DEFAULT_TTL, 0);
	sdp_message_add_time(sdp, "0", "0", NULL);
#if 0
    sdp_message_add_attribute(sdp, "tool", TR_SERVER_SHORT_BANNER);
	sdp_message_add_attribute(sdp, "type", "broadcast");
#endif
    sdp_message_add_attribute(sdp, "control", "*");
#if 1
    snprintf(max_range, _MAX_RANGE, "%s", mi->range[0] ? (char*)mi->range : "0-");
    sdp_message_add_attribute(sdp, "range", max_range);
#endif

	for (stm_i = 0; stm_i < mi->n_stms; ++stm_i)
	{
		sdp_media_new(&media);
		stm_type = mi->stms[stm_i].stm_type;
		sdp_media_set_media(media, __str(__stm_name(stm_type)));
		sdp_media_set_port_info(media, 0, 1);
		sdp_media_set_proto(media, __str(__stm_proto(stm_type)));
		sprintf(max_range, "%d", __stm_pt(stm_type, mi->stms[stm_i].fmt));
		sdp_media_add_format(media, max_range);
		sdp_media_add_bandwidth(media, "AS", mi->stms[stm_i].bit_rate);
		sdp_media_add_rtpmap(media, &mi->stms[stm_i], __stm_pt(stm_type, mi->stms[stm_i].fmt));
        sprintf(max_range, TRACK_INDICATOR"%d", stm_i);
		sdp_media_add_attribute(media, "control", max_range);
        sdp_media_append_fields(media, &mi->stms[stm_i], __stm_pt(stm_type, mi->stms[stm_i].fmt));

		sdp_message_add_media(sdp, media);
		sdp_media_free(media);
	}

	return sdp;
}


static __inline__ char *
rtsp_method_get_content_base(rtsp_message *req, uint32_t *c_size)
{
	const char *url = NULL, *p;
	char *content_base;
	uint32_t size;

	rtsp_message_parse_request(req, NULL, &url, NULL, NULL);
	if (!url)
	{
		return NULL;
	}

	size = strlen(url);
	p = url + size;
	size += 1;	//@{'\0'}

	if (p > url && *(p - 1) != '/')
	{
		size += 1;
		p = NULL;
	}

	content_base = tr_alloc(size);
	strcpy(content_base, url);
	if (!p)
	{
		strcat(content_base, "/");
	}

	*c_size = size;
	return content_base;
}


static __inline__ void
rtsp_method_do_desc(rtsp_client *rc, rtsp_message *req, media *media)
{
	RTSP_STATUS_CODE rsc;
	rtsp_message *res;
	int32_t err, sdp_size;
	media_info mi;
	sdp_message *sdp = NULL;
	char *content_base = NULL;
	uint8_t *sdp_buf;
	uint32_t cb_size;

	err = media_info_get(media, &mi);
	if (!err)
	{
		content_base = rtsp_method_get_content_base(req, &cb_size);
		if (!content_base)
		{
			LOG_W(
				"rtsp_method_do_desc()->rtsp_method_get_content_base() failed."
			);
			err = -EINVAL;
			goto desc_err;
		}

		sdp = rtsp_method_mi_to_sdp(&mi);
		sdp_buf = (uint8_t*)rtsp_mem_alloc(MAX_RTSP_BUF_SIZE);
		sdp_size = sdp_message_as_text(sdp, (char*)sdp_buf, MAX_RTSP_BUF_SIZE);
		sdp_message_free(sdp);
		if (sdp_size < 0 || sdp_size >= MAX_RTSP_BUF_SIZE)
		{
			LOG_W(
				"rtsp_method_do_desc()->sdp_message_as_text() failed."
			);
			tr_free(content_base, cb_size);
			err = -EINVAL;
			goto desc_err;
		}

		media_info_clear(&mi);
		res = rtsp_impl_new_generic_response(req, RTSP_STS_OK);
		rtsp_message_take_body(res, sdp_buf, sdp_size);
		rtsp_message_add_header(res, RTSP_HDR_CONTENT_TYPE, "application/sdp");
		rtsp_message_add_header(res, RTSP_HDR_CONTENT_BASE, content_base);
		tr_free(content_base, cb_size);
		rtsp_impl_send_message(rc, res);
		return;
	}

desc_err:
	media_info_clear(&mi);
	rsc = rtsp_impl_trans_status_code(TR_RTSP_DESC, err);
	res = rtsp_impl_new_generic_response(req, rsc);
	rtsp_impl_send_message(rc, res);
}


static void
rtsp_method_pq_desc_fun(void *parm1, void *parm2, void *parm3)
{
	rtsp_wait_block *rwb = (rtsp_wait_block*)parm1;

	rtsp_method_do_desc(rwb->rc, rwb->req_msg, rwb->orig_media);
	rtsp_wait_block_free(rwb);
}


void
rtsp_method_on_desc(rtsp_client *rc, rtsp_message *req)
{
	RTSP_STATUS_CODE rsc;
	rtsp_message *res;
	const char *url = NULL;
	int32_t err;

	rtsp_message_parse_request(req, NULL, &url, NULL, NULL);
	if (!url)
	{
		LOG_I(
			"Invalid request url ''."
		);
		goto desc_err;
	}

	err = rtsp_impl_queue_request(rc, req, NULL, W_MEDIA_INFO,
		rtsp_method_pq_desc_fun);
	if (!err)
	{
		return;
	}

desc_err:
	rsc = rtsp_impl_trans_status_code(TR_RTSP_DESC, err);
	res = rtsp_impl_new_generic_response(req, rsc);
	rtsp_impl_send_message(rc, res);
}


//:~ End
