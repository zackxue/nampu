
/*
 *	author:	zyt
 *	time:	begin in 2012/8/16
 */
#ifndef __RTSP_TRANSPORT_H__
#define __RTSP_TRANSPORT_H__

#include "rtsp_defs.h"
#include "rtsp_mem.h"


typedef enum
{
	RTSP_TRANS_UNKNOWN	= 0,
	RTSP_TRANS_RTP			= (1 << 0)
} RTSP_TRANS_MODE;

typedef enum
{
	RTSP_PROFILE_UNKNOWN	= 0,
	RTSP_PROFILE_AVP		= (1 << 0),
	RTSP_PROFILE_SAVP		= (1 << 1)
} RTSP_PROFILE;

typedef enum
{
	RTSP_LOWER_TRANS_UNKNOWN	= 0,
	RTSP_LOWER_TRANS_UDP			= (1 << 0),
	RTSP_LOWER_TRANS_UDP_MCAST	= (1 << 1),
	RTSP_LOWER_TRANS_TCP			= (1 << 2),
	RTSP_LOWER_TRANS_HTTP		= (1 << 4)
} RTSP_LOWER_TRANS;

typedef struct
{
	int min;
	int max;
} rtsp_range;


typedef struct
{
	RTSP_TRANS_MODE	trans;
	RTSP_PROFILE		profile;
	RTSP_LOWER_TRANS	lower_transport;


	char			*destination;
	uint32		destination_size;
	char			*source;
	uint32		source_size;
	uint32		layers;
	mbool		mode_play;
	mbool		mode_record;
	mbool		append;
	rtsp_range	interleaved;

	/* multicast specific */
	uint32		ttl;

	/* UDP specific */
	rtsp_range	port;
	rtsp_range	client_port;
	rtsp_range	server_port;

	/* RTP specific */
	uint32		ssrc;
} rtsp_transport;


RTSP_RESULT rtsp_transport_new(rtsp_transport **transport);
RTSP_RESULT rtsp_transport_init(rtsp_transport *transport);

RTSP_RESULT rtsp_transport_parse(const char *str, rtsp_transport *transport);
int rtsp_transport_as_text(rtsp_transport *transport, char *str, uint32 str_size);

RTSP_RESULT rtsp_transport_get_mime(RTSP_TRANS_MODE trans, const char **mime);
RTSP_RESULT rtsp_transport_get_manager(RTSP_TRANS_MODE trans, 
	const char **manager, uint32 option);

RTSP_RESULT rtsp_transport_free(rtsp_transport *transport);


#endif

