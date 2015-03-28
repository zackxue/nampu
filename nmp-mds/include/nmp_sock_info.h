#ifndef __NMP_MEDIA_SOCK_INFO_H__
#define __NMP_MEDIA_SOCK_INFO_H__

#include <rtspwatch.h>
#include "nmp_config.h"

typedef enum
{
	MEDIA_TRANS_TCP,
	MEDIA_TRANS_UDP
}JpfMediaTrans;


typedef struct _JpfMediaSocket JpfMediaSocket;
struct _JpfMediaSocket
{
	JpfMediaTrans		transport;		/* transport style */
	gint				rtp_sock;		/* socket for rtp packets */
	gint				rtp_port;		/* rtp port, host order */

#ifdef CONFIG_RTCP_SUPPORT
	gint				rtcp_sock;		/* socket for rtcp controlling */
	gint				rtcp_port;		/* rtcp port, host order */
#endif
};


gint nmp_media_sock_info_init(JpfMediaSocket *sk_info, 
	JpfMediaTrans trans);

void nmp_media_sock_info_reset(JpfMediaSocket *sk_info);

gint nmp_media_sock_connect(JpfMediaSocket *sk_info, gchar *ip, 
	GstRTSPTransport *transport);

void nmp_media_sock_get_server_ports(JpfMediaSocket *sk_info, 
	GstRTSPTransport *ct);

gint nmp_media_sock_est_rtp_sock(JpfMediaSocket *sk_info);

gint nmp_media_sock_est_rtcp_sock(JpfMediaSocket *sk_info);

#endif	//__NMP_MEDIA_SOCK_INFO_H__
