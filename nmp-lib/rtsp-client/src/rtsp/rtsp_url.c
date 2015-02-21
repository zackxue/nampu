/* *
 * This file is part of rtsp-client lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#include <string.h>
#include "rtsp_url.h"

/**
 * Url parse.
 * 	urlname looks like "RTSP://192.168.1.12:554/home/fangyi/media/kiss.mp4".
*/
static __inline__ void RTSP_Url_init(RTSP_Url *url, gchar *urlname)
{
	gchar * protocol_begin, *hostname_begin, *port_begin, *path_begin;
	gsize protocol_len, hostname_len, port_len, path_len;

	hostname_begin = strstr(urlname, "://");
	if (hostname_begin == NULL)
	{	/* "RTSP://" omitted */
		hostname_begin = urlname;
		protocol_begin = NULL;
		protocol_len = 0;
	}
	else
	{
		protocol_len = (gsize)(hostname_begin - urlname);
		hostname_begin = hostname_begin + 3;
		protocol_begin = urlname;
	}

	hostname_len = strlen(urlname) - ((gsize)(hostname_begin - urlname));

	path_begin = strstr(hostname_begin, "/");
	if (path_begin == NULL)
		path_len = 0;
	else
	{
		hostname_len = (gsize)(path_begin - hostname_begin);
		path_len = strlen(urlname) - ((gsize)(path_begin - urlname));
	}

	port_begin = strstr(hostname_begin, ":");
	if ((port_begin == NULL) ||
		((port_begin >= path_begin) && (path_begin != NULL)))
	{
		port_len = 0;
		port_begin = NULL;
	}
	else
	{
		++port_begin;
		if (path_len)
			port_len = (gsize)(path_begin - port_begin);
		else
			port_len = strlen(urlname) - ((gsize)(port_begin - urlname));
		hostname_len = (gsize)(port_begin - hostname_begin - 1);
	}

	if (protocol_len)
		url->protocol = g_strndup(protocol_begin, protocol_len);

	if (port_len)
		url->port = g_strndup(port_begin, port_len);

	if (path_len)
		url->path = g_strndup(path_begin, path_len);

	url->hostname = g_strndup(hostname_begin, hostname_len);
}

void RTSP_Url_destroy(RTSP_Url * url)
{
	g_assert(url != NULL);

	g_free(url->protocol);
	g_free(url->hostname);
	g_free(url->port);
	g_free(url->path);

	g_free(url);
}

RTSP_Url * RTSP_url_parse(gchar *urlname)
{
	RTSP_Url *url;

	url = g_new0(RTSP_Url, 1);
	RTSP_Url_init(url, urlname);

	return url;
}

gboolean RTSP_url_is_complete(RTSP_Url *url)
{
	if (!url)
		return FALSE;

	if (!url->hostname || !url->port || !url->path)
		return FALSE;

	return TRUE;
}


//:~ End
