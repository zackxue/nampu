#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include "nmplib.h"
#include <limits.h>
#include <errno.h>
#include "nmp_conn.h"

#define LISTEN_QUEUE			5
#define DNS_BUFFER_LEN			8192
#define DEFAULT_TIMEOUT			(10 * 1000)

struct _nmp_connection
{
	int		fd;
	int		timeout;	/* milli seconds, for event polling */
	int		flags;
};

static __inline__ int
__nmp_conn_set_flags(nmp_conn_t *conn, int flgs)
{
    int old_flgs;

    old_flgs = fcntl(conn->fd, F_GETFL, 0);
    if (NMP_UNLIKELY(old_flgs < 0))
        return -errno;

    old_flgs |= flgs;

    if (fcntl(conn->fd, F_SETFL, old_flgs) < 0)
        return -errno;

    return 0;
}


__export nmp_conn_t *
nmp_conn_new(struct sockaddr *sa, unsigned flags, int *errp)
{
	nmp_conn_t *conn;
	unsigned mask = CF_TYPE_TCP | CF_TYPE_UDP;
	int err, reuse = 1;

	if (!(flags & mask) || ((flags & mask) == mask))
	{
		if (errp)
			*errp = -EINVAL;
		return NULL;
	}

	conn = nmp_new0(nmp_conn_t, 1);	/* jlib has its own OOM facility */
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
		err = __nmp_conn_set_flags(conn, O_NONBLOCK);
		if (err)
			goto new_conn_failed;
	}

	return conn;

new_conn_failed:
	if (errp)
		*errp = err;

	if (conn->fd > 0)
		close(conn->fd);

	nmp_del(conn, nmp_conn_t, 1);
	return NULL;
}


__export int
nmp_conn_listen(nmp_conn_t *conn)
{
	NMP_ASSERT(conn != NULL);

	if (!(conn->flags & CF_TYPE_TCP))
		return -EINVAL;

	if (listen(conn->fd, LISTEN_QUEUE) < 0)
		return -errno;
	return 0;
}


__export int
nmp_conn_connect(nmp_conn_t *conn, struct sockaddr *sa)
{
	int err;
	NMP_ASSERT(conn != NULL);

	if (connect(conn->fd, sa, sizeof(*sa)) < 0)
	{
		err = -errno;
		if (err == -EINPROGRESS)
			conn->flags |= CF_FLGS_CONN_INPROGRESS;
		return err;
	}
	return 0;
}


__export int
nmp_conn_get_fd(nmp_conn_t *conn)
{
	NMP_ASSERT(conn != NULL);

	return conn->fd;
}


__export int
nmp_conn_set_flags(nmp_conn_t *conn, unsigned flgs)
{
	int ret;
	NMP_ASSERT(conn != NULL);

	if (flgs & CF_FLGS_NONBLOCK)
	{
		ret =  __nmp_conn_set_flags(conn, O_NONBLOCK);
		if (!ret)
			conn->flags |= CF_FLGS_NONBLOCK;
		return 0;
	}

	return -EINVAL;
}


__export int
nmp_conn_is_blocked(nmp_conn_t *conn)
{
	NMP_ASSERT(conn != NULL);

	return !(conn->flags & CF_FLGS_NONBLOCK);
}


__export nmp_conn_t *
nmp_conn_accept(nmp_conn_t *listen, int *errp)
{
	nmp_conn_t *conn;
	int ret;
	NMP_ASSERT(listen != NULL);

	if (!(listen->flags & CF_TYPE_TCP))
	{
		*errp = -EINVAL;
		return NULL;
	}

	conn = nmp_new0(nmp_conn_t, 1);
	conn->fd = accept(listen->fd, NULL, NULL);
	if (conn->fd < 0)
	{
		*errp = errno;
		nmp_del(conn, nmp_conn_t, 1);
		return NULL;
	}

	if (listen->flags & CF_FLGS_NONBLOCK)
	{
		ret = __nmp_conn_set_flags(conn, O_NONBLOCK);
		if (ret < 0)
		{
			*errp = ret;
			close(conn->fd);
			nmp_del(conn, nmp_conn_t, 1);
			return NULL;
		}

		conn->flags |= CF_FLGS_NONBLOCK;
	}

	conn->timeout = DEFAULT_TIMEOUT;
	return conn;
}


__export int
nmp_conn_is_ingrogress(nmp_conn_t *conn, int clear)
{
	NMP_ASSERT(conn != NULL);

	if (conn->flags & CF_FLGS_CONN_INPROGRESS)
	{
		if (clear)
			conn->flags &= ~CF_FLGS_CONN_INPROGRESS;
		return 1;
	}

	return 0;
}


__export int
nmp_conn_read(nmp_conn_t *conn, char buf[], size_t size)
{
	int ret;

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


__export int
nmp_conn_write(nmp_conn_t *conn, char *buf, size_t size)
{
	size_t left = size;
	int ret;

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


__export int
nmp_resolve_host(struct sockaddr_in *sin, char *host, int port)
{
	char dns_buf[DNS_BUFFER_LEN];
	struct hostent ht, *ret = NULL;
	int err, rc;

	NMP_ASSERT(sin != NULL && host != NULL);
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


__export int
nmp_conn_get_timeout(nmp_conn_t *conn)
{
	NMP_ASSERT(conn != NULL);

	return conn->timeout;
}


__export void
nmp_conn_set_timeout(nmp_conn_t *conn, int millisec)
{
	int max_timeout;
	NMP_ASSERT(conn != NULL);

	max_timeout = INT_MAX >> 10;

	if (millisec <= 0)
		millisec = DEFAULT_TIMEOUT;
	else if (millisec > max_timeout)
		millisec = max_timeout;

	conn->timeout = millisec;
}


__export void
nmp_conn_close(nmp_conn_t *conn)
{
	NMP_ASSERT(conn != NULL);

	if (conn->fd > 0)
		close(conn->fd);

	nmp_del(conn, nmp_conn_t, 1);
}

__export char *
nmp_conn_get_host(nmp_conn_t *conn, char *ip)
{
	unsigned char *sa;
	struct sockaddr_in addr;
	socklen_t len;

	if (!conn)
		return NULL;

	len = sizeof(addr);
	bzero(&addr, sizeof(addr));

	if (getsockname(conn->fd, (struct sockaddr*)&addr, &len))
		return NULL;
	
	if (addr.sin_family != AF_INET)
		return NULL;

	sa = (unsigned char*)&addr.sin_addr;
	sprintf(ip, "%d.%d.%d.%d", (int)sa[0], (int)sa[1], (int)sa[2], (int)sa[3]);

	return ip;
}

__export char *
nmp_conn_get_peer(nmp_conn_t *conn, char *ip)
{
	unsigned char *sa;
	struct sockaddr_in addr;
	socklen_t len;

	if (!conn)
		return NULL;

	len = sizeof(addr);
	bzero(&addr, sizeof(addr));

	if (getpeername(conn->fd, (struct sockaddr*)&addr, &len))
		return NULL;
	
	if (addr.sin_family != AF_INET)
		return NULL;

	sa = (unsigned char*)&addr.sin_addr;
	sprintf(ip, "%d.%d.%d.%d", (int)sa[0], (int)sa[1], (int)sa[2], (int)sa[3]);

	return ip;
}


//: ~End
