#include "rtsp_def.h"
#include "rtsp_impl.h"
#include "tr_log.h"
#include "unix_sock.h"
#include "rtp_sinker.h"
#include "rtsp_session.h"
#include "rtsp_url2.h"

typedef struct __tr_trans tr_trans;
struct __tr_trans
{
	int32_t proto;	/* L4_TCP/L4_UDP */
	char text[MAX_TRANS_TEXT];
};


static __inline__ int32_t
rtsp_method_get_rtp_trans(rtsp_transport *trans)
{
	if (trans->lower_transport == RTSP_LOWER_TRANS_UDP)
	{
		return RTP_OVER_UDP;
	}

	if (trans->lower_transport == RTSP_LOWER_TRANS_TCP)
	{
		if (trans->client_port.min < 0)
		{
			return RTP_OVER_RTSP;
		}
		return RTP_OVER_TCP;
	}

	return RTP_TRANS_INVALID;
}


static __inline__ int32_t
rtsp_method_get_l4_proto(int32_t rtp_trans)
{
	int32_t l4;

	switch (rtp_trans)
	{
	case RTP_OVER_TCP:
		l4 = L4_TCP;
		break;

	case RTP_OVER_UDP:
		l4 = L4_UDP;
		break;

	case RTP_OVER_RTSP:
		l4 = L4_NONE;
		break;

	default:
		l4 = L4_NONE;
		break;
	}

	return l4;
}


static __inline__ int32_t
rtsp_method_parse_client_transport(rtsp_client *rc, rtsp_message *req,
	rtsp_transport *ct, rtp_port_conf *rpc_c)
{
	memset(rpc_c, 0, sizeof(*rpc_c));
	rpc_c->trans = rtsp_method_get_rtp_trans(ct);
	rpc_c->rtsp_client = rc;

	if (rpc_c->trans == RTP_OVER_TCP || rpc_c->trans == RTP_OVER_UDP)
	{
		rpc_c->ports.min_port = ct->client_port.min;
		rpc_c->ports.max_port = ct->client_port.max;
	}
	else
	{
		rpc_c->ports.min_port = ct->interleaved.min;
		rpc_c->ports.max_port = ct->interleaved.max;
	}

	if (ct->destination)
	{
		strncpy(__str(rpc_c->host), ct->destination, MAX_HOST_LENGTH - 1);
	}
	else
	{
		strncpy(__str(rpc_c->host), __str(((network_client*)rc)->ip),
			MAX_HOST_LENGTH - 1);
	}

	if (!strlen(__str(rpc_c->host)))
	{
		LOG_W(
			"Can't get client IP."
		);
		return -EINVAL;
	}

	return 0;
}


static __inline__ int32_t
rtsp_method_generate_st(tr_trans *st, rtp_port_conf *rpc_s,
	rtp_port_conf *rpc_c)
{
	int32_t err = 0;

	switch (rpc_s->trans)
	{
	case RTP_OVER_TCP:
		st->proto = L4_TCP;
		snprintf(
			st->text, 
			MAX_TRANS_TEXT,
			"RTP/AVP/TCP;unicast;client_port=%d-%d;server_port=%d-%d;ssrc=%08X",
			rpc_c->ports.min_port, rpc_c->ports.max_port, rpc_s->ports.min_port,
			rpc_s->ports.max_port, 0
		);
		break;

	case RTP_OVER_UDP:
		st->proto = L4_UDP;
		snprintf(
			st->text,
			MAX_TRANS_TEXT,
			"RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d;ssrc=%08X",
			rpc_c->ports.min_port, rpc_c->ports.max_port, rpc_s->ports.min_port,
			rpc_s->ports.max_port, 0
		);
		break;

	case RTP_OVER_RTSP:
		st->proto = L4_NONE;
		snprintf(
			st->text,
			MAX_TRANS_TEXT,
			/*"RTP/AVP/TCP;interleaved=%d-%d;ssrc=%08X",*/
			"RTP/AVP/TCP;unicast;interleaved=%d-%d;ssrc=%08X;mode=\"play\"",
			rpc_s->ports.min_port, rpc_s->ports.max_port,
			0
		);
		break;

	default:
		err = -EINVAL;
		break;
	}

	return err;
}


static __inline__ void
rtsp_method_init_sp(rtsp_client *rc, sinker_param *sp, media_info *mi,
	int32_t rtp_trans)
{
	int32_t stm_i;

	sp->factory = 0;	//@{TODO: factory chosen}
	sp->l4_proto = rtsp_method_get_l4_proto(rtp_trans);

	for (stm_i = 0; stm_i < ST_MAX; ++stm_i)
	{
		if (stm_i < mi->n_stms)
		{
			sp->stms_type[stm_i] = mi->stms[stm_i].stm_type;
		}
		else
		{
			sp->stms_type[stm_i] = ST_NONE;
		}
	}
}


static __inline__ rtsp_session *
rtsp_method_session_get(rtsp_client *rc, char *sid, media *media,
	int32_t rtp_trans, int32_t *err)
{
	network_sinker *ns;
	sinker_param sp;
	rtsp_session *rtsp_s;
	media_info mi;
	int32_t _err;

	if (!sid)
	{
		_err = media_info_get(media, &mi);
		if (_err)
		{
			LOG_W(
				"rtsp_method_session_get()->media_info_get() failed."
			);
			*err = _err;
			media_info_clear(&mi);
			return NULL;
		}

		rtsp_s = (rtsp_session*)client_create_s((client*)rc,
			(void*)media_seq(media));
		if (!rtsp_s)
		{
			LOG_W(
				"rtsp_method_session_get()->client_create_s() failed."
			);
			*err = -ENOMEM;
			media_info_clear(&mi);
			return NULL;
		}

		rtsp_method_init_sp(rc, &sp, &mi, rtp_trans);
		media_info_clear(&mi);

		ns = (network_sinker*)rtp_sinker_new(&sp);
		if (!ns)
		{
			LOG_W(
				"rtsp_method_session_get()->client_create_s() failed."
			);
			*err = -ENOMEM;
			client_kill_unref_s((client*)rc, (session*)rtsp_s);
			return NULL;
		}

		_err = session_add_sinker((session*)rtsp_s, ns);
		if (_err)
		{
			LOG_W(
				"rtsp_method_session_get()->session_add_sinker() failed."
			);
			*err = _err;
			media_sinker_kill_unref((media_sinker*)ns);
			client_kill_unref_s((client*)rc, (session*)rtsp_s);
			return NULL;
		}

		media_sinker_unref((media_sinker*)ns);
		_err = media_insert_sinker(media, &((media_sinker*)ns)->list);
		if (_err)
		{
			LOG_W(
				"rtsp_method_session_get()->media_insert_sinker() failed."
			);
			*err = _err;
			client_kill_unref_s((client*)rc, (session*)rtsp_s);
			return NULL;
		}

		return rtsp_s;
	}
	else
	{
		rtsp_s = (rtsp_session*)client_find_and_get_s((client*)rc, __u8_str(sid));
		if (!rtsp_s)
		{
			LOG_I(
				"rtsp_method_session_get()->client_find_and_get_s()."
			);
			*err = -ESESSION;
			return NULL;
		}
	}

	return rtsp_s;
}


static __inline__ int32_t
__rtsp_method_do_setup(rtsp_client *rc, rtsp_message *req, media *media, char *sid,
	rtsp_transport *ct, int32_t track, tr_trans *st, char s_id[])
{
	rtsp_session *rtsp_s;
	int32_t rtp_trans, err = 0;
	rtp_port_conf rpc_s, rpc_c;

	if (media_killed(media))
	{
		LOG_W(
			"__rtsp_method_do_setup()->media_killed()."
		);
		return -EKILLED;
	}

	rtp_trans = rtsp_method_get_rtp_trans(ct);
	if (rtp_trans == RTP_TRANS_INVALID)
	{
		LOG_W(
			"__rtsp_method_do_setup()->rtsp_method_get_rtp_trans()."
		);
		return -EINVAL;
	}

	rtsp_s = rtsp_method_session_get(rc, sid, media, rtp_trans, &err);
	if (!rtsp_s)
	{
		LOG_W(
			"__rtsp_method_do_setup()->rtsp_method_session_get() failed."
		);
		return err;
	}

	err = rtsp_method_parse_client_transport(rc, req, ct, &rpc_c);
	if (err)
	{
		LOG_W(
			"__rtsp_method_do_setup()->rtsp_method_parse_client_transport() failed, "
			"err '%d'.", err
		);
		client_kill_unref_s((client*)rc, (session*)rtsp_s);
		return err;
	}

	err = session_set_sinker_config((session*)rtsp_s, &track, &rpc_c);
	if (err)
	{
		LOG_W(
			"__rtsp_method_do_setup()->session_set_sinker_config() failed, "
			"err '%d'.", err
		);
		client_kill_unref_s((client*)rc, (session*)rtsp_s);
		return err;
	}

	err = session_get_sinker_config((session*)rtsp_s, &track, &rpc_s);
	if (err)
	{
		LOG_W(
			"__rtsp_method_do_setup()->session_get_sinker_config() failed, "
			"err '%d'.",err
		);
		client_kill_unref_s((client*)rc, (session*)rtsp_s);
		return err;
	}

	rtsp_method_generate_st(st, &rpc_s, &rpc_c);
	strcpy(s_id, __str(session_id((session*)rtsp_s)));
	session_unref((session*)rtsp_s);
	return 0;
}


static __inline__ void
rtsp_method_do_setup(rtsp_client *rc, rtsp_message *req, media *media, int32_t track)
{
	char *trans, *sid = NULL;
	RTSP_STATUS_CODE rsc;
	RTSP_RESULT ret;
	rtsp_message *res;
	int32_t err = 0;
	rtsp_transport *ct;
	tr_trans st;
	char session_id[SESS_MAX_LEN];

	ret = rtsp_message_get_header(req, RTSP_HDR_TRANSPORT, &trans, 0);
	if (ret != RTSP_OK)
	{
		LOG_W(
			"rtsp_method_do_setup()->rtsp_message_get_header(TRANSPORT) failed."
		);
		err = -EINVAL;
		goto setup_err;		
	}

	rtsp_transport_new(&ct);
	ret = rtsp_transport_parse(trans, ct);
	if (ret != RTSP_OK)
	{
		LOG_W(
			"rtsp_method_do_setup()->rtsp_transport_parse() failed."
		);
		err = -EINVAL;
		goto setup_err;
	}

	rtsp_message_get_header(req, RTSP_HDR_SESSION, &sid, 0);
	err = __rtsp_method_do_setup(rc, req, media, sid, ct, track, &st, session_id);
	if (err)
	{
		LOG_W(
			"rtsp_method_do_setup()->rtsp_transport_parse() failed."
		);
		rtsp_transport_free(ct);
		goto setup_err;		
	}

	rtsp_transport_free(ct);
	res = rtsp_impl_new_generic_response(req, RTSP_STS_OK);
    strcat(session_id, ";timeout=15");
    rtsp_message_add_header(res, RTSP_HDR_SESSION, session_id);
	rtsp_message_add_header(res, RTSP_HDR_TRANSPORT, st.text);
	rtsp_impl_send_message(rc, res);
	return;

setup_err:
	rsc = rtsp_impl_trans_status_code(TR_RTSP_SETUP, err);
	res = rtsp_impl_new_generic_response(req, rsc);
	rtsp_impl_send_message(rc, res);
	return;
}


static void
rtsp_method_pq_setup_fun(void *parm1, void *parm2, void *parm3)
{
	rtsp_wait_block *rwb = (rtsp_wait_block*)parm1;

	rtsp_method_do_setup(rwb->rc, rwb->req_msg, rwb->orig_media, rwb->i_track);
	rtsp_wait_block_free(rwb);
}


void
rtsp_method_on_setup(rtsp_client *rc, rtsp_message *req)
{
	rtsp_message *res;
	int32_t err, track;
	RTSP_STATUS_CODE rsc;

	err = rtsp_impl_queue_request(rc, req, &track, W_MEDIA_STM,
		rtsp_method_pq_setup_fun);
	if (!err)
	{
		return;
	}

	rsc = rtsp_impl_trans_status_code(TR_RTSP_SETUP, err);
	res = rtsp_impl_new_generic_response(req, rsc);
	rtsp_impl_send_message(rc, res);
}


//:~ End
