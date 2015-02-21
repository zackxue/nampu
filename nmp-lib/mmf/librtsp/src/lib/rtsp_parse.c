
#include <string.h>
#include <assert.h>

#include "rtsp_parse.h"
#include "rtsp_ctx.h"
#include "rtsp_msg.h"
#include "rtsp_defs.h"


rtsp_parser *rtsp_parser_new(size_t rb_size)
{
	rtsp_parser *parser;

	parser = (rtsp_parser *)my_alloc(sizeof(rtsp_parser));
	memset(parser, 0, sizeof(rtsp_parser));

	parser->ctx = rtsp_ctx_new(rb_size);
	build_init(&parser->builder);
	return parser;
}


void rtsp_parse_free(rtsp_parser *parser)
{
	assert(parser != NULL);

	rtsp_message_unset(&parser->message);
	build_reset(&parser->builder);
	rtsp_ctx_free(parser->ctx);
	my_free(parser, sizeof(rtsp_parser));
}


RTSP_RESULT rtsp_parser_recv(rtsp_parser *parser, int fd, rtsp_message **msg, 
	int *err)
{
	RTSP_RESULT res;

	if (parser->msg_complete)
	{
		build_reset(&parser->builder);
		rtsp_message_unset(&parser->message);
		parser->msg_complete = 0;
	}

	res = rtsp_ctx_recv(parser->ctx, fd, &parser->builder, 
		&parser->message, err);
	if (res == RTSP_OK)
	{
		if (msg)
			*msg = &parser->message;
		parser->msg_complete = 1;
	}

	return res;
}

