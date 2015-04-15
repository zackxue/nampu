/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_UNIX_SOCK_H__
#define __TINY_RAIN_UNIX_SOCK_H__

#include <fcntl.h>
#include <netinet/in.h>
#include "def.h"

#define FORCE_BIND		0x01

typedef enum
{
	L4_TCP,
	L4_UDP,
	L4_NONE
}L4_type;

int32_t unix_sock_bind(L4_type l4, uint32_t host,
	uint16_t port, uint32_t flags);
int32_t unix_sock_tcp_listen(int32_t sock);
int32_t unix_sock_tcp_accept(int32_t sock);

void unix_sock_close(int32_t sock);

int32_t unix_sock_connect(int32_t sock, uint8_t *ip, uint16_t port);
int32_t unix_sock_set_flags(int32_t sock, uint32_t flag);
int32_t unix_resolve_host(struct sockaddr_in *sin, uint8_t *host, uint16_t port);

int32_t unix_sock_get_peer(int32_t sock, uint8_t *ip, int32_t size);

int socket_write(int fd, char *buf, int size);
int socket_read(int fd, char *buf, int size);
int socket_set_noblock(int fd);
int socket_set_linger(int fd);

#endif	//__TINY_RAIN_UNIX_SOCK_H__
