#include "tr_factory.h"
#include "listener_template.h"
#include "rtsp_parser.h"
#include "rtp_parser.h"
#include "rtsp_impl.h"
#include "ldl_src.h"
#include "ports.h"
#include "rtp_msg.h"
#include "frame_filter.h"
#include "ldh_src.h"
#include "tr_log.h"
#include "rtsp_def.h"

#define N_LOOPS				1
#define RTSP_RB_SIZE		(4096*2)
#define FACTORY_0_PUSDK		0
#define LDH_THREADS			1
#define LDL_THREADS			1


static client *
create_new_rtsp_client(listener_tmpl *lt, int32_t sock)
{
	client *cli;

	cli = rtsp_impl_client_new(FACTORY_0_PUSDK, (void*)sock);
	if (!cli)
	{
		LOG_W(
			"Create rtsp client failed, sock:'%d'.", sock
		);
	}
	else
	{
		LOG_I(
			"New rtsp client '%p', sock:'%d'.", cli, sock
		);
	}

	return cli;
}


static listener_tmpl_ops rtsp_client_listener_ops =
{
	.new_cli	= create_new_rtsp_client
};


extern int32_t jpf_facility_init( void );

static int32_t
init_rtsp_server_factory(tr_factory *tr_f)
{
	register_rtsp_msg();
	register_rtp_msg();
	tr_ports_init();
	tr_ports_set_default_range();
	ldl_facility_init(LDL_THREADS);
	ldh_facility_init(LDH_THREADS);
	jpf_facility_init();
	return 0;
}


static listener *
create_rtsp_listener(tr_factory *tr_f)
{
	return alloc_listener_tmpl(&rtsp_client_listener_ops);
}


static JLoopScher *
create_scheduler(tr_factory *tr_f)
{
	return j_sched_new(N_LOOPS);
}


static proto_parser *
create_rtsp_proto_parser(tr_factory *tr_f)
{
	return alloc_rtsp_parser(RTSP_RB_SIZE);
}


static void
destroy_rtsp_proto_parser(tr_factory *tr_f, proto_parser *p)
{
	free_rtsp_parser(p);
}


static proto_parser *
create_rtp_proto_parser(tr_factory *tr_f)
{
	return alloc_rtp_parser();
}


static void
destroy_rtp_proto_parser(tr_factory *tr_f, proto_parser *p)
{
	free_rtp_parser(p);
}


static media_src *
create_local_media_src(tr_factory *tr_f, int32_t m_type)
{
	ld_src *src;

	if (m_type == 0)
	{
		src = (ld_src*)ldl_src_alloc(NULL);
		if (!src)
		{
			LOG_W("Create local hitory media src failed.");
		}
	}
	else
	{
		src = (ld_src*)ldh_src_alloc(NULL);
		if (!src)
		{
			LOG_W("Create local hitory media src failed.");
		}
		else
		{
			if (m_type == MS_DOWNLD)
			{
				ldh_src_set_download_mode((ldh_src*)src);
			}
		}
	}
	return (media_src*)src;
}


static media_filter *
create_h264_frame_filter(tr_factory *factory)
{
	return frame_filter_new();
}


tr_factory rtsp_server_factory =
{
	.name							= __u8_str("#pusdk lib factory"),
	.id								= FACTORY_0_PUSDK,
	.init_factory					= init_rtsp_server_factory,
	.create_listener				= create_rtsp_listener,
	.create_scheduler				= create_scheduler,
	.create_client_proto_parser		= create_rtsp_proto_parser,
	.destroy_client_proto_parser	= destroy_rtsp_proto_parser,
	.create_sinker_proto_parser		= create_rtp_proto_parser,
	.destroy_sinker_proto_parser	= destroy_rtp_proto_parser,
	.create_media_src				= create_local_media_src,
	.create_msfilter				= create_h264_frame_filter
};


//:~ End
