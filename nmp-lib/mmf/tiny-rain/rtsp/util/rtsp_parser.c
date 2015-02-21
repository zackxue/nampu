#include "tr_log.h"
#include "alloc.h"
#include "rtsp_parser.h"
#include "rtsp_parse.h"

#define RTSP_MSG_BUF	(4096*2)


static msg *
rtsp_msg_ref(msg *m)
{
	BUG();
	return NULL;
}


static void
rtsp_msg_unref(msg *m)
{
	rtsp_message *rm;
	msg *_m = GET_MSG_ADDR(m);
	BUG_ON(_m == m);

	rm = (rtsp_message*)_m;
	rtsp_message_free(rm);
}


static msg *
rtsp_msg_dup(msg *m)
{
	BUG();
	return NULL;
}


static uint32_t
rtsp_msg_size(msg *m)
{
	return sizeof(rtsp_message);
}


static void
fin_rtsp_msg_mb(mem_block *mb)
{
	tr_free(mb->ptr, mb->b_size);
}


static mem_block *
rtsp_msg_to_mb(msg *m)
{
	mem_block *mb;
	rtsp_message *rm;
	RTSP_MSG_TYPE mt;
	int32_t ret;

	msg *_m = GET_MSG_ADDR(m);
	BUG_ON(_m == m);
	rm = (rtsp_message*)_m;

	mt = rtsp_message_get_type(rm);
	if (mt != RTSP_MESSAGE_DATA)
	{
		uint8_t *ptr = tr_alloc(RTSP_MSG_BUF);
		mb = alloc_memblock();
		BUG_ON(!mb);
		BUG_ON(!ptr);
		mb->ptr = ptr;
		mb->b_size = RTSP_MSG_BUF;
		ret = rtsp_message_to_string(rm, (char*)ptr, RTSP_MSG_BUF);
		if (ret < 0 || ret >= RTSP_MSG_BUF)
			ret = 0;
		mb->offset = 0;
		mb->size = ret;
		mb->flags = 0;
		mb->finalize = fin_rtsp_msg_mb;
		rtsp_message_free(rm);
		return mb;
	}
	else
	{
		return NULL;
	}
}


static msg_ops rtsp_msg_ops =
{
	.ref 		= rtsp_msg_ref,
	.unref		= rtsp_msg_unref,
	.dup		= rtsp_msg_dup,
	.msg_size	= rtsp_msg_size,
	.msg_to_mb	= rtsp_msg_to_mb
};


int32_t
register_rtsp_msg()
{
	return register_msg_type(RTSP_MSG_MT, &rtsp_msg_ops);
}


static int32_t
rtsp_parser_init(proto_parser *p, void *u)
{
	uint32_t rb_size = (uint32_t)u;

	p->private = rtsp_parser_new(rb_size);
	if (!p->private)
	{
		LOG_W("rtsp_parser_init()->rtsp_parser_new() failed.");
		return -ENOMEM;
	}

	return 0;
}


static void
rtsp_parser_finalize(proto_parser *p)
{
	rtsp_parser *parser = (rtsp_parser*)p->private;

	if (parser)
	{
		LOG_V("rtsp_parse_free() ok.");
		rtsp_parse_free(parser);
	}
}


static msg *
rtsp_parser_parse(proto_parser *p, uint8_t *data, uint32_t len,
	int32_t *perr)
{
	if (perr)
	{
		*perr = -EPERM;
	}

	return NULL;
}


static msg *
rtsp_parser_parse_io(proto_parser *p, void *io, int32_t *perr)
{
	int32_t re, err = -EINVAL;
	msg *m = NULL;
	rtsp_parser *parser = (rtsp_parser*)p->private;
	RTSP_RESULT res;

	if (!parser)
	{
		goto parse_out;
	}

	res = rtsp_parser_recv(parser, (int)io, (rtsp_message**)&m, &re);
	if (res == RTSP_OK)
	{
		err = 0;
		m = SET_MSG_TYPE(m, RTSP_MSG_MT);
		goto parse_out;
	}

	if (res == RTSP_EINTR)
	{
		err = -EAGAIN;
		m = NULL;
		goto parse_out;
	}

	if (res == RTSP_EEOF)
	{
		err = -ECONNRESET;
		m = NULL;
		goto parse_out;
	}

	err = re;
	m = NULL;

parse_out:
	if (perr)
	{
		*perr = err;
	}
	return m;
}


static proto_parser_ops rtsp_parser_impl_ops =
{
	.init		= rtsp_parser_init,
	.fin		= rtsp_parser_finalize,
	.parse		= rtsp_parser_parse,
	.parse_io 	= rtsp_parser_parse_io
};


proto_parser *alloc_rtsp_parser(uint32_t rb_size)
{
	return alloc_proto_parser(sizeof(proto_parser),
		&rtsp_parser_impl_ops, (void*)rb_size);
}


void free_rtsp_parser(proto_parser *parser)
{
	free_proto_parser(parser);
}


//:~ End
