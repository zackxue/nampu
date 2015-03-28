#include "nmp_ring_buffer.h"
#include "rtp_packet.h"
#include "nmp_rtp_buffer.h"
#include "nmp_errno.h"
#include "nmp_debug.h"

#define RTP_WRAP_MAGIC			0xE1E2E3E4

gint
nmp_rtp_check(b_ptr_t start, gsize size)
{
#ifndef NDEBUG
	NmpRtpWrap *rtp = (NmpRtpWrap*)start;

	if (G_UNLIKELY(size <= sizeof(NmpRtpWrap)))
		return -E_PACKET;
	rtp->magic =  RTP_WRAP_MAGIC;
	BUG_ON(size != rtp->length + sizeof(NmpRtpWrap));
#endif

	return 0;
}
int lost;

gint
nmp_rtp_count(b_ptr_t start, b_ptr_t end, gsize *lp, gsize *lb)
{
	NmpRtpWrap *rtp;

	*lp = 0;
	*lb = 0;

	while (start < end)
	{
		rtp = (NmpRtpWrap*)start;
				
#ifndef NDEBUG
		if (G_UNLIKELY(rtp->magic != RTP_WRAP_MAGIC))
			return -E_PACKET;
#endif

		*lp += 1;
		*lb += rtp->length;

		start += sizeof(NmpRtpWrap);
		start += rtp->length;
	}

	return 0;
}


gint
nmp_rtp_drop(b_ptr_t point, b_ptr_t upper, b_ptr_t *adjust,
	gsize *lp, gsize *lb)
{
	NmpRtpWrap *rtp;

	*lp = 0;
	*lb = 0;

	while (*adjust < upper)
	{
		rtp = (NmpRtpWrap*)*adjust;

		if ((b_ptr_t)rtp > point)
			break;

#ifndef NDEBUG
		if (G_UNLIKELY(rtp->magic != RTP_WRAP_MAGIC)) {printf("-------------------------rtp:%p, point:%p\n", rtp, point);
		return -E_PACKET;}
#endif

		*lp += 1;
		*lb += rtp->length;	printf("==> y lost one %d\n" ,++lost);

		*adjust += sizeof(NmpRtpWrap);
		*adjust += rtp->length;
	}

	return 0;
}

gsize
nmp_rtp_get_packet(b_ptr_t start, b_ptr_t end)
{
	NmpRtpWrap *rtp;
	gint buf_size, packet_size;

	buf_size = end - start;
	if (G_UNLIKELY(buf_size < sizeof(NmpRtpWrap)))
		return -E_PACKET;

	rtp = (NmpRtpWrap*)start;
	packet_size = rtp->length + sizeof(NmpRtpWrap);

	if (G_UNLIKELY(buf_size < packet_size))
		return -E_PACKET;

	return packet_size;
}


NmpRingBufferOps rtp_buffer_ops =
{
	.check = nmp_rtp_check,
	.count = nmp_rtp_count,
	.drop = nmp_rtp_drop,
	.packet = nmp_rtp_get_packet
};


//:~ End
