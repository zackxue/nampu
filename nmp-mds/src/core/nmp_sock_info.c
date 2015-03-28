#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "nmp_errno.h"
#include "nmp_sock_info.h"
#include "nmp_debug.h"
#include "nmp_ports.h"
#include "nmp_utils.h"

#define LISTEN_QUEUE_CNT		5
#define SOCK_RECV_BUFF_SIZE		(512*1024)

static __inline__ void
nmp_media_sock_set_opt(gint fd)
{//{This makes no sense, defense UDP net burst?? }
	gint opt = SOCK_RECV_BUFF_SIZE;

	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &opt, sizeof(opt)))
	{
		nmp_warning(
			"Set sock recv buff size failed, err:'%d'.",
			-errno
		);
	}
}


static __inline__ gint
nmp_media_sock_get_fd(JpfMediaTrans trans, gint *port)
{
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	gint sock, sock_type, err;

	sock_type = SOCK_DGRAM;

	if (trans != MEDIA_TRANS_UDP)
		sock_type = SOCK_STREAM;

	sock = socket(AF_INET, sock_type, 0);
	if (sock < 0)
	{
		err = -errno;
		nmp_warning(
			"Create media socket failed, code:%d.",
			err
		);
		return err;
	}

	nmp_media_sock_set_opt(sock);

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(*port);
	sin.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr*)&sin, sizeof(sin)))
	{
		err = -errno;
		nmp_warning(
			"Bind media socket %d on port %d failed, code:%d",
			sock, *port, err
		);
		close(sock);
		return err;
	}

	if (!*port)
	{
		if (getsockname(sock, (struct sockaddr*)&sin, &len))
		{
			err = -errno;
			nmp_warning(
				"Get socket local info failed, code:%d",
				err
			);
			close(sock);
			return err;
		}

		*port = ntohs(sin.sin_port);
	}

	if (sock_type == SOCK_STREAM)
		listen(sock, LISTEN_QUEUE_CNT);

	return sock;
}


__export gint
nmp_media_sock_info_init(JpfMediaSocket *sk_info, JpfMediaTrans trans)
{
	gint low = 0, hi = 0;
	G_ASSERT(trans == MEDIA_TRANS_UDP || trans == MEDIA_TRANS_TCP);

	for (;;)
	{
		memset(sk_info, 0, sizeof(*sk_info));

#ifdef CONFIG_RTCP_SUPPORT
		if (!nmp_media_ports_get_pair(&sk_info->rtp_port, 
				&sk_info->rtcp_port))
		{
			sk_info->rtp_sock = nmp_media_sock_get_fd(
				trans, 
				&sk_info->rtp_port
			);

			if (sk_info->rtp_sock < 0)
			{
				nmp_media_ports_put_pair(
					sk_info->rtp_port, sk_info->rtcp_port
				);

				if (sk_info->rtp_sock != -EADDRINUSE)
				{
					nmp_warning(
						"Bind rtp socket on port '%d' failed:'%s'.",
						sk_info->rtp_port,
						Err(sk_info->rtp_sock)
					);
					return sk_info->rtp_sock;
				}

				if (low == sk_info->rtp_port)
				{
					nmp_media_ports_get_range(&low, &hi);

					nmp_warning(
						"System out of port in range:[%d, %d), port conflicts "
						"with other process.", low, hi
					);					
					break;
				}

				if (!low)
				{
					low = sk_info->rtp_port;
				}
				
				continue;
			}

			sk_info->rtcp_sock = nmp_media_sock_get_fd(
				trans, 
				&sk_info->rtcp_port
			);

			if (sk_info->rtcp_sock < 0)
			{
				close(sk_info->rtp_sock);

				nmp_media_ports_put_pair(
					sk_info->rtp_port, sk_info->rtcp_port
				);

				if (sk_info->rtcp_sock != -EADDRINUSE)
				{
					nmp_warning(
						"Bind rtcp socket on port '%d' failed:'%s'.",
						sk_info->rtcp_port,
						Err(sk_info->rtcp_sock)
					);
					return sk_info->rtcp_sock;					
				}

				continue;
			}
printf("=============>sock:%d %d, port: %d %d\n", sk_info->rtp_sock,  sk_info->rtcp_sock, sk_info->rtp_port, sk_info->rtcp_port);
			set_fd_flags(sk_info->rtp_sock, O_NONBLOCK);
			set_fd_flags(sk_info->rtcp_sock, O_NONBLOCK);
			sk_info->transport = trans;

			return 0;
		}
#else
		if (!nmp_media_ports_get_one(&sk_info->rtp_port))
		{
			sk_info->rtp_sock = nmp_media_sock_get_fd(
				trans, 
				&sk_info->rtp_port
			);

			if (sk_info->rtp_sock < 0)
			{
				nmp_media_ports_put_one(sk_info->rtp_port);

				if (sk_info->rtp_sock != -EADDRINUSE)
				{
					nmp_warning(
						"Bind rtp socket on port '%d' failed:'%s'.",
						sk_info->rtp_port,
						Err(sk_info->rtp_sock)
					);
					return sk_info->rtp_sock;
				}

				continue;
			}

			set_fd_flags(sk_info->rtp_sock, O_NONBLOCK);	//要和RTSP socket同样处理so_linger!	
			sk_info->transport = trans;

			return 0;
		}
#endif
		else
		{
			nmp_media_ports_get_range(&low, &hi);

			nmp_warning(
				"System out of ports, range: [%d, %d).",
				low, hi
			);
			return -E_OUTOFPORTS;
		}
	}

	return -E_OUTOFPORTS;
}


static __inline__ gint
nmp_media_sock_connect_one(gint sock, gchar *Ip, gint port)
{
	gint err;
	struct sockaddr_in sin;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = inet_addr(Ip);

	if (connect(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		err = -errno;
		nmp_warning(
			"Can't Access host '%s:%d', code:%d.",
			Ip,
			port,
			err
		);
		return err;
	}

	return 0;
}


gint
nmp_media_sock_connect(JpfMediaSocket *sk_info, gchar *remote_Ip,
	GstRTSPTransport *ct)
{
	gint err;
	g_assert(sk_info != NULL);

	if (G_UNLIKELY(!remote_Ip || !ct))
		return -E_INVAL;

	if (ct->lower_transport != GST_RTSP_LOWER_TRANS_UDP)
		return -E_INVAL;

	err = nmp_media_sock_connect_one(sk_info->rtp_sock,
		remote_Ip, ct->client_port.min);

	if (G_LIKELY(!err))
	{
		ct->server_port.min = sk_info->rtp_port;

#ifdef CONFIG_RTCP_SUPPORT
		err = nmp_media_sock_connect_one(sk_info->rtcp_sock,
			remote_Ip, ct->client_port.max);
		ct->server_port.max = sk_info->rtcp_port;
#else
		ct->server_port.max = sk_info->rtp_port + 1;		/* 不妥，要是客户端MPLAYER等真向这个端口发包...*/
#endif
	}

	return err;	
}


void
nmp_media_sock_get_server_ports(JpfMediaSocket *sk_info, 
	GstRTSPTransport *ct)
{
	g_assert(sk_info != NULL && ct != NULL);

	ct->server_port.min = sk_info->rtp_port;

#ifdef CONFIG_RTCP_SUPPORT
	ct->server_port.max = sk_info->rtcp_port;
#else
	ct->server_port.max = sk_info->rtp_port + 1;
#endif
}


gint
nmp_media_sock_est_rtp_sock(JpfMediaSocket *sk_info)
{
	gint sock;

	if (sk_info->transport != MEDIA_TRANS_TCP)
		return -EINVAL;

	sock = accept(sk_info->rtp_sock, NULL, NULL);
	if (sock < 0)
		return -errno;

	close(sk_info->rtp_sock);
	sk_info->rtp_sock = sock;
	set_fd_flags(sk_info->rtp_sock, O_NONBLOCK);

	return 0;
}


gint
nmp_media_sock_est_rtcp_sock(JpfMediaSocket *sk_info)
{
#ifdef CONFIG_RTCP_SUPPORT
	gint sock;

	if (sk_info->transport != MEDIA_TRANS_TCP)
		return -EINVAL;

	sock = accept(sk_info->rtcp_sock, NULL, NULL);
	if (sock < 0)
		return -errno;

	close(sk_info->rtcp_sock);
	sk_info->rtcp_sock = sock;
	set_fd_flags(sk_info->rtcp_sock, O_NONBLOCK);
#endif

	return 0;
}

void
nmp_media_sock_info_reset(JpfMediaSocket *sk_info)
{
	if (sk_info->rtp_sock > 0)
	{
		close(sk_info->rtp_sock);
		sk_info->rtp_sock = -1;

#ifdef CONFIG_RTCP_SUPPORT
		close(sk_info->rtcp_sock);
		nmp_media_ports_put_pair(
			sk_info->rtp_port, sk_info->rtcp_port
		);
#else
		nmp_media_ports_put_one(sk_info->rtp_port);
#endif
	}
}

//:~ End
