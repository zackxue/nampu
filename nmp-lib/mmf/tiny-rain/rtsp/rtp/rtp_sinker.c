#include "rtp_sinker.h"
#include "rtsp_def.h"
#include "unix_sock.h"
#include "rtp_msg.h"
#include "mem_block.h"
#include "tr_log.h"


static int32_t
rtp_sinker_init(network_sinker *ns, sinker_param *sp)
{
	rtp_sinker *rs = (rtp_sinker*)ns;

	rs->trans_mode = RTP_TRANS_INVALID;
	rs->interleaved = NULL;
	return 0;
}


static void
rtp_sinker_fin(network_sinker *ns)
{
	rtp_sinker *rs = (rtp_sinker*)ns;

	if (rs->interleaved)
	{
		client_unref((client*)rs->interleaved);
		rs->interleaved = NULL;
	}
}


static void
fin_rtp_msg_interleaved_mb(mem_block *mb)
{
	rtp_mem_t *rm = (rtp_mem_t*)mb->u;

	(*rm->unref)(rm);
}


static __inline__ mem_block *
rtp_msg_to_interleaved_mb(msg *m, int8_t il_ch)
{
	uint8_t *il_header;
	void *ptr;
	uint32_t size;
	mem_block *mb = NULL;
	rtp_mem_t* rm = rtp_msg_to_rtp_mem_s(m);

	if (rm)
	{
		ptr = (*rm->pdata)(rm, &size);
		if (ptr)
		{
			BUG_ON(size > RTP_MTU);
			mb = alloc_memblock();
			BUG_ON(!mb);
			mb->ptr = ptr - 4;
			mb->size = size + 4;
			mb->b_size = mb->size;
			mb->offset = 0;
			mb->flags = MEMBLOCK_FLGS_DROPABLE;
			mb->u = rm;
			mb->finalize = fin_rtp_msg_interleaved_mb;

			il_header = (uint8_t*)mb->ptr;
			il_header[0] = '$';
			il_header[1] = il_ch;
			il_header[2] = (size & 0xff00) >> 8;
			il_header[3] = (size & 0xff);
		}
	}

	return mb;
}


static int32_t
rtp_sinker_consumable(network_sinker *ns, proto_watch *pw,
	int32_t stm_i, uint32_t size)
{
	int32_t err = -EINVAL;
	media_sinker *sinker = (media_sinker*)ns;
	rtp_sinker *rs = (rtp_sinker*)ns;
	network_client *nc = (network_client*)rs->interleaved;

	if (rs->trans_mode == RTP_OVER_RTSP && nc)
	{//@{Interleaved rtp mode}
		client_ref((client*)nc);
		RELEASE_LOCK(sinker->lock);

		err = network_client_consumable(nc, size);
		client_unref((client*)nc);

		AQUIRE_LOCK(sinker->lock);
		return err;
	}

	if (pw)
	{
		return proto_watch_writeable(pw, size);
	}

	return err;
}


static int32_t
rtp_sinker_consume(network_sinker *ns, proto_watch *pw, 
	int32_t stm_i, msg *m, uint32_t seq)
{
	int32_t ret = -ENOMEM;
	media_sinker *sinker = (media_sinker*)ns;
	rtp_sinker *rs = (rtp_sinker*)ns;
	media_sinker *ms = (media_sinker*)ns;
	mem_block *mb;
	network_client *nc = (network_client*)rs->interleaved;

	if (rs->trans_mode == RTP_OVER_UDP)
	{
		BUG_ON(!pw);
		proto_watch_ref(pw);
		RELEASE_LOCK(sinker->lock);

		msg_ref(m);
		ret = proto_watch_write(pw, m, seq, 0);
		proto_watch_unref(pw);
	
		AQUIRE_LOCK(sinker->lock);
		return ret;
	}

	mb = rtp_msg_to_interleaved_mb(m, ns->stms[stm_i].interleaved);
	if (mb)
	{
		MEMBLOCK_SET_SEQ(mb, seq);
		if (rs->trans_mode == RTP_OVER_TCP)
		{
			BUG_ON(!pw);
			proto_watch_ref(pw);
			RELEASE_LOCK(sinker->lock);

			msg_ref(m);
			ret = proto_watch_write_mb(pw, mb, 0);
			proto_watch_unref(pw);
	
			AQUIRE_LOCK(sinker->lock);			
		}
		else
		{
			BUG_ON(!nc);
			RELEASE_LOCK(ms->lock);
	
			msg_ref(m);
			ret = network_client_send_mb(nc, mb, 0);
	
			AQUIRE_LOCK(ms->lock);
		}
	}

	return ret;
}


static int32_t
rtp_sinker_set_config(network_sinker *sinker, void *in, void *data)
{
	rtp_sinker *rs = (rtp_sinker*)sinker;
	int32_t stm_i = *((int32_t*)in);
	rtp_port_conf *rpc = (rtp_port_conf*)data;
	proto_watch *pw;

	if (!VALID_STM_TYPE(stm_i) || sinker->stms[stm_i].stm_type == ST_NONE)
		return -EINVAL;

	if (!rpc->rtsp_client)
		return -EINVAL;

	rs->trans_mode = rpc->trans;
	pw = sinker->stms[stm_i].stm_watch;
	if (!pw)
	{
		if (rpc->trans != RTP_OVER_RTSP)
			return -EINVAL;

		if (!rs->interleaved)
		{
			rs->interleaved = (rtsp_client*)client_ref((client*)rpc->rtsp_client);
		}

		return 0;
	}

	if (!proto_watch_attach(pw, ((client*)rpc->rtsp_client)->sched))
		return proto_watch_set_dst(pw, rpc->host, rpc->ports.min_port);

	return -EINVAL;
}


static int32_t
rtp_sinker_get_config(network_sinker *sinker, void *in, void *data)
{
	rtp_sinker *rs = (rtp_sinker*)sinker;
	int32_t stm_i = *((int32_t*)in);
	rtp_port_conf *rpc = (rtp_port_conf*)data;

	if (!VALID_STM_TYPE(stm_i))
		return -EINVAL;

	if (sinker->stms[stm_i].stm_type == ST_NONE)
		return -EINVAL;

	rpc->trans = rs->trans_mode;
	rpc->ports.min_port = sinker->stms[stm_i].port;
	rpc->ports.max_port = rpc->ports.min_port + 1;

	return 0;
}


static network_sinker_ops rtp_sinker_ops =
{
	.init		 = rtp_sinker_init,
	.fin		 = rtp_sinker_fin,
	.consumable  = rtp_sinker_consumable,
	.consume	 = rtp_sinker_consume,
	.set_config	 = rtp_sinker_set_config,
	.get_config  = rtp_sinker_get_config
};


rtp_sinker *
rtp_sinker_new(sinker_param *sp)
{
	return (rtp_sinker*)network_sinker_alloc(sizeof(rtp_sinker),
		&rtp_sinker_ops, sp);
}


//:~ End
