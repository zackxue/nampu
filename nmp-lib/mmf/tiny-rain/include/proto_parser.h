/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_PROTO_PARSER_H__
#define __TINY_RAIN_PROTO_PARSER_H__

#include "msg.h"

BEGIN_NAMESPACE

typedef struct __proto_parser proto_parser;
typedef struct __proto_parser_ops proto_parser_ops;

struct __proto_parser
{
	obj	obj_base;

	proto_parser_ops *ops;
	void *private;
};


struct __proto_parser_ops
{
	int32_t (*init)(proto_parser *p, void *u);
	void	(*fin)(proto_parser *p);

	msg *(*parse)(proto_parser *p, uint8_t *data, uint32_t len, int32_t *err);
	msg *(*parse_io)(proto_parser *p, void *io, int32_t *err);
};


proto_parser *alloc_proto_parser(uint32_t size, proto_parser_ops *ops, void *u);
void free_proto_parser(proto_parser *parser);

msg *parse_proto(proto_parser *p, uint8_t *data, uint32_t len, int32_t *err);
msg *parse_proto_io(proto_parser *p, void *io, int32_t *err);

END_NAMESPACE

#endif	//__TINY_RAIN_PROTO_PARSER_H__
