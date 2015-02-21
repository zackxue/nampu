#include "rtp_msg.h"
#include "h264_filter.h"


static msg *
rtp_msg_ref(msg *m)
{
	rtp_mem_t *rm = rtp_msg_to_rtp_mem_s(m);
	(*rm->ref)(rm);
	return m;
}


static void
rtp_msg_unref(msg *m)
{
	rtp_mem_t *rm = rtp_msg_to_rtp_mem_s(m);
	(*rm->unref)(rm);
}


static msg *
rtp_msg_dup(msg *m)
{
	BUG();
	return NULL;
}


static uint32_t
rtp_msg_size(msg *m)
{
	return 0;
}


static void
fin_rtp_msg_mb(mem_block *mb)
{
	rtp_mem_t *rm = (rtp_mem_t*)mb->u;

	(*rm->unref)(rm);
}


static mem_block *
rtp_msg_to_mb(msg *m)
{
	mem_block *mb = NULL;
	void *ptr;
	uint32_t size;
	rtp_mem_t *rm = rtp_msg_to_rtp_mem_s(m);

	if (rm)
	{
		ptr = (*rm->pdata)(rm, &size);
		if (ptr)
		{
			BUG_ON(size > RTP_MTU);
			mb = alloc_memblock();
			BUG_ON(!mb);
			mb->ptr = ptr;
			mb->size = size;
			mb->b_size = size;
			mb->offset = 0;
			mb->flags = MEMBLOCK_FLGS_DROPABLE;
			mb->u = rm;
			mb->finalize = fin_rtp_msg_mb;
		}
	}

	return mb;
}


static msg_ops rtp_msg_ops =
{
	.ref 		= rtp_msg_ref,
	.unref		= rtp_msg_unref,
	.dup		= rtp_msg_dup,
	.msg_size	= rtp_msg_size,
	.msg_to_mb	= rtp_msg_to_mb
};


int32_t register_rtp_msg( void )
{
	return register_msg_type(RTP_MSG_MT, &rtp_msg_ops);
}


//:~ End
