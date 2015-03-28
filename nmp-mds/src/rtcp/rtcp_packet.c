#include <string.h>
#include "rtcp_packet.h"
#include "nmp_debug.h"

#define RTCP_DUMP 	printf


static __inline__ void
pack_rtcp_comm_header(rtcp_comm_header *comm_h,
	guint8 r_blocks, guint8 padding,
	guint8 version, guint8 type, guint16 length)
{
	G_ASSERT(comm_h != NULL && !(length%4));

	comm_h->count = r_blocks;
	comm_h->padding = padding;
	comm_h->version = version;
	comm_h->packet_type = type;
	comm_h->length = g_htons(length/4 - 1);
}


guint32
get_rtcp_sr_src(gpointer sr)
{
	G_ASSERT(sr != NULL);

	return g_ntohl(((rtcp_sr_header*)sr)->ssrc);
}


guint32
get_rtcp_sr_lsr(gpointer sr)
{
	rtcp_sr_header *_sr;
	G_ASSERT(sr != NULL);

	_sr = (rtcp_sr_header*)sr;
	return g_ntohl((_sr->ntp_hi>>16)|((_sr->ntp_lo&0xffff)<<16));
}


void
init_rtcp_rr_packet(rtcp_rr_header *rh)
{
	G_ASSERT(rh != NULL);

	memset(rh, 0, sizeof(*rh));
	pack_rtcp_comm_header(
		&rh->comm_h, 1, 0, 0x02, RTCP_RR, 32
	);
}


void
set_rtcp_rr_src(rtcp_rr_header *rh, guint32 src)
{
	G_ASSERT(rh != NULL);

	rh->ssrc = g_htonl(src);
}


void
set_rtcp_rr_ssrc(rtcp_rr_header *rh, guint32 ssrc)
{
	G_ASSERT(rh != NULL);

	rh->rr_b[0].ssrc = g_htonl(ssrc);
}


void
set_rtcp_rr_lost(rtcp_rr_header *rh, guint8 lost_rate,
	guint32 lost_packets)
{
	G_ASSERT(rh != NULL);

	rh->rr_b[0].lost_rate = lost_rate;
	rh->rr_b[0].lost = g_htonl(lost_packets) >> 8;
}


void
set_rtcp_rr_jitter(rtcp_rr_header *rh, guint32 jitter)
{
	G_ASSERT(rh != NULL);

	rh->rr_b[0].jitter = g_htonl(jitter);
}


void
set_rtcp_rr_lastseq(rtcp_rr_header *rh, guint32 lastseq)
{
	G_ASSERT(rh != NULL);

	rh->rr_b[0].last_seq = g_htonl(lastseq);
}


void
set_rtcp_rr_lsr(rtcp_rr_header *rh, guint32 lsr)
{
	G_ASSERT(rh != NULL);

	rh->rr_b[0].lsr = g_htonl(lsr);
}


void
set_rtcp_rr_dlsr(rtcp_rr_header *rh, guint32 dlsr)
{
	G_ASSERT(rh != NULL);

	rh->rr_b[0].dlsr = g_htonl(dlsr * 65535);
}


void
init_rtcp_bye_packet(rtcp_bye_header *bh)
{
	G_ASSERT(bh != NULL);

	memset(bh, 0, sizeof(*bh));
	pack_rtcp_comm_header(
		&bh->comm_h, 1, 0, 0x02, RTCP_BYE, sizeof(*bh)
	);
}


void
set_rtcp_bye_src(rtcp_bye_header *bh, guint32 src)
{
	bh->src[0] = g_htonl(src);
}


void
dump_rtcp_packet(gpointer packet, gint len)
{
	rtcp_comm_header *comm_h;
	rtcp_sr_header *sr_h;
	rtcp_rr_header *rr_h;
	G_ASSERT(packet != NULL);

	if (len <= sizeof(*comm_h))
	{
		RTCP_DUMP("RTCP - PIECES");
		return;
	}

	comm_h = (rtcp_comm_header*)packet;

	switch (comm_h->packet_type)
	{
	case RTCP_SR:
		sr_h = (rtcp_sr_header*)packet;
		RTCP_DUMP("'<RTCP - SR>'\n");
		RTCP_DUMP("\t'packet_len: %d/%d'\n", g_ntohs(comm_h->length), len);
		RTCP_DUMP("\t'src: %u %u'\n", g_ntohl(sr_h->ssrc), g_ntohl(sr_h->rr_b[0].ssrc));
		RTCP_DUMP("\t'rtp_sent: %u packets'\n", g_ntohl(sr_h->pkt_sent));
		RTCP_DUMP("\t'rtp_sent: %u bytes'\n", g_ntohl(sr_h->oct_sent));
		break;

	case RTCP_RR:
		rr_h = (rtcp_rr_header*)packet;
		RTCP_DUMP("'<RTCP - RR>'\n");
		RTCP_DUMP("\t'src: %u %u'\n", g_ntohl(rr_h->ssrc), g_ntohl(rr_h->rr_b[0].ssrc));
		RTCP_DUMP("\t'lost_rate: %u lost_packets: %u'\n", rr_h->rr_b[0].lost_rate,
			g_ntohl(rr_h->rr_b[0].lost & 0x00ffffff) >> 8);
		RTCP_DUMP("\t'last_seq: %u'\n", g_ntohl(rr_h->rr_b[0].last_seq));
		break;

	case RTCP_SDES:
		RTCP_DUMP("'<RTCP - SDES>'\n");
		break;	

	case RTCP_APP:
		RTCP_DUMP("'<RTCP - APP>'\n");
		break;

	case RTCP_BYE:
		RTCP_DUMP("'<RTCP - BYE>'\n");
		break;
	
	default:
		RTCP_DUMP("'<RTCP - UNKNOWN>'\n");
		break;
	}

	RTCP_DUMP("\n");
}



//:~ End
