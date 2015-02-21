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

#ifndef __DEMUXER_H__
#define __DEMUXER_H__

#include "media.h"

struct BufferQueue_Producer;

typedef struct _DemuxerInfo DemuxerInfo;
struct _DemuxerInfo
{
	const gchar *name;	    /* name of demuxer module */
	const gchar *short_name;	/* short name (for config strings) (e.g.:"sd") */
	const gchar *author;		/* author ("Author name & surname <mail>") */
	const gchar *comment;	/* any additional comments */

	/* served file extensions */
	const gchar *extensions; /* coma separated list of extensions (w/o '.') */
};

typedef struct _Demuxer Demuxer;
struct _Demuxer
{
	const DemuxerInfo *info;
	gint (*probe)(const gchar *filename);
	gint (*init)(Resource *);
	gint (*read_packet)(Resource *);	//从文件读，不区分track，全读.
	gint (*seek)(Resource *, gdouble time_sec);
	GDestroyNotify uninit;
/**
 * Extensions:
 *	These are extended for jxj live stream mode. In order to keep push-style,
 *	stream src(parser?) should grab and release 1 reference count of @rtp_s,
 *	which are done in these following function.
*/
	gint (*ctrl_track)(const gchar *mrl, gint cmd, void *value);
	void (*start_track)(struct BufferQueue_Producer *, gchar *uri);		//to start
	void (*stop_track)(struct BufferQueue_Producer *, gchar *uri);		//to stop
	void (*play_track)(struct BufferQueue_Producer *, gchar *url);
	void (*pause_track)(struct BufferQueue_Producer *, gchar *url);
};

const Demuxer *r_find_demuxer(const char *filename);

#endif	/* __DEMUXER_H__ */
