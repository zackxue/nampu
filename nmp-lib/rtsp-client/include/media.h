/* *
 * This file is part of rtsp-client lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#ifndef __RTSP_MEDIA_H__
#define __RTSP_MEDIA_H__

#include <glib.h>
#include "rtp.h"
#include "nmpevsched.h"
#include "rtspparse.h"
#include "usrcallback.h"

#define CONFIG_RTCP_SUPPORT 

typedef struct _RTP_Parser RTP_Parser;
typedef struct _RTP_Session RTP_Session;

enum R_Type
{
	R_EV_RTP,
	R_EV_RTCP
};

typedef struct _R_Event R_Event;
struct _R_Event
{
	GEvent			event_base;	/* event base object */
	enum R_Type		type;		/* event type */
	RTP_Session		*rtp_s;
};


typedef struct _RTP_Ssrc RTP_Ssrc;
struct _RTP_Ssrc
{
	rtp_ssrc_stats	state;
	RTP_Ssrc		*next;
};


struct _RTP_Session
{
	gint			ref_count;	/* reference counting */
	gint			rtp_killed;	/* rtp session state */

	gint			interleaved;	/* interleaved mode */

	gint			rtp_port;	/* local rtp port */
	gint			rtp_sock;	/* rtp sock fd */
	R_Event			*rtp_src;	/* rtp packet source */

#ifdef CONFIG_RTCP_SUPPORT
	gint			rtcp_port;	/* local rtcp port */
	gint			rtcp_sock;	/* rtcp sock fd */
	R_Event			*rtcp_src;	/* rtcp packet source */
#endif

	gint			stm_type;	/* stream type, audio/video */
	GstSDPMedia		*m;			/* SDP media info */

	RTP_Parser		*parser;	/* RTP parser , fix me! only one pt can be handled */
	RTP_Ssrc		*ssrc;		/* sync src list */
	rtp_session_state state;	/* session state */
	gint			pt;			/* payload type, one type each session */

	RTP_Ssrc		rtp_ssrc_init;	/* only 1 supported now */
	void			*rtsp_s;	/* rtsp session we belong to */
};

typedef struct _RTSP_media RTSP_media;
struct _RTSP_media
{
	gchar			*content_base;	/* media content-base */
	GList			*rtp_sessions;

	GstSDPMessage 	*sdp;			/* sdp info */
	gint			ntrack;			/* how many tracks we have? */
	gint			request_itrack;
	void			*rtsp_s;		/* rtsp session we belong to */
};

RTP_Session *rtsp_media_create_rtp_session(RTSP_media *medium, const GstSDPMedia *m);

void rtp_session_port(RTP_Session *rtp_s, gint *rtp, gint *rtcp);

RTSP_media *rtsp_media_create( void );
void rtsp_media_free(RTSP_media *medium);

gint rtsp_session_media_interleaved_rcv(RTSP_media *media, guint8 channel,
	guint8* data, guint size);

#endif	/* __RTSP_MEDIA_H__ */
