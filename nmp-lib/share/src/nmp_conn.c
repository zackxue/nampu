#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_conn.h"

#define LISTEN_QUEUE			5
#define DNS_BUFFER_LEN			8192
#define DEFAULT_TIMEOUT			(10 * 1000)

struct _HmConnection
{
	gint		fd;
	gint		timeout;	/* milli seconds, for event polling */
	gint		flags;
	gint        buffer_size;
};


static __inline__ gint
__hm_connection_set_flags(HmConnection *conn, gint flgs)
{
    gint old_flgs;

    old_flgs = fcntl(conn->fd, F_GETFL, 0);
    if (G_UNLIKELY(old_flgs < 0))
        return -errno;

    old_flgs |= flgs;

    if (fcntl(conn->fd, F_SETFL, old_flgs) < 0)
        return -errno;

    return 0;
}


__export HmConnection *
hm_connection_new(struct sockaddr *sa, guint flags, gint *errp)
{
	HmConnection *conn;
	guint mask = CF_TYPE_TCP | CF_TYPE_UDP;
	gint err, reuse = 1;

	if (!(flags & mask) || ((flags & mask) == mask))
	{
		if (errp)
			*errp = -EINVAL;
		return NULL;
	}

	conn = g_new0(HmConnection, 1);	/* glib has its own OOM facility */
	conn->flags = flags;
	conn->timeout = DEFAULT_TIMEOUT;

	switch (mask & flags)
	{
	case CF_TYPE_TCP:
		conn->fd = socket(AF_INET, SOCK_STREAM, 0);
		if (conn->fd < 0)
		{
			err = -errno;
			goto new_conn_failed;
		}

		break;

	case CF_TYPE_UDP:
		conn->fd = socket(AF_INET, SOCK_DGRAM, 0);
		if (conn->fd < 0)
		{
			err = -errno;
			goto new_conn_failed;
		}

		break;
	}

	if (sa)
	{
		setsockopt(conn->fd, SOL_SOCKET, SO_REUSEADDR, 
			&reuse, sizeof(reuse));

		if (bind(conn->fd, sa, sizeof(*sa)) < 0)
		{
			err = -errno;
			goto new_conn_failed;
		}
	}

	if (flags & CF_FLGS_NONBLOCK)
	{
		err = __hm_connection_set_flags(conn, O_NONBLOCK);
		if (err)
			goto new_conn_failed;
	}

	return conn;

new_conn_failed:
	if (errp)
		*errp = err;

	if (conn->fd > 0)
		close(conn->fd);

	g_free(conn);
	return NULL;
}


__export gint
hm_connection_listen(HmConnection *conn)
{
	G_ASSERT(conn != NULL);

	if (!(conn->flags & CF_TYPE_TCP))
		return -EINVAL;

	if (listen(conn->fd, LISTEN_QUEUE) < 0)
		return -errno;
	return 0;
}


__export gint
hm_connection_connect(HmConnection *conn, struct sockaddr *sa)
{
	gint err;
	G_ASSERT(conn != NULL);

	if (connect(conn->fd, sa, sizeof(*sa)) < 0)
	{
		err = -errno;
		if (err == -EINPROGRESS)
			conn->flags |= CF_FLGS_CONN_INPROGRESS;
		return err;
	}
	return 0;
}

__export gint
hm_connection_get_fd(HmConnection *conn)
{
	G_ASSERT(conn != NULL);

	return conn->fd;
}


__export gint
hm_connection_set_flags(HmConnection *conn, guint flgs)
{
	gint ret;
	G_ASSERT(conn != NULL);

	if (flgs & CF_FLGS_NONBLOCK)
	{
		ret =  __hm_connection_set_flags(conn, O_NONBLOCK);
		if (!ret)
			conn->flags |= CF_FLGS_NONBLOCK;
		return 0;
	}

	return -EINVAL;
}


__export gint
hm_connection_is_blocked(HmConnection *conn)
{
	G_ASSERT(conn != NULL);

	return !(conn->flags & CF_FLGS_NONBLOCK);
}


__export HmConnection *
hm_connection_accept(HmConnection *listen, gint *errp)
{
	HmConnection *conn;
	gint ret;
	G_ASSERT(listen != NULL);

	if (!(listen->flags & CF_TYPE_TCP))
	{
		*errp = -EINVAL;
		return NULL;
	}

	conn = g_new0(HmConnection, 1);
	conn->fd = accept(listen->fd, NULL, NULL);
	if (conn->fd < 0)
	{
		*errp = -errno;
		g_free(conn);
		return NULL;
	}

	if (listen->flags & CF_FLGS_NONBLOCK)
	{
		ret = __hm_connection_set_flags(conn, O_NONBLOCK);
		if (ret < 0)
		{
			*errp = ret;
			close(conn->fd);
			g_free(conn);
			return NULL;
		}

		conn->flags |= CF_FLGS_NONBLOCK;
	}

	conn->timeout = DEFAULT_TIMEOUT;
	return conn;
}


__export gint
hm_connection_is_ingrogress(HmConnection *conn, int clear)
{
	G_ASSERT(conn != NULL);

	if (conn->flags & CF_FLGS_CONN_INPROGRESS)
	{
		if (clear)
			conn->flags &= ~CF_FLGS_CONN_INPROGRESS;
		return 1;
	}

	return 0;
}


__export gint
hm_connection_read(HmConnection *conn, gchar buf[], gsize size)
{
	gint ret;

	for (;;)
	{
		ret = read(conn->fd, buf, size);
		if (ret < 0)
		{
			ret = -errno;
			if (ret == -EINTR)
				continue;

			return ret;
		}

		return ret;
	}

	return 0;	/* never */
}


__export gint
hm_connection_write(HmConnection *conn, gchar *buf, gsize size)
{
	gsize left = size;
	gint ret;

	while (left > 0)
	{
		ret = write(conn->fd, buf, left);
		if (ret < 0)
		{
			ret = -errno;
			if (ret == -EINTR)
				continue;

			if (ret == -EAGAIN)
				break;

			return ret;
		}

		buf += ret;
		left -= ret;
	}

	return size - left;
}


__export gint
hm_resolve_host(struct sockaddr_in *sin, gchar *host, gint port)
{
	gchar dns_buf[DNS_BUFFER_LEN];
	struct hostent ht, *ret = NULL;
	gint err, rc;

	G_ASSERT(sin != NULL && host != NULL);
	bzero(sin, sizeof(*sin));

	rc = gethostbyname_r(host, &ht, dns_buf, DNS_BUFFER_LEN,
		&ret, &err);

	if (!rc && ret)
	{
		if (ret->h_addrtype != AF_INET)
			return -EPFNOSUPPORT;

		sin->sin_family = AF_INET;
		sin->sin_port = htons(port);
		sin->sin_addr = *((struct in_addr*)ret->h_addr);

		return 0;
	}

	return err ? -err : -EADDRNOTAVAIL;
}


__export void
hm_connection_set_buffer_size(HmConnection *conn, gint size)
{
	G_ASSERT(conn != NULL);
	conn->buffer_size = size;
}


__export gint
hm_connection_get_buffer_size(HmConnection *conn)
{
	G_ASSERT(conn != NULL);
	return conn->buffer_size;
}


__export void
hm_connection_set_heavy(HmConnection *conn)
{
	G_ASSERT(conn != NULL);

	conn->flags |= CF_FLGS_IO_HEAVY;
}


__export gint
hm_connection_is_heavy(HmConnection *conn)
{
	G_ASSERT(conn != NULL);

	return conn->flags & CF_FLGS_IO_HEAVY;
}


__export gint
hm_connection_get_timeout(HmConnection *conn)
{
	G_ASSERT(conn != NULL);

	return conn->timeout;
}


__export void
hm_connection_set_timeout(HmConnection *conn, gint millisec)
{
	gint max_timeout;
	G_ASSERT(conn != NULL);

	max_timeout = INT_MAX >> 10;

	if (millisec <= 0)
		millisec = DEFAULT_TIMEOUT;
	else if (millisec > max_timeout)
		millisec = max_timeout;

	conn->timeout = millisec;
}


__export void
hm_connection_close(HmConnection *conn)
{
	G_ASSERT(conn != NULL);

	if (conn->fd > 0)
		close(conn->fd);

	g_free(conn);
}


__export gchar *
hm_connection_get_peer(HmConnection *conn)
{
	struct sockaddr_in addr;
	socklen_t len;
	guint8 *pa;

	if (!conn)
		return NULL;

	bzero(&addr, sizeof(addr));
	len = sizeof(addr);

	if (getpeername(conn->fd, (struct sockaddr*)&addr, &len))
		return NULL;

	if (addr.sin_family != AF_INET)
		return NULL;

	pa = (guint8*)&addr.sin_addr;
	return g_strdup_printf("%d.%d.%d.%d", pa[0], pa[1], pa[2], pa[3]);
}

//: ~End
