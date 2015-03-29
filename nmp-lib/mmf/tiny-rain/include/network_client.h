/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_PROTO_CLIENT_H__
#define __TINY_RAIN_PROTO_CLIENT_H__

#include "client.h"
#include "proto_watch.h"

BEGIN_NAMESPACE

#define MAX_IP4_LEN 	16
	
typedef struct __network_client network_client;
typedef struct __network_client_ops network_client_ops;

struct __network_client
{
	client	__super;
	uint8_t ip[MAX_IP4_LEN];
	uint32_t factory;
	proto_watch *proto_watch;
	uint32_t state;
	LOCK_T lock;
	network_client_ops *ops;
};

struct __network_client_ops
{
	int32_t (*init)(network_client *c);
	void	(*fin)(network_client *c);

	session *(*create_session)(network_client *c, void *p);
	int32_t (*msg_recv)(network_client *c, msg *m);
	int32_t (*msg_sent)(network_client *c, uint32_t seq);
	void	(*kill)(network_client *c);
	void    (*closed)(network_client *c);
	void    (*error)(network_client *c, int32_t err);
};

network_client *network_client_new(uint32_t size, network_client_ops *ops,
	uint32_t factory, void *io);
network_client *network_client_ref(network_client *nc);
void network_client_kill_unref(network_client *nc);

int32_t network_client_consumable(network_client *nc, uint32_t size);
int32_t network_client_send_msg(network_client *nc, msg *m, uint32_t seq, uint32_t flags);
int32_t network_client_send_mb(network_client *nc, mem_block *mb, uint32_t flags);

END_NAMESPACE

#endif	//__TINY_RAIN_PROTO_CLIENT_H__
