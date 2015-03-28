#include <string.h>
#include "nmp_rtsp.h"
#include "nmp_device_mng.h"
#include "nmp_debug.h"
#include "nmp_path_regex.h"

#define KEEP_ALIVE_CHECK_TIMES		3

#define ID_REGEX_1 "PUID=[A-Z]{3}-[A-Z]{3}-\\d{8}"
#define ID_REGEX_2 "PUID=MSS-\\d{4}"
#define MDSIP_REGEX "[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}"
#define KA_REGEX "keepalive=\\d+"
#define L4_REGEX "l4proto=\\d+"


typedef struct _NmpDeviceReg NmpDeviceReg;
struct _NmpDeviceReg		/* 设备注册所携带的信息 */
{
	gchar		id[MAX_DEVICE_ID_LEN];	/* 设备ID */
	gchar		ip[__MAX_IP_LEN];
	gint		keep_alive_time;		/* 保活周期，秒 */
	gint		l4_proto;			/* 流类型: 0-UDP, 1-TCP */
};


static __inline__ gint
nmp_rtsp_parse_abspath(const gchar *path, NmpDeviceReg *reg)
{
	gchar *id, *ka, *l4;
	gint ka_int = 0;

	if (!path || !*path)
		return -1;

	memset(reg, 0, sizeof(*reg));
	reg->keep_alive_time = MAX_DEVICE_TTD_SEC/KEEP_ALIVE_CHECK_TIMES;

	id = nmp_rtsp_param_string(path, ID_REGEX_1);
	if (!id)
	{
		id = nmp_rtsp_param_string(path, ID_REGEX_2);
		if (!id)
			return -1;
	}

	sscanf(id, "PUID=%s", reg->id);
	g_free(id);

	if (reg->id[0] == 0)
		return -1;

	ka = nmp_rtsp_param_string(path, KA_REGEX);
	if (ka)
	{
		sscanf(ka, "keepalive=%d", &ka_int);
		g_free(ka);
	}

	l4 = nmp_rtsp_param_string(path, L4_REGEX);
	if (l4)
	{
		sscanf(l4, "l4proto=%d", &reg->l4_proto);
		g_free(l4);		
	}

	if (ka_int > 0)
		reg->keep_alive_time = ka_int;

	if (reg->keep_alive_time * KEEP_ALIVE_CHECK_TIMES <= 0)		/* 溢出 */
		reg->keep_alive_time = MAX_DEVICE_TTD_SEC/KEEP_ALIVE_CHECK_TIMES;

	return 0;
}


static __inline__ void
nmp_rtsp_parse_host(const gchar *host, NmpDeviceReg *reg)
{
	char *ip;

	if (host)
	{
		ip = nmp_rtsp_param_string(host, MDSIP_REGEX);
		if (ip)
		{
			strncpy(reg->ip, ip, __MAX_IP_LEN - 1);
			g_free(ip);
			return;
		}
	}

	reg->ip[0] = 0;
}


static __inline__ void
nmp_rtsp_device_send_generic_response(NmpMediaDevice *device,
	GstRTSPStatusCode code, NmpRtspState *state)
{
	gst_rtsp_message_init_response(state->response, code,
		gst_rtsp_status_as_text(code), state->request);

	nmp_rtsp_device_send_response(device, state->response);
}


static __inline__ gint
nmp_rtsp_do_device_opt(NmpMediaDevice *device, NmpDeviceReg *reg)
{
	NmpDeviceMediaType mt;
	gchar conflict_ip[__MAX_IP_LEN];

	if (G_UNLIKELY(nmp_rtsp_device_is_new(device)))
	{
		nmp_rtsp_device_set_info(device, reg->id, reg->ip,
			KEEP_ALIVE_CHECK_TIMES * reg->keep_alive_time);

		mt = reg->l4_proto == 0 ?  NMP_MT_TCP : NMP_MT_UDP;
		nmp_rtsp_device_set_media_type(device, mt);

		if (!nmp_rtsp_device_mng_accepted(
			(NmpDevMng*)device->private_data, device))
		{
			nmp_print(
				"Device '%p', ID:'%s' register timeout.",
				device, reg->id
			);
			return -1;
		}

		nmp_rtsp_device_set_registered(device);

		if (nmp_rtsp_device_mng_add_dev(
			(NmpDevMng*)device->private_data, device, conflict_ip))
		{
			nmp_print(
				"Refuse device '%p', ID:'%s' is already used by '%s'.", 
				 device, reg->id, conflict_ip
			);
			nmp_rtsp_device_set_illegal(device);
			return -1;
		}

		nmp_print(
			"Device '%p' ID:'%s' registered, ka=%d, l4proto=%d.",
			device, reg->id, reg->keep_alive_time, reg->l4_proto
		);

		return 0;
	}
	else
	{
		if (G_UNLIKELY(!nmp_rtsp_device_check_id(device, reg->id, 
				KEEP_ALIVE_CHECK_TIMES * reg->keep_alive_time)))
		{
			nmp_print(
				"Device '%p' changes it's ID from '%s' to '%s'!",
				device, device->id, reg->id
			);

			nmp_rtsp_device_mng_remove(device);
			nmp_rtsp_device_set_illegal(device);
			return -1;
		}

		return 0;
	}
}


static __inline__ GstRTSPStatusCode
nmp_rtsp_do_options_request(NmpMediaDevice *device, GstRTSPUrl *url)
{
	NmpDeviceReg reg_info;

	if (G_UNLIKELY(nmp_rtsp_parse_abspath(url->abspath, &reg_info)))
	{
		/*
		 * URL格式不正确, 返回400.(Bad Request)
		*/
		return GST_RTSP_STS_BAD_REQUEST;
	}

	nmp_rtsp_parse_host(url->host, &reg_info);

	if (G_UNLIKELY(nmp_rtsp_do_device_opt(device, &reg_info)))
	{
		/*
		 * 设备ID冲突, 返回401.(Unauthorized)
		*/		
		return GST_RTSP_STS_UNAUTHORIZED;
	}

	return GST_RTSP_STS_OK;
}


static __inline__ void
nmp_rtsp_give_options_response(NmpMediaDevice *device, 
	NmpRtspState *state, GstRTSPStatusCode code)
{
	GstRTSPMethod options;
	gchar *str;

	if (code == GST_RTSP_STS_OK)
	{
		options = GST_RTSP_OPTIONS | GST_RTSP_TEARDOWN;
		str = gst_rtsp_options_as_text(options);
		gst_rtsp_message_init_response(state->response, GST_RTSP_STS_OK,
	 		gst_rtsp_status_as_text(GST_RTSP_STS_OK), state->request);
		gst_rtsp_message_add_header(state->response, GST_RTSP_HDR_PUBLIC, str);
		g_free(str);
		nmp_rtsp_device_send_response(device, state->response);	
	}
	else
	{
		nmp_rtsp_device_send_generic_response(device, code, state);
	}
}


static GstRTSPStatusCode
nmp_rtsp_handle_options_request(NmpMediaDevice *device, 
	NmpRtspState *state)
{
	GstRTSPStatusCode rtsp_code;

	rtsp_code = nmp_rtsp_do_options_request(device, state->url);
	nmp_rtsp_give_options_response(device, state, rtsp_code);
	return rtsp_code;
}


static GstRTSPStatusCode
nmp_rtsp_handle_request(NmpMediaDevice *device, GstRTSPMessage *request)
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
		/* we can only handle 1.0 requests */
		nmp_rtsp_device_send_generic_response(device, 
			GST_RTSP_STS_RTSP_VERSION_NOT_SUPPORTED, &state);
	    return GST_RTSP_STS_RTSP_VERSION_NOT_SUPPORTED;
	}

	state.method = method;

	if (gst_rtsp_url_parse(url_str, &url) != GST_RTSP_OK)
	{
    	nmp_rtsp_device_send_generic_response(device,
    		GST_RTSP_STS_BAD_REQUEST, &state);
		return GST_RTSP_STS_BAD_REQUEST;
	}

	state.url = url;

	switch (method)
	{
	case GST_RTSP_OPTIONS:
		rtsp_code = nmp_rtsp_handle_options_request(device, &state);
		break;

	case GST_RTSP_TEARDOWN:
//		nmp_rtsp_handle_teardown_request(device, &state);
		break;

	case GST_RTSP_DESCRIBE:
	case GST_RTSP_SETUP:
	case GST_RTSP_PLAY:
	case GST_RTSP_PAUSE:
	case GST_RTSP_SET_PARAMETER:
	case GST_RTSP_GET_PARAMETER:
	case GST_RTSP_ANNOUNCE:
	case GST_RTSP_RECORD:
	case GST_RTSP_REDIRECT:
		nmp_rtsp_device_send_generic_response(device,
		GST_RTSP_STS_NOT_IMPLEMENTED, &state);
		break;

	case GST_RTSP_INVALID:
	default:
		nmp_rtsp_device_send_generic_response(device,
		GST_RTSP_STS_BAD_REQUEST, &state);
		break;
	}

	gst_rtsp_url_free(url);
	return rtsp_code;
}


static GstRTSPStatusCode
nmp_rtsp_handle_response(NmpMediaDevice *device, GstRTSPMessage *response)
{
	GstRTSPStatusCode res_code;
	GstRTSPVersion version;
	const gchar *reason = NULL;
	gchar *sessid = NULL, *cseq = NULL, *content_base = NULL;
	GstRTSPResult ret;
	NmpRtspMedia *media = NULL;
	NmpSessionState s_state;
	GstSDPMessage *sdp = NULL;
	guint8 *sdp_body = NULL;
	guint sdp_size, seq;

	gst_rtsp_message_parse_response(response, &res_code, &reason, &version);
	if (version != GST_RTSP_VERSION_1_0)
	{
		nmp_warning("Device RTSP response version mismatch");
		return GST_RTSP_STS_RTSP_VERSION_NOT_SUPPORTED;
	}

	ret = gst_rtsp_message_get_header(response, GST_RTSP_HDR_CSEQ, &cseq, 0);
	if (ret != GST_RTSP_OK)
	{
		nmp_warning("Device RTSP response, No 'CSeq:' field");
		return GST_RTSP_STS_INVALID;
	}

	seq = atoi(cseq);
	if (!seq)
		return GST_RTSP_OK;

	media = nmp_rtsp_device_get_media(device, seq);
	if (!media)
	{
		//TEARDOWN或者SET_PARAMETER响应
		nmp_print(
			"Drop device response(TEARDOWN|SET_PARAMETER), seq:'%s'.", cseq
		);
		return GST_RTSP_OK;
	}

	s_state = nmp_rtsp_media_get_session_state(media);
	if (s_state == NMP_SESSION_TEARDOWN)
	{
		nmp_print(
			"Response _After_ TEARDOWN ?? seq:'%s'", cseq
		);
		goto handle_teardown_ok;
	}

	if (res_code != GST_RTSP_STS_OK)
	{
		nmp_warning("Device RTSP res-code: %d", res_code);
		goto handle_response_failed;
	}

	ret = gst_rtsp_message_get_header(response, GST_RTSP_HDR_SESSION, &sessid, 0);
	if (ret != GST_RTSP_OK)
	{
		//只有DESCRIBE消息没有SESSION域
		if (s_state != NMP_SESSION_DESCRIBE)
		{
			nmp_warning("Device RTSP response, No 'Session:' field");
			goto handle_response_failed;
		}

		ret = gst_rtsp_message_get_header(response, GST_RTSP_HDR_CONTENT_BASE, 
			&content_base, 0);
		if (ret != GST_RTSP_OK)
		{
			nmp_warning("Device RTSP response, No 'Content-Base:' field");
			goto handle_response_failed;
		}

		//处理DESCRIBE消息
		if (gst_sdp_message_new(&sdp) != GST_SDP_OK)
		{
			nmp_warning("Create SDP message failed");
			goto handle_response_failed;			
		}

		gst_rtsp_message_get_body(response, &sdp_body, &sdp_size);
		if (gst_sdp_message_parse_buffer(sdp_body, sdp_size, sdp) != GST_SDP_OK)
		{
			nmp_warning("Device DESCRIBE response, invalid SDP info");
			gst_sdp_message_free(sdp);
			goto handle_response_failed;
		}

		nmp_rtsp_media_set_sdp_info(media, sdp);
	}
	else
	{
		if (s_state == NMP_SESSION_SETUP)
			nmp_rtsp_media_set_session_id(media, sessid);
	}

	if (nmp_rtsp_media_session_state_next(media) == NMP_SESSION_PLAY)
		nmp_rtsp_media_deal_pending_request(media);

	if (nmp_rtsp_device_request(device, media))
	{
		nmp_warning("Device RTSP Request failed");
		goto handle_response_failed;
	}

handle_teardown_ok:

	nmp_rtsp_media_unref(media);
	return GST_RTSP_OK;

handle_response_failed:
	nmp_rtsp_media_die_announce(media, device);
//	nmp_rtsp_media_deal_pending_request(media);	/* nmp_rtsp_media_kill_unref() can do this */
	nmp_rtsp_device_remove_media(device, media);
	nmp_rtsp_media_kill_unref(media);

	return GST_RTSP_OK;
}


static GstRTSPResult
nmp_rtsp_message_received(GstRtspWatch *watch, 
	GstRTSPMessage *message, gpointer user_data)	//user_data => device
{
	GstRTSPResult result = GST_RTSP_OK;
	NmpMediaDevice *device = (NmpMediaDevice*)user_data;

	nmp_rtsp_device_update_ttd(device);

	switch (message->type)
	{
	case GST_RTSP_MESSAGE_REQUEST:
		result = nmp_rtsp_handle_request(device, message);
		break;

	case GST_RTSP_MESSAGE_RESPONSE:
		result = nmp_rtsp_handle_response(device, message);
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
	NmpMediaDevice *device = (NmpMediaDevice*)user_data;

	nmp_print(
		"Device '%p', ID: '%s' reset connection.",
		device, device->id
	);

	nmp_rtsp_device_mng_remove(device);
	nmp_rtsp_device_set_illegal(device);

	return GST_RTSP_OK;
}


static GstRTSPResult
nmp_rtsp_conn_error(GstRtspWatch *watch, GstRTSPResult result,
	gpointer user_data)
{
	NmpMediaDevice *device = (NmpMediaDevice*)user_data;
	gchar *err_string = gst_rtsp_strresult(result);

	nmp_print(
		"Device '%p', ID: '%s' connection r/w Error:'%s'.", 
		device, device->id, err_string
	);

	g_free(err_string);
	nmp_rtsp_device_mng_remove(device);
	nmp_rtsp_device_set_illegal(device);

	return GST_RTSP_OK;
}


GstRtspWatchFuncs nmp_rtsp_watch_funcs = 
{
	/*
	 * message_received()/message_sent()的返回值是不被GST检查的，
	 * 原因很简单:这属于较高层次(即协议处理层面的事)，应在上层
	 * 完成。
	*/
	nmp_rtsp_message_received,
	nmp_rtsp_message_sent,

	/*
	 * 读到了EOF
	*/
	nmp_rtsp_conn_closed,

	/*
	 * 读或写出错
	*/
	nmp_rtsp_conn_error
};


//:~ End
