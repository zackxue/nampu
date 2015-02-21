/* *
 * This file is part of rtsp-server
 *
 * Copyright (C) 2012 fangyi <fangyi@szjxj.net>
 * See COPYING for more details
 *
 * rtsp-server is a stream transporting and controlling module using
 * RTP/RTCP/RTSP protocols, which is designed for jxj platform servers
 * such as mss, proxy-server etc.
 *
 * */
 
#ifndef __RTSP_H__
#define __RTSP_H__

#include <netembryo/wsocket.h>
#include <time.h>
#include <glib.h>
#include <rtspparse.h>

#include "evsched.h"
#include "macros.h"

struct Resource;		/* forward declaration */

typedef enum			/* states of: rtsp state machine */
{
    RTSP_SERVER_INIT,
    RTSP_SERVER_READY,
    RTSP_SERVER_PLAYING,
    RTSP_SERVER_RECORDING
}RTSP_Server_State;


typedef struct RTSP_Range
{
	gdouble begin_time;			/* 该段的流NTP起始时间 */
	gdouble end_time;			/* 该段的流NTP终止时间 */
	gdouble playback_time;		/* 开始播放的实时时间 */
}RTSP_Range;


typedef struct _RTSP_Ps RTSP_Ps;	/* rtsp parser state (information) */
struct _RTSP_Ps
{
	GstRTSPMessage      *request;
	GstRTSPMethod        method;
	GstRTSPUrl			*url;
	GstRTSPMessage      *response;	
};


typedef struct _RTSP_session RTSP_session;
struct _RTSP_session
{
	gint ref_count;
	gchar *session_id;
	gint timeout;	/* timeout seconds */

	GMutex *mutex;
	gboolean killed;	/* async killed */

    RTSP_Server_State cur_state;
    time_t session_timer;
    gint started;
    GSList *rtp_sessions; /* RTP_session list */
    struct Resource *resource;
    gchar *resource_uri;	/* resouce uri given, eg: "/media/test.mov" in "rtsp://127.0.0.1:554/media/test.mov" */
   		/* mrl means "media real locaton", a certain prefix directory + resource_uri */

    /**
     * @brief List of playback requests (of type @ref RTSP_Range)
     *
     * RFC 2326 Section 10.5 defines queues of PLAY requests, which
     * allows precise editing by the client; we keep a list here to
     * make it easier to actually implement the (currently missing)
     * feature.
     */
    GQueue *play_requests;

    POINTER_BACK_TO(RTSP_Client)
};


typedef struct _RTSP_Client RTSP_Client;
struct _RTSP_Client
{
	GEvent base;	/* event base object */
    Sock *sock;

	GMutex *mutex;

	gboolean	killed;	/* async killed */
	GstRtspParser *rtsp_parser;	/* rtsp protocol parser */
    /**
     * @brief Output buffer
     *
     * This is the output buffer as read straight from the sock socket.
     */
    GQueue *output;
    guint8 *write_buffer;
	gsize write_off;
	gsize write_size;
	gsize backlog;		/* backlog in queue */

	guint rec_id;	/* writing rec id generator */

	gint err_code;	/* err_code */
    GList *session_list;

    /**
     * @brief Interleaved setup data
     *
     * Singly-linked list of @ref RTSP_interleaved instances, one per
     * RTP session linked to the RTSP session.
     */
    GSList *interleaved;

	GEventDispath rw_func;	/* on read/write */
	GEventFinalize fin_func;	/* on finalize */

	POINTER_BACK_TO(Listen_watch)
};

RTSP_Client *rtsp_client_new(Sock *client_sock);
RTSP_Client *rtsp_client_ref(RTSP_Client *client);
void rtsp_client_unref(RTSP_Client *client);

RTSP_session *rtsp_client_session_find(RTSP_Client *client, gchar *sid);
void rtsp_client_session_remove(RTSP_Client *client, RTSP_session *rtsp_s);
void rtsp_client_remove_dead_session(RTSP_Client *client);

gint rtsp_client_would_block(RTSP_Client *client, gsize size);

gboolean rtsp_client_default_rw_cb(GEvent *ev, gint revents, void *user_data);
void rtsp_client_default_finalize(GEvent *ev);

RTSP_session *__rtsp_client_session_new(RTSP_Client *client);
RTSP_Client *rtsp_client_new_base(gsize size, Sock *sock, 
	gint events, gint timeout);

void rtsp_session_kill(RTSP_session *rtsp_s);
void rtsp_client_kill(RTSP_Client *client);
void rtsp_client_orphan(RTSP_Client *client);
RTSP_session *rtsp_session_ref(RTSP_session *rtsp_s);
void rtsp_session_unref(RTSP_session *rtsp_s);

void rtsp_client_send_request(RTSP_Client *client, GstRTSPMessage *request);
void rtsp_client_send_response(RTSP_Client *client, RTSP_session *session,
	GstRTSPMessage *response);
void rtsp_client_send_generic_response(RTSP_Client *client, GstRTSPStatusCode code,
	RTSP_Ps *state);

static __inline__ gint rtsp_client_errno(RTSP_Client *client)
{
	return client->err_code;
}

#endif	/* __RTSP_H__ */
