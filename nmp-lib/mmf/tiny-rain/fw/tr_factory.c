#include "tr_factory.h"

extern tr_factory rtsp_server_factory;


int32_t
init_factory(tr_factory *factory)
{
	if (!factory->init_factory)
		return 0;

	return (*factory->init_factory)(factory);
}


listener *
factroy_create_sclr(tr_factory *factory)
{
	if (!factory->create_listener)
		return NULL;
	return (*factory->create_listener)(factory);
}


void
factory_destroy_sclr(tr_factory *factory, listener *l)
{
	if (!factory->destroy_listener)
		BUG();
	(*factory->destroy_listener)(factory, l);
}


JLoopScher *
factory_create_scheduler(tr_factory *factory)
{
	if (!factory->create_scheduler)
		return NULL;
	return (*factory->create_scheduler)(factory);
}


void
factory_destroy_scheduler(tr_factory *factory, JLoopScher *sched)
{
	if (!factory->destroy_scheduler)
		BUG();
	(*factory->destroy_scheduler)(factory, sched);
}


proto_parser *
factory_create_scpp(tr_factory *factory)
{
	if (!factory->create_client_proto_parser)
		return NULL;
	return (*factory->create_client_proto_parser)(factory);
}


void
factory_destroy_scpp(tr_factory *factory, proto_parser *p)
{
	if (!factory->destroy_client_proto_parser)
		BUG();
	(*factory->destroy_client_proto_parser)(factory, p);
}



proto_parser *
factory_create_spp(tr_factory *factory)
{
	if (!factory->create_sinker_proto_parser)
		return NULL;
	return (*factory->create_sinker_proto_parser)(factory);
}


void
factory_destroy_spp(tr_factory *factory, proto_parser *p)
{
	if (!factory->destroy_sinker_proto_parser)
		BUG();
	(*factory->destroy_sinker_proto_parser)(factory, p);
}


media_src *
factory_create_msrc(tr_factory *factory, int32_t m_type)
{
	if (!factory->create_media_src)
		return NULL;
	return (*factory->create_media_src)(factory, m_type);	
}


media_filter *
factory_create_msfilter(tr_factory *factory)
{
	if (!factory->create_msfilter)
		BUG();
	return (*factory->create_msfilter)(factory);	
}


static tr_factory *factory_array[] = 
{
	&rtsp_server_factory
};


tr_factory *get_tr_factory(uint32_t factory_id)
{
	int32_t f = sizeof(factory_array)/sizeof(factory_array[0]);

	if (factory_id >= f)
		return NULL;

	return factory_array[factory_id];
}


//:~ End

