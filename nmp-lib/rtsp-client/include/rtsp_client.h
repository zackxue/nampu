/* *
 * This file is part of rtsp-client lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#ifndef __RTSP_CLIENT_H__
#define __RTSP_CLIENT_H__

#include <glib.h>

#define SDP_ATTR_SIZE		128

typedef struct _RTSP_client_session RTSP_client_session;

typedef enum
{
	L4_TCP,
	L4_UDP
}L4_Proto;

enum
{
	STM_VIDEO,
	STM_AUDIO,
	STM_OTHER
};

typedef struct _RTP_Frame RTP_Frame;
struct _RTP_Frame		/* RTP frame */
{
	guint32		stm;
	guint32		ts;
	guint8		pt;
	guint8		*data;
	gsize		len;
};

typedef struct _RTP_Cnf RTP_Cnf;
struct _RTP_Cnf		/* Configuration data, such as profile-level-id, sps/pps from sdp. */
{
	gint		stm;
	gchar		sps[SDP_ATTR_SIZE];	/* sprop-parameter-sets */
	gchar		pli[SDP_ATTR_SIZE];	/* profile-level-id */
	gchar		pri[SDP_ATTR_SIZE];	/* private */
	gsize		sps_len;
	gsize		pli_len;
	gsize		pri_len;
};

typedef struct _RTP_Ext RTP_Ext;
struct _RTP_Ext		/* Rtp extension data */
{
};

typedef void (*Exp_Callback)(void *u, gint why);
typedef void (*Data_Callback)(void *u, RTP_Frame *fr, RTP_Cnf *rc, RTP_Ext *re);

//INTERFACES
void rtsp_client_init( void );
void rtsp_client_fin( void );

RTSP_client_session *rtsp_client_create_session( void );

void rtsp_client_set_data_mode(RTSP_client_session *session, gint mode);
void rtsp_client_set_callback(RTSP_client_session *session, Exp_Callback ecb,
	Data_Callback dcb, void *u);

gint rtsp_client_open_session(RTSP_client_session *session, const gchar *url, L4_Proto l4);
gint rtsp_client_play_session(RTSP_client_session *session, gdouble start, gdouble end);
gint rtsp_client_pause_session(RTSP_client_session *session);
gint rtsp_client_close_session(RTSP_client_session *session);

#endif	/* __RTSP_CLIENT_H__ */

