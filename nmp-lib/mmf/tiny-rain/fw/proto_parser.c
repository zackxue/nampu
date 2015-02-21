#include <stdio.h>
#include "proto_parser.h"


static __inline__ void
proto_parser_finalize(proto_parser *parser)
{//@{Empty}
}


static __inline__ void
on_obj_fin(obj *p)
{
	proto_parser *parser = (proto_parser*)p;

	if (parser->ops && parser->ops->fin)
	{
		(*parser->ops->fin)(parser);
	}

	proto_parser_finalize(parser);
}


proto_parser *
alloc_proto_parser(uint32_t size, proto_parser_ops *ops, void *u)
{
	proto_parser *parser;
	int32_t err;

	if (size < sizeof(*parser))
		return NULL;

	parser = (proto_parser*)obj_new(size, on_obj_fin);
	parser->ops = NULL;

	if (ops && ops->init)
	{
		err = (*ops->init)(parser, u);
		if (err)
		{
			obj_unref(parser);
			return NULL;
		}
	}

	parser->ops = ops;
	return parser;
}


void free_proto_parser(proto_parser *parser)
{
	obj_unref(parser);
}


msg *parse_proto(proto_parser *p, uint8_t *data, uint32_t len,
	int32_t *err)
{
	msg *m = NULL;

	if (p->ops && p->ops->parse)
	{
		m = (*p->ops->parse)(p, data, len, err);
	}

	return m;
}


msg *parse_proto_io(proto_parser *p, void *io, int32_t *err)
{
	msg *m = NULL;

	if (p->ops && p->ops->parse_io)
	{
		m = (*p->ops->parse_io)(p, io, err);
	}

	return m;
}


//:~ End
