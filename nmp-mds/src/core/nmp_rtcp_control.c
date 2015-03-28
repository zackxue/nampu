#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_rtcp_control.h"
#include "rtp_packet.h"


void
nmp_rtcp_control_bye(NmpRtcpControl *ctl_block);

gint
nmp_rtcp_control_recv(NmpRtcpControl *ctl_block, gint tcp)
{
	struct sockaddr_in sin;
	socklen_t len;
	gint size, err;
	rtcp_rr_header rr;

	G_ASSERT(ctl_block != NULL);

	if (tcp)	/* 暂不支持 */
		return -1;

	len = sizeof(sin);

	while ((size = recvfrom(ctl_block->rtcp_recv.fd, 
		ctl_block->rtcp_buffer.buffer,
		SMART_BUFFER_SIZE, 0, (struct sockaddr*)&sin, &len)) > 0)
	{
		if (!ctl_block->rtcp_connected)
		{
			if (!connect(ctl_block->rtcp_send.fd, (struct sockaddr*)&sin,
				len))
			{
				ctl_block->rtcp_connected = TRUE;
			}
		}

		//'SR'包
//		dump_rtcp_packet(ctl_block->rtcp_buffer.buffer, size);

		init_rtcp_rr_packet(&rr);
		set_rtcp_rr_src(&rr, (guint32)ctl_block);
		set_rtcp_rr_ssrc(&rr, get_rtcp_sr_src(ctl_block->rtcp_buffer.buffer));
		set_rtcp_rr_lastseq(&rr, ctl_block->last_rtp_seq);
		set_rtcp_rr_lsr(&rr, get_rtcp_sr_lsr(ctl_block->rtcp_buffer.buffer));
//		set_rtcp_rr_lost(&rr, 1, 10);
//		set_rtcp_rr_dlsr(&rr, 0);

		//'RR' 包
//		dump_rtcp_packet(&rr, size);
		sendto(ctl_block->rtcp_send.fd, &rr, sizeof(rr), 0, (struct sockaddr*)&sin, len);
	}

	err = errno;
	if (!size || err != EAGAIN)
		return -err;

	return 0;
}


void
nmp_rtcp_control_bye(NmpRtcpControl *ctl_block)
{
	rtcp_bye_header br;

	if (ctl_block->rtcp_connected)
	{
		init_rtcp_bye_packet(&br);				/* Fix me */
		set_rtcp_bye_src(&br, (guint32)ctl_block);		
//		dump_rtcp_packet(&br, sizeof(br));
		sendto(ctl_block->rtcp_send.fd, &br, sizeof(br), 0, NULL, 0);
	}
}


gint
nmp_rtcp_control_update(NmpRtcpControl *ctl_block,
	gpointer rtp_packet, gsize size)
{
	ctl_block->rtp_ssrc = get_rtp_ssrc(rtp_packet);
	ctl_block->last_rtp_seq = get_rtp_seq(rtp_packet);
	return 0;
}


//:~ End
