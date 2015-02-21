/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#ifndef __TINY_RAIN_CLIENT_SET_H__
#define __TINY_RAIN_CLIENT_SET_H__

#include "client.h"

BEGIN_NAMESPACE

typedef struct __client_set client_set;
struct __client_set
{
	LOCK_T lock;
	struct list_head c_list;
};

void client_set_init(client_set *cs);
int32_t client_set_add(client_set *cs, client *c);
void client_set_del(client_set *cs, client *c);

END_NAMESPACE

#endif	//__TINY_RAIN_CLIENT_SET_H__
