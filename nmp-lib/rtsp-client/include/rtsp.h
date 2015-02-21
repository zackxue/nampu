/* *
 * This file is part of rtsp-client lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#ifndef __CLIENT_RTSP_H__
#define __CLIENT_RTSP_H__

#include <glib.h>
#include <netembryo/wsocket.h>
#include <rtspwatch.h>
#include "media.h"
#include "usrcallback.h"
#include "rtsp_url.h"


typedef GstRTSPResult (*RTSP_state_fun)(RTSP_client_session *rtsp_s, GstRTSPMethod method, 
	GstRTSPMessage *message);

typedef enum 		/* RTSP state machine */
{
	RTSP_INIT = 0,
	RTSP_READY,
	RTSP_PLAYING,
	RTSP_RECORDING,
	STATES_NUM
}RTSP_state;


typedef struct _RTSP_wait_for
{
	GstRTSPMethod	method;		/* waiting response */
	gint		seq;
	gchar		*session_id;
}RTSP_wait_for;


struct _RTSP_client_session
{
	gint			ref_count;
	gint			killed;			/* session killed? for async controlling */

	gchar			*session_id;
	RTSP_Url		*url;
	gint			timeout;		/* session timeout */

	RTSP_media		*medium;

	gint			rtsp_seq;		/* seq generator */
	gint			rtp_over_rtsp;	/* stream mode */
	gint			interleaved;

	gboolean		play_pending;	/* play request is pending */
	gdouble			range_start;	/* ntp=range_start-range_end */
	gdouble			range_end;		/* ntp=range_start-range_end */

	RTSP_state	state;				/* machine state */
	RTSP_wait_for	wait_for;
	Usr_Callback	usr_cb;			/* user callback data block */

	gint			rtsp_sock;		/* rtsp connction socket */
	gint			ev_timer_counter;

	GEvent			*ev_timer;
	GstRtspWatch	*rtsp_watch;	/* rtsp watch */

	GMutex			*s_mutex;		/* session lock */
};

#define INVOKE_DAT_HOOK_FRA(rtsp, fr, re) \
{\
	if ((rtsp)->usr_cb.dat_cb)\
		(*((rtsp)->usr_cb.dat_cb))((rtsp)->usr_cb.u, (fr), NULL, (re)); \
}

#define INVOKE_DAT_HOOK_CNF(rtsp, cnf) \
{\
	if ((rtsp)->usr_cb.dat_cb)\
		(*((rtsp)->usr_cb.dat_cb))((rtsp)->usr_cb.u, NULL, cnf, NULL); \
}

RTSP_client_session *rtsp_client_session_new( void );
void rtsp_session_kill_sync(RTSP_client_session *rtsp_s);	/* invoked with client->lock held */

GstRTSPResult rtsp_sesseion_send_message(RTSP_client_session *rtsp_s,
	GstRTSPMessage *message);

RTSP_client_session *rtsp_client_session_ref(RTSP_client_session *rtsp_s);
void rtsp_client_session_unref(RTSP_client_session *rtsp_s);

gint rtsp_session_lock(RTSP_client_session *rtsp_s);
void rtsp_session_unlock(RTSP_client_session *rtsp_s);

gint rtsp_session_get_interleaved(RTSP_client_session *rtsp_s);

#endif	/* __CLIENT_RTSP_H__ */

