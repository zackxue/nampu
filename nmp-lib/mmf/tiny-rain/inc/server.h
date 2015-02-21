/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_SERVER_H__
#define __TINY_RAIN_SERVER_H__

#include "client_set.h"
#include "listener.h"

BEGIN_NAMESPACE

typedef struct __tr_server tr_server;
struct __tr_server
{
	client_set cs;	/* client set */
	listener *l;	
	JLoopScher *sched;
};

tr_server *tr_create_server(uint32_t factory);
int32_t server_bind_port(tr_server *svr, uint16_t port);
int32_t start_server(tr_server *svr);

END_NAMESPACE

#endif	//__TINY_RAIN_SERVER_H__
