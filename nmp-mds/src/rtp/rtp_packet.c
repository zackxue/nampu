
#include "rtp_packet.h"
#include "nmp_debug.h"

gboolean
check_rtp_packet(gchar *packet, gint pack_len)
{
	rtp_header *rh;
	G_ASSERT(packet != NULL && pack_len >=0);

	rh = (rtp_header*)packet;
	return rh->version >= 0x04 && pack_len <= RTP_MTU;
}


guint8
get_rtp_csrc_count(gchar *packet)
{
	G_ASSERT(packet != NULL);

	return ((rtp_header*)packet)->csrc_count;
}


guint8
get_rtp_extension_bit(gchar *packet)
{
	G_ASSERT(packet != NULL);

	return ((rtp_header*)packet)->extension;
}


guint8
get_rtp_payload_type(gchar *packet)
{
	G_ASSERT(packet != NULL);

	return ((rtp_header*)packet)->payload_type;	
}


guint16
get_rtp_seq(gchar *packet)
{
	G_ASSERT(packet != NULL);

	return g_ntohs(((rtp_header*)packet)->seq);		
}


guint32
get_rtp_timestamp(gchar *packet)
{
	G_ASSERT(packet != NULL);

	return g_ntohl(((rtp_header*)packet)->timestamp);		
}


guint32
get_rtp_ssrc(gchar *packet)
{
	G_ASSERT(packet != NULL);

	return g_ntohl(((rtp_header*)packet)->ssrc);
}


gpointer
get_rtp_payload(gchar *packet)
{
	G_ASSERT(packet != NULL);

	return packet + RTP_HEADER_LEN + 
	get_rtp_ssrc(packet) * sizeof(guint32);
}


//:~ End
