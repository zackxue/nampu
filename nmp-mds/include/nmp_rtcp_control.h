#ifndef __NMP_RTCP_CONTROL_H__
#define __NMP_RTCP_CONTROL_H__

#include <glib.h>
#include "nmp_smart_buffer.h"
#include "rtcp_packet.h"

typedef struct _JpfRtcpControl JpfRtcpControl;
struct _JpfRtcpControl
{
	GPollFD				rtcp_recv;			/* for rtcp recving */
	GPollFD				rtcp_send;			/* for rtcp sending */
	gboolean			rtcp_connected;		/* link connected ? */

	JpfSmartBuffer		rtcp_buffer;

	guint32	rtp_ssrc;	/* rtp synchronization source */
	guint16 last_rtp_seq;
};

gint nmp_rtcp_control_recv(JpfRtcpControl *ctl_block, gint tcp);
void nmp_rtcp_control_bye(JpfRtcpControl *ctl_block);

gint nmp_rtcp_control_update(JpfRtcpControl *ctl_block, 
	gpointer rtp_packet, gsize size);

#endif	//__NMP_RTCP_CONTROL_H__
