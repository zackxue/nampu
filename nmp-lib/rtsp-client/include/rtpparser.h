/* *
 * This file is part of rtsp-client lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#ifndef __RTP_PARSER_H__
#define __RTP_PARSER_H__

#include "media.h"

#define RTP_PKT_OK				0
#define RTP_PKT_UNKNOWN			1
#define RTP_PKT_AGAIN			2
#define RTP_ERRALLOC			3
#define RTP_EPARSE				4

#define RTP_PARSER_COMMON	\
	ParserInit	initialize; \
	ParserRtp	parse; \
	ParserFin	finalize;

typedef gint (*ParserInit)(RTP_Session *rtp_s, RTP_Parser *parser, gint pt);	/* rtp payload type*/
typedef gint (*ParserRtp)(RTP_Session *rtp_s, gchar *rtp, gint size, void *frame);
typedef gint (*ParserFin)(RTP_Session *rtp_s);

struct _RTP_Parser		/* for rtp packets parsing */
{
	RTP_PARSER_COMMON
	void	*private;
};

typedef struct _RTP_Parser_Template RTP_Parser_Template;
struct _RTP_Parser_Template
{
	gchar	**mime;

	RTP_PARSER_COMMON
};

RTP_Parser *rtp_parser_create(gchar *mime);
void rtp_parser_release(RTP_Parser *parser);


#endif	/* __RTP_PARSER_H__ */

