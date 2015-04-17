
#include "stdafx.h"
#include <winsock2.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "network.h"

#ifdef WIN32
typedef int	socklen_t;
#endif

int socket_set_noblock(int fd)
{
#ifdef WIN32
	int bNoBlock = 1;

	if (ioctlsocket(fd, FIONBIO, (unsigned long *)&bNoBlock) < 0 )
		return -1;
#else
	int flags;

	flags = fcntl(fd, F_GETFL);
	if (flags < 0)
		return flags;
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) 
		return -1;
#endif
	return 0;
}

int socket_connect(char *ip, unsigned int port)
{
	int fd;
	struct sockaddr_in dst_addr;
	socklen_t addrlen = sizeof(dst_addr);

	if(ip == NULL || port == 0)
	{
		return -1;
	}
	
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0)
	{
		return -1;
	}

	//if(socket_set_noblock(fd) < 0)
	//{
	//	goto _error;
	//}

	dst_addr.sin_family = AF_INET;
	dst_addr.sin_port = htons(port);
#ifdef WIN32
	dst_addr.sin_addr.s_addr = inet_addr(ip);
#else
	inet_aton(ip, &dst_addr.sin_addr);
#endif    
	if(connect(fd, (struct sockaddr *)&dst_addr, addrlen) < 0)
	{
#ifdef WIN32
		goto _error;
#else
		return -errno;
#endif
	}
	return fd;

_error:
	if (fd > 0)
		closesocket(fd);
	
	return -WSAGetLastError();
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
				continue;

			if (WSAEWOULDBLOCK == ret)
				break;

			return (-ret);
#else
			ret = -errno;
			if (ret == -EINTR)
				continue;

			if (ret == -EAGAIN)
				break;
			return ret;
#endif


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
			// 原来的错误值返回的正值, 上层不能判断是否正确
			// 原来的错误值(10055)返回以后, 一直出错, 结果出现了一个(10053)错误, 彩蛋啊,以前没见过
			ret = -WSAGetLastError();
			if (WSAEINTR == -ret)
			{
				continue;
			}
#else
			ret = -errno;
			if (ret == -EINTR)
				continue;
#endif
			return ret;
		}

		return ret;
	}

	return 0;
}