#include "tr_log.h"
#include "rtsp_client.h"


static __inline__ uint8_t *
rtsp_cmd_p(int32_t id)
{
	uint8_t *str[] = {
		"UNKNOW",
		"OPTION",
		"DESCRIBE",
		"SETUP",
		"PLAY",
        "PAUSE",
		"TEARDOWN",
		"SET_PARAMETER",
		"GET_PARAMETER",
		"$DATA",
		"RESPONSE"
	};

	if (id < TR_RTSP_UNKNOWN || id > TR_RTSP_RESPONSE)
		return "ERROR";
	return str[id];
}


static int32_t
rtsp_client_init(network_client *c)
{
//	rtsp_client *rc = (rtsp_client*)c;
	return 0;
}


static void
rtsp_client_finalize(network_client *c)
{
	rtsp_client *rc = (rtsp_client*)c;

	if (rc->ops && rc->ops->fin)
	{
		(*rc->ops->fin)(rc);
	}
}


static session *
rtsp_client_create_session(network_client *c, void *p)
{
	rtsp_client *rc = (rtsp_client*)c;
	session *s = NULL;

	if (rc->ops && rc->ops->create_session)
	{
		s = (*rc->ops->create_session)(rc, p);
	}

	return s;
}


static __inline__ uint32_t
rtsp_client_recognize(rtsp_client *rc, msg *req)
{
	uint32_t id = TR_RTSP_UNKNOWN;

	if (rc->ops && rc->ops->recognize)
	{
		id = (*rc->ops->recognize)(rc, req);
	}

	return id;
}


#define SET_RTSP_MSG_HANDLER(c) \
static __inline__ int32_t \
rtsp_client_handler_##c(rtsp_client *rc, msg *req) \
{\
	int32_t err = -EPERM; \
\
	if (rc->ops && rc->ops->on_##c) \
	{\
		err = (*rc->ops->on_##c)(rc, req); \
	}\
\
	return err;\
}

SET_RTSP_MSG_HANDLER(option)
SET_RTSP_MSG_HANDLER(desc)
SET_RTSP_MSG_HANDLER(setup)
SET_RTSP_MSG_HANDLER(play)
SET_RTSP_MSG_HANDLER(pause)
SET_RTSP_MSG_HANDLER(teardown)
SET_RTSP_MSG_HANDLER(setparm)
SET_RTSP_MSG_HANDLER(getparm)
SET_RTSP_MSG_HANDLER(data)

#define CALL_HANDLER(c, rc, req) \
	 rtsp_client_handler_##c((rc), (req))


static int32_t
rtsp_client_handle_msg(network_client *c, msg *req)
{
	int32_t id, err = -EPERM;
	rtsp_client *rc = (rtsp_client*)c;

	id = rtsp_client_recognize(rc, req);
	LOG_I(
		"rtsp-client '%p' handle msg '%s'.",
		c, __str(rtsp_cmd_p(id)));

	switch (id)
	{
	case TR_RTSP_OPTION:
		err = CALL_HANDLER(option, rc, req);
		break;

	case TR_RTSP_DESC:
		err = CALL_HANDLER(desc, rc, req);
		break;

	case TR_RTSP_SETUP:
		err = CALL_HANDLER(setup, rc, req);
		break;

	case TR_RTSP_PLAY:
		err = CALL_HANDLER(play, rc, req);
		break;

	case TR_RTSP_PAUSE:
		err = CALL_HANDLER(pause, rc, req);
		break;

	case TR_RTSP_TEARDOWN:
		err = CALL_HANDLER(teardown, rc, req);
		break;

	case TR_RTSP_SETPARM:
		err = CALL_HANDLER(setparm, rc, req);
		break;

	case TR_RTSP_GETPARM:
		err = CALL_HANDLER(getparm, rc, req);
		break;

	case TR_RTSP_DATA:
		err = CALL_HANDLER(data, rc, req);
		break;

	case TR_RTSP_RESPONSE:
		err = 0;
		break;

	case TR_RTSP_UNKNOWN:
		break;
	}

	if (!err)
	{
		LOG_I(
			"rtsp-client '%p' handle msg '%s' ok.",
			c, __str(rtsp_cmd_p(id)));
	}
	else
	{
		LOG_W(
			"rtsp-client '%p' handle msg '%s' failed!",
			c, __str(rtsp_cmd_p(id)));
	}
	return err;
}


static int32_t
rtsp_client_msg_sent(network_client *c, uint32_t seq)
{
#if 0
	int32_t err = -EINVAL;
	rtsp_client *rc = (rtsp_client*)c;

	if (rc->ops && rc->ops->on_msg_sent)
	{
		err = (*rc->ops->on_msg_sent)(rc, seq);
	}
#endif

	return 0;
} 


static void
rtsp_client_kill(network_client *c)
{
	rtsp_client *rc = (rtsp_client*)c;

	if (rc->ops && rc->ops->kill)
	{
		(*rc->ops->kill)(rc);
	}
}


static void
rtsp_client_conn_closed(network_client *c)
{
	rtsp_client *rc = (rtsp_client*)c;

	if (rc->ops && rc->ops->on_closed)
	{
		(*rc->ops->on_closed)(rc);
	}
}


static void
rtsp_client_conn_error(network_client *c, int32_t err)
{
	rtsp_client *rc = (rtsp_client*)c;

	if (rc->ops && rc->ops->on_error)
	{
		(*rc->ops->on_error)(rc, err);
	}
}


static network_client_ops rtsp_client_ops_impl =
{
	.init					= rtsp_client_init,
	.fin					= rtsp_client_finalize,
	.create_session 		= rtsp_client_create_session,
	.msg_recv				= rtsp_client_handle_msg,
	.msg_sent				= rtsp_client_msg_sent,
	.kill					= rtsp_client_kill,
	.closed					= rtsp_client_conn_closed,
	.error					= rtsp_client_conn_error
};


rtsp_client *rtsp_client_new(uint32_t size, rtsp_client_ops *ops,
	uint32_t factory, void *io)
{
	int32_t err;
	rtsp_client *rc;

	if (size < sizeof(rtsp_client))
	{
		unix_sock_close((int32_t)io);
		return NULL;
	}

	rc = (rtsp_client*)network_client_new(
		size, &rtsp_client_ops_impl, factory, io);
	if (rc)
	{
		if (ops && ops->init)
		{
			err = (*ops->init)(rc);
			if (err)
			{
				network_client_kill_unref((network_client*)rc);
				return NULL;
			}
		}

		rc->ops = ops;
	}

	return rc;
}


int32_t
rtsp_client_send_msg(rtsp_client *rc, msg *res, uint32_t seq)
{
	int32_t err;
	network_client *nc = (network_client*)rc;

	err = network_client_send_msg(nc, res, seq, 0);
	if (err < 0)
	{
		LOG_W(
			"rtsp_client_send_msg()->network_client_send_msg() failed, "
			"err:'%d'.", err);
	}

	return err;
}


//:~ End
