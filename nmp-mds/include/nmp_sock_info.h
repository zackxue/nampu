#ifndef __NMP_MEDIA_SOCK_INFO_H__
#define __NMP_MEDIA_SOCK_INFO_H__

#include <rtspwatch.h>
#include "nmp_config.h"

typedef enum
{
	MEDIA_TRANS_TCP,
	MEDIA_TRANS_UDP
}NmpMediaTrans;


typedef struct _NmpMediaSocket NmpMediaSocket;
struct _NmpMediaSocket
{
	NmpMediaTrans		transport;		/* transport style */
	gint				rtp_sock;		/* socket for rtp packets */
	gint				rtp_port;		/* rtp port, host order */

#ifdef CONFIG_RTCP_SUPPORT
	gint				rtcp_sock;		/* socket for rtcp controlling */
	gint				rtcp_port;		/* rtcp port, host order */
#endif
};


gint nmp_media_sock_info_init(NmpMediaSocket *sk_info, 
	NmpMediaTrans trans);

void nmp_media_sock_info_reset(NmpMediaSocket *sk_info);

gint nmp_media_sock_connect(NmpMediaSocket *sk_info, gchar *ip, 
	GstRTSPTransport *transport);

void nmp_media_sock_get_server_ports(NmpMediaSocket *sk_info, 
	GstRTSPTransport *ct);

gint nmp_media_sock_est_rtp_sock(NmpMediaSocket *sk_info);

gint nmp_media_sock_est_rtcp_sock(NmpMediaSocket *sk_info);

#endif	//__NMP_MEDIA_SOCK_INFO_H__
