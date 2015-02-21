#include "rtp_parser.h"


static int32_t
rtp_parser_init(proto_parser *p, void *u)
{
	return 0;
}


static void
rtp_parser_finalize(proto_parser *p)
{
}


static msg *
rtp_parser_parse(proto_parser *p, uint8_t *data, uint32_t len, int32_t *err)
{
	return NULL;
}


static msg *
rtp_parser_parse_io(proto_parser *p, void *io, int32_t *err)
{
	return NULL;
}


static proto_parser_ops rtp_parser_impl_ops =
{
	.init	= rtp_parser_init,
	.fin	= rtp_parser_finalize,
	.parse	= rtp_parser_parse,
	.parse_io = rtp_parser_parse_io
};


proto_parser *alloc_rtp_parser( void )
{
	return alloc_proto_parser(sizeof(proto_parser),
		&rtp_parser_impl_ops, NULL);
}


void free_rtp_parser(proto_parser *parser)
{
	free_proto_parser(parser);
}


//:~ End
