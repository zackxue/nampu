/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_LISTENER_H__
#define __TINY_RAIN_LISTENER_H__

#include "client.h"

BEGIN_NAMESPACE

typedef struct __listener listener;
typedef struct __listener_ops listener_ops;

struct __listener
{
	obj obj_base;
	listener_ops *ops;
	void *server;	/* pointer back */
};


struct __listener_ops
{
	int32_t (*init)(listener *l, void *u);
	void 	(*fin)(listener *l);
	int32_t (*set_port)(listener *l, uint16_t port);
	int32_t (*run)(listener *l);
	client* (*new_cli)(listener *l, void *parm);
};


listener *listener_alloc(uint32_t size, listener_ops *ops, void *u);
int32_t listener_bind(listener *l, uint16_t port);
int32_t listener_start(listener *l);
client *listener_generate(listener *l, void *parm);

void listener_set_owner(listener *l, void *p);
void *listener_get_owner(listener *l);

END_NAMESPACE

#endif	//__TINY_RAIN_LISTENER_H__
