/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_LISTENER_TEMPLATE_H__
#define __TINY_RAIN_LISTENER_TEMPLATE_H__

#include "nmplib.h"
#include "listener.h"

BEGIN_NAMESPACE

typedef struct __listener_tmpl listener_tmpl;
typedef struct __listener_tmpl_ops listener_tmpl_ops;

struct __listener_tmpl
{
	listener obj_base;		/* private inheritance */

	uint16_t port;
	uint32_t host;

	int32_t sock;
	JEvent *event;

	listener_tmpl_ops *ops;
	void *user_data;
};


struct __listener_tmpl_ops
{
	int32_t (*init)(listener_tmpl *lt);
	void	(*fin)(listener_tmpl *lt);

	client* (*new_cli)(listener_tmpl *lt, int32_t sock);
};


listener *alloc_listener_tmpl(listener_tmpl_ops *ops);

END_NAMESPACE

#endif	//__TINY_RAIN_LISTENER_TEMPLATE_H__
