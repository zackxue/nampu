/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_SESSION_H__
#define __TINY_RAIN_SESSION_H__

#include <string.h>
#include "obj.h"
#include "list.h"

BEGIN_NAMESPACE

#define SESS_MAX_LEN			32

typedef struct __session session;
typedef struct __session_ops session_ops;

struct __session
{
	obj __super;

	int32_t state;
	struct list_head list;
	uint8_t id[SESS_MAX_LEN];
	void *client;
	void *media_sinker;
	LOCK_T lock;
	session_ops *ops;
};

struct __session_ops
{
	int32_t (*init)(session *s, void *u);
	void	(*fin)(session *s);
	void	(*kill)(session *s);
	int32_t (*ready)(session *s);
};

session *session_alloc(uint32_t size, session_ops *ops, void *u);
int32_t session_add(session *s, void *client);
int32_t session_del(session *s, void *client);

session *session_ref(session *s);
void session_unref(session *s);
void session_kill_unref(session *s);

int32_t session_add_sinker(session *s, void *sinker);
int32_t session_ready(session *s);
int32_t session_ctl(session *s, int32_t cmd, void *data);

int32_t session_set_sinker_config(session *s, void *in, void *data);
int32_t session_get_sinker_config(session *s, void *in, void *data);

uint8_t *session_id(session *s);
int32_t session_equal(session *s, uint8_t *id);

END_NAMESPACE

#endif	//__TINY_RAIN_SESSION_H__
