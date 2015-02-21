/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_FACTORY_H__
#define __TINY_RAIN_FACTORY_H__

#include "client_set.h"
#include "listener.h"
#include "proto_parser.h"
#include "media_src.h"
#include "media_filter.h"

BEGIN_NAMESPACE

typedef struct __tr_factory tr_factory;
struct __tr_factory
{
	uint8_t *name;	//@{factory name}
	uint32_t id;	//@{factory id}

	int32_t (*init_factory)(tr_factory *tr_f);

	listener *(*create_listener)(tr_factory *tr_f);
	void (*destroy_listener)(tr_factory *tr_f, listener *l);

	JLoopScher *(*create_scheduler)(tr_factory *tr_f);
	void (*destroy_scheduler)(tr_factory *tr_f, JLoopScher *sched);

	proto_parser *(*create_client_proto_parser)(tr_factory *tr_f);
	void (*destroy_client_proto_parser)(tr_factory *tr_f, proto_parser *p);

	proto_parser *(*create_sinker_proto_parser)(tr_factory *tr_f);
	void (*destroy_sinker_proto_parser)(tr_factory *tr_f, proto_parser *p);

	media_src *(*create_media_src)(tr_factory *tr_f, int32_t m_type);
	media_filter *(*create_msfilter)(tr_factory *factory);
};

//{@get a factory object by id}
tr_factory *get_tr_factory(uint32_t factory_id);

//@{init factory}
int32_t init_factory(tr_factory *factory);

//@{factory create/destroy sinker client listener}
listener *factroy_create_sclr(tr_factory *factory);
void factory_destroy_sclr(tr_factory *factory, listener *l);

//@{factory create/destroy event scheduler}
JLoopScher *factory_create_scheduler(tr_factory *factory);
void factory_destroy_scheduler(tr_factory *factory, JLoopScher *sched);

//@{factory create/destroy sinker client proto parser}
proto_parser *factory_create_scpp(tr_factory *factory);
void factory_destroy_scpp(tr_factory *factory, proto_parser *p);

//@{factory create/destroy sinker proto parser}
proto_parser *factory_create_spp(tr_factory *factory);
void factory_destroy_spp(tr_factory *factory, proto_parser *p);

//@{factory create media source}
media_src *factory_create_msrc(tr_factory *factory, int32_t m_type);

//@{factory create media src filter}
media_filter *factory_create_msfilter(tr_factory *factory);

END_NAMESPACE

#endif	//__TINY_RAIN_FACTORY_H__
