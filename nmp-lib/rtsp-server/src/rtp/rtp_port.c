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

#include "mediautils.h"
#include "rtp.h"
#include "ports.h"


gint get_port_pair(void *srv, port_pair * pair)
{
	if (!jpf_media_ports_get_pair(&pair->RTP, &pair->RTCP))
		return ERR_NOERROR;

	return ERR_NOERROR;
}


gint put_port_pair(void *srv, port_pair * pair)
{
	jpf_media_ports_put_pair(pair->RTP, pair->RTCP);
	return ERR_NOERROR;
}

//:~ End
