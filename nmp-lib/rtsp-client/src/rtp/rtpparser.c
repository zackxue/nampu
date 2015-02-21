/* *
 * This file is part of rtsp-client lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#include <string.h>
#include "rtpparser.h"

extern RTP_Parser_Template rtp_fake_parser_template;

static RTP_Parser_Template *template_array[] = 
{
	&rtp_fake_parser_template,
	NULL
};

static __inline__ gint
is_correct_parser_tpl(RTP_Parser_Template *tpl, const gchar *mime)
{
	gint i = 0;

	for (; tpl->mime[i]; ++i)
	{
		if (!strcmp(tpl->mime[i], mime))
			return 1;
	}

	return 0;
}


RTP_Parser *rtp_parser_create(gchar *mime)
{
	RTP_Parser_Template **ptl;
	RTP_Parser *parser = NULL;

	for (ptl = template_array; *ptl; ++ptl)
	{
		if (is_correct_parser_tpl(*ptl, mime))
			break;
	}

	if (*ptl)
	{
		parser = g_new0(RTP_Parser, 1);
		parser->initialize = (*ptl)->initialize;
		parser->parse = (*ptl)->parse;
		parser->finalize = (*ptl)->finalize;
	}

	return parser;
}

void rtp_parser_release(RTP_Parser *parser)
{
	g_free(parser);
}

//:~ End
