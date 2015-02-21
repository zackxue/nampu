#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "unix_sock.h"

#define LISTEN_Q				5
#define DNS_BUFFER_LEN			8192

int32_t unix_sock_bind(L4_type l4, uint32_t host,
	uint16_t port, uint32_t flags)
{
	struct sockaddr_in addr;
	int32_t sock, err, tag;

	sock = socket(AF_INET, l4 == L4_TCP ?
		SOCK_STREAM : SOCK_DGRAM, 0);
	if (sock < 0)
	{
		return -errno;
	}

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	addr.sin_addr.s_addr = host;

	if (flags & FORCE_BIND)
	{
		tag = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
			(void *)&tag, sizeof(tag));
	}

	err = bind(sock, (struct sockaddr*)&addr,
		sizeof(addr));
	if (err < 0)
	{
		err = -errno;
		close(sock);
		return err;
	}

	return sock;
}


int32_t unix_sock_tcp_listen(int32_t sock)
{
	int32_t err;

	err = listen(sock, LISTEN_Q);
	if (err < 0)
		return -errno;

	return 0;
}


int32_t unix_sock_tcp_accept(int32_t sock)
{
	int32_t new_sock;

	new_sock = accept(sock, NULL, NULL);
	if (new_sock < 0)
		return -errno;

	return new_sock;
}


int32_t unix_sock_connect(int32_t sock, uint8_t *ip,
	uint16_t port)
{
	struct sockaddr_in addr;
	int32_t err;

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(__str(ip));

	err = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
	if (err < 0)
	{
		return -errno;
	}
	return err;
}


int32_t unix_resolve_host(struct sockaddr_in *sin, uint8_t *host,
	uint16_t port)
{
	uint8_t dns_buf[DNS_BUFFER_LEN];
	struct hostent ht, *ret = NULL;
	int32_t err, rc;

	bzero(sin, sizeof(*sin));

	rc = gethostbyname_r(__str(host), &ht, __str(dns_buf),
		DNS_BUFFER_LEN, &ret, &err);

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

int32_t unix_sock_set_flags(int32_t sock, uint32_t flag)
{
    uint32_t old_flgs;

    old_flgs = fcntl(sock, F_GETFL, 0);
    if (old_flgs < 0)
        return -errno;

    old_flgs |= flag;

    if (fcntl(sock, F_SETFL, old_flgs) < 0)
        return -errno;

    return 0;
}


int32_t unix_sock_get_peer(int32_t sock, uint8_t *ip, int32_t size)
{//@{Not thread-safe, can't be reentry}
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);

	bzero(&sin, sizeof(sin));
	if (getpeername(sock, (struct sockaddr*)&sin, &len) < 0)
		return -errno;
	
	strncpy(__str(ip), inet_ntoa(sin.sin_addr), size - 1);
	ip[size -1] = '\0';

	return 0;
}


void unix_sock_close(int32_t sock)
{
	if (sock <= 2)
		return;

	close(sock);
}


//:~ End
