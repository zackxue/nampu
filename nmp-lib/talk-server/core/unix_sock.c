#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include "unix_sock.h"

#define __u8_str(p) ((uint8_t*)(p))
#define __str(p) ((char*)(p))

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

int socket_write(int fd, char *buf, int size)
{
    size_t left = size;
    int ret;

    while (left > 0)
    {
#ifdef WIN32
        ret = send(fd, buf, left, 0);
#else
        ret = write(fd, buf, left);                                                                                                                                        
#endif
        if (ret < 0)
        {
#ifdef WIN32
            ret = WSAGetLastError();
            if (WSAEINTR == ret)
            {
                continue;
            }
#else
    #if 0
            ret = -errno;
            if (ret == -EINTR)
            {
                continue;
            }
            
            if (ret == -EAGAIN)
            {    
                break;
            }
            return ret;
    #else
            break;
    #endif

#endif
        }
        else if (ret == 0)
        {
            break;
        }

        buf += ret;
        left -= ret;
    }

    return size - left;
}

int socket_read(int fd, char *buf, int size)
{
    int ret;

    for (;;)
    {
#ifdef WIN32
    	ret = recv(fd, buf, size, 0);
#else
        ret = read(fd, buf, size);
#endif
        if (ret < 0)
        {
#ifdef WIN32
    		ret = WSAGetLastError();
    		if (WSAEINTR == ret)
    		{
    		    continue;
            }
#else
            ret = -errno;
            if (ret == -EINTR)
            {
                continue;
            }
#endif
            return ret;
        }
        else if (ret == 0)
        {// a. client断开连接 
         // b. 当client断开以后，如果依然读该fd, errno一直返回RST, 而且会一直触发select的可读事件
         //    这个时候client的TIMEOUT机制就无效了, TIMEOUT只有在连接正常时生效，难怪mds要额外增加
         //    心跳,(但是MDS的心跳是OPTION啊，而select触发是RST, 可能与epoll有关?[疑问]epoll也会和
         //    select一样吗?)
#ifdef WIN32        
            ret = WSAGetLastError();
#else
            ret = -errno;
#endif
        }
        
        return ret;
    }

    return 0;    
}

int socket_set_noblock(int fd)
{
#ifdef WIN32
    int bNoBlock = 1;
	
    if (ioctlsocket(fd, FIONBIO, (unsigned long *)&bNoBlock) < 0)
        return -1;
#else    
    int flags;    
    flags = fcntl(fd, F_GETFL);    
    if (flags < 0)
    {
        return flags;    
        flags |= O_NONBLOCK;    
        if (fcntl(fd, F_SETFL, flags) < 0)             
        {
            return -1;
        }
    }
#endif    
    return 0;
}

int socket_set_linger(int fd)
{
	struct linger nLinger;

	if (fd <= 0)
		return -1;
	
	memset(&nLinger, 0, sizeof(struct linger));
	nLinger.l_onoff = 1;  // 使用SO_LINGER生效
	nLinger.l_linger= 0;  // 套接字关闭时，直接抛弃保留在套接字发送缓存区的数据，直接发RST给对方
                          // 避免TIME_WAIT的产生.
	if (setsockopt(fd, SOL_SOCKET, SO_LINGER, (void *)&nLinger, sizeof(struct linger)) < 0)
    {
        return -1;
    }
	return 0;
}

int socket_set_attr_send_buf(int fd, int send_buf)
{
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void  *)&send_buf, sizeof(send_buf)) < 0)
    {
        return -1;
    }
    return 0;
}

//:~ End
