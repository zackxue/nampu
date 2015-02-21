/* *
 * This file is part of rtsp-client lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#include <glib.h>
#include <string.h>
#include "fnc_log.h"
#include "rtsp_client.h"
#include "nmpevsched.h"
#include "rtsp.h"
#include "media.h"
#include "ports.h"


#define N_CPU_COUNT		2

typedef struct _RTSP_Client RTSP_Client;
struct _RTSP_Client
{
	GMutex	*lock;
	GList	*list;			/* RTSP session list */
};

static RTSP_Client *client_single = NULL;

static __inline__ RTSP_Client *
rtsp_client_new( void )
{
	RTSP_Client *rc;

	rc = g_new0(RTSP_Client, 1);
	rc->lock = g_mutex_new();
	rc->list = NULL;

	return rc;
}


static __inline__ void
rtsp_client_delete(RTSP_Client *rc)
{
	g_assert(rc && !rc->list);

	g_mutex_free(rc->lock);
	g_free(rc);
}


static __inline__ void
__rtsp_client_cleanup(RTSP_Client *rc)
{
	RTSP_client_session *rtsp_s;
	GList *list;

	while (rc->list)
	{
		list = rc->list;
		rtsp_s = list->data;
		rc->list = g_list_delete_link(rc->list, list);
		g_mutex_unlock(rc->lock);

		rtsp_session_kill_sync(rtsp_s);

		g_mutex_lock(rc->lock);
	}
}


static __inline__ void
rtsp_client_cleanup(RTSP_Client *rc)
{
	g_mutex_lock(rc->lock);
	__rtsp_client_cleanup(rc);
	g_mutex_unlock(rc->lock);
}


static __inline__ gint
rtsp_client_del_session(RTSP_Client *rc, RTSP_client_session *rtsp_s)
{
	gint found = 0;
	GList *list;

	g_mutex_lock(rc->lock);
	list = g_list_find(rc->list, rtsp_s);
	if (list)
	{
		rc->list = g_list_delete_link(rc->list, list);
		found = 1;
	}
	g_mutex_unlock(rc->lock);

	return found;
}


void rtsp_client_init( void )		/* only once! */
{
	if (!client_single)
	{
		client_single = rtsp_client_new();
		jpf_media_ports_set_range(25000, 30000);
		g_scheduler_init(N_CPU_COUNT * 2);
	}
}


void rtsp_client_fin( void )
{
	if (client_single)
	{
		rtsp_client_cleanup(client_single);
		client_single = NULL;
	}
}


static __inline__ void
rtsp_client_add_session(RTSP_Client *rc, RTSP_client_session *rtsp_s)
{
	g_mutex_lock(rc->lock);
	rc->list = g_list_prepend(rc->list, rtsp_s);
	g_mutex_unlock(rc->lock);	
}


RTSP_client_session *rtsp_client_create_session( void )
{
	RTSP_client_session *rtsp_s;

	if (!client_single)
		return NULL;

	rtsp_s = rtsp_client_session_new();
	if (rtsp_s)
	{
		rtsp_client_add_session(client_single, rtsp_s);
	}

	return rtsp_s;
}


gint rtsp_client_close_session(RTSP_client_session *session)
{
	if (!session || !client_single)
		return -1;

	if (!rtsp_client_del_session(client_single, session))
		return -1;

	rtsp_session_kill_sync(session);
	return 0;
}


//:~ End
