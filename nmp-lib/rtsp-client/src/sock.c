/* *
 * This file is part of rtsp-client lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "fnc_log.h"
#include "ports.h"
#include "sock.h"


gint
unix_sock_bind(const gchar *host, gint port, L4_Proto l4)
{
	struct sockaddr_in sin;
	gint sock, err;

	sock = socket(AF_INET, l4 == L4_TCP ? SOCK_STREAM : SOCK_DGRAM, 0);
	if (sock < 0)
	{
		err = -errno;
		fnc_log(FNC_LOG_ERR, "[FC] Create socket failed.");
		return err;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = host ? inet_addr(host) : INADDR_ANY;

	if (bind(sock, (struct sockaddr*)&sin, sizeof(sin)))
	{
		err = -errno;
		fnc_log(FNC_LOG_ERR, "[FC] Bind socket failed on port '%d'.", port);
		close(sock);
		return err;
	}

	return sock;
}


gint
unix_sock_connect(gint sock, const char *host, gint port)
{
	struct sockaddr_in sin;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = host ? inet_addr(host) : INADDR_ANY;

	if (connect(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
		return -errno;
	return 0;
}


gint
set_fd_flags(gint fd, gint flgs)
{
    gint old_flgs, err;

    old_flgs = fcntl(fd, F_GETFL, 0);
    if (G_UNLIKELY(old_flgs < 0))
    {
    	err = -errno;
    	fnc_log(FNC_LOG_ERR,
    		"set_fd_flags()->fcntl(fd, F_GETFL, 0)"
    	);
        return err;
    }

    old_flgs |= flgs;

    if (fcntl(fd, F_SETFL, old_flgs) < 0)
    {
    	err = -errno;
    	fnc_log(FNC_LOG_ERR,
    		"set_fd_flags()->fcntl(fd, F_SETFL, 0)"
    	);
        return err;
    }

    return 0;
}


static __inline__ gint 
get_new_udp_sock(gint *port)
{
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	gint sock, err;

	sock = unix_sock_bind(NULL, *port, L4_UDP);
	if (sock < 0)
		return sock;

	if (!*port)
	{
		if (getsockname(sock, (struct sockaddr*)&sin, &len))
		{
			err = -errno;
			fnc_log(FNC_LOG_ERR, "[FC] getsockname() failed.");			
			close(sock);
			return err;
		}
		*port = ntohs(sin.sin_port);
	}

	return sock;
}


gint
rtp_session_sock_init(RTP_Session *rtp_s)
{
	gint low = 0, hi = 0;

	for (;;)
	{
#ifdef CONFIG_RTCP_SUPPORT
		if (!jpf_media_ports_get_pair(&rtp_s->rtp_port, 
				&rtp_s->rtcp_port))
		{
			rtp_s->rtp_sock = get_new_udp_sock(&rtp_s->rtp_port);
			if (rtp_s->rtp_sock < 0)
			{
				jpf_media_ports_put_pair(rtp_s->rtp_port, rtp_s->rtcp_port);
				if (rtp_s->rtp_sock != -EADDRINUSE)
				{
					fnc_log(FNC_LOG_ERR, "[FC] Bind rtp socket on port '%d' failed.",
						rtp_s->rtp_port);
					return rtp_s->rtp_sock;
				}

				if (low == rtp_s->rtp_port)	/* Walk around back */
				{
					jpf_media_ports_get_range(&low, &hi);
					fnc_log(FNC_LOG_ERR, "[FC] System out of port, range:[%d, %d).", low, hi);
					break;
				}

				if (!low)
					low = rtp_s->rtp_port;

				continue;
			}

			rtp_s->rtcp_sock = get_new_udp_sock(&rtp_s->rtcp_port);
			if (rtp_s->rtcp_sock < 0)
			{
				close(rtp_s->rtp_sock);
				jpf_media_ports_put_pair(rtp_s->rtp_port, rtp_s->rtcp_port);
				if (rtp_s->rtcp_sock != -EADDRINUSE)
				{
					fnc_log(FNC_LOG_ERR, "[FC] Bind rtcp socket on port '%d' failed.",
						rtp_s->rtcp_port);					
					return rtp_s->rtcp_sock;					
				}

				continue;
			}

			/* other flags was reset */
			fcntl(rtp_s->rtp_sock, F_SETFL, O_NONBLOCK);
			fcntl(rtp_s->rtcp_sock, F_SETFL, O_NONBLOCK);
			return 0;
		}
#else
		if (!jpf_media_ports_get_one(&rtp_s->rtp_port))
		{
			rtp_s->rtp_sock = get_new_udp_sock(&rtp_s->rtp_port);
			if (rtp_s->rtp_sock < 0)
			{
				jpf_media_ports_put_one(rtp_s->rtp_port);
				if (rtp_s->rtp_sock != -EADDRINUSE)
				{
					fnc_log(FNC_LOG_ERR, "[FC] Bind rtp socket on port '%d' failed.",
						rtp_s->rtp_port);
					return rtp_s->rtp_sock;
				}

				continue;
			}

			fcntl(rtp_s->rtp_sock, F_SETFL, O_NONBLOCK); //要和RTSP socket同样处理so_linger!
			return 0;
		}
#endif
		else
		{
			jpf_media_ports_get_range(&low, &hi);
			fnc_log(FNC_LOG_ERR, "[FC] System out of ports, range: [%d, %d).",low, hi);
			return -E_OUTOFPORTS;
		}
	}

	return -E_OUTOFPORTS;
}


void
rtp_session_sock_close(RTP_Session *rtp_s)
{
	if (rtp_s->rtp_sock <= 0)
		return;

	close(rtp_s->rtp_sock);
	rtp_s->rtp_sock = -1;

#ifdef CONFIG_RTCP_SUPPORT
	close(rtp_s->rtcp_sock);
	rtp_s->rtcp_sock = -1;
	jpf_media_ports_put_pair(rtp_s->rtp_port, rtp_s->rtcp_port);
#else
	jpf_media_ports_put_one(rtp_s->rtp_port);
#endif
}


//:~ End
