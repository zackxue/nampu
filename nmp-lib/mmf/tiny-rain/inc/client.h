/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_CLIENT_H__
#define __TINY_RAIN_CLIENT_H__

#include "list.h"
#include "msg.h"
#include "session.h"

BEGIN_NAMESPACE

typedef struct __client client;
typedef struct __client_ops client_ops;

struct __client
{
	obj	__super;

	struct list_head list_node;
	struct list_head s_list; //@{session list}
	void *cs;
	void *sched;
	LOCK_T lock; 			//@{session list lock}
	client_ops *ops;
};

struct __client_ops
{
	int32_t (*init)(client *c, void *u);
	void	(*fin)(client *c);
	void	(*kill)(client *c);
	session *(*create_s)(client *c, void *p);
	int32_t (*attach)(client *c, void *loop);
};

client *client_alloc(uint32_t size, client_ops *ops, void *u);
client *client_ref(client *c);
void client_unref(client *c);
void client_kill_unref(client *c);

session *client_create_s(client *c, void *p);
session *client_find_and_get_s(client *c, uint8_t *id);
void client_kill_unref_s(client *c, session *s);

int32_t client_attach(client *c, void *sched);

END_NAMESPACE

#endif	//__TINY_RAIN_CLIENT_H__
