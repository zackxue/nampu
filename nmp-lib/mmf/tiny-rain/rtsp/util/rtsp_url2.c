#include <string.h>
#include "rtsp_url2.h"

/**
 * Url parse.
 * 	urlname looks like "RTSP://192.168.1.12:554/home/fangyi/media/kiss.mp4".
*/
static __inline__ void RTSP_Url_init(RTSP_Url *url, char *urlname)
{
	char * protocol_begin, *hostname_begin, *port_begin, *path_begin;
	uint32_t protocol_len, hostname_len, port_len, path_len;

	hostname_begin = strstr(urlname, "://");
	if (hostname_begin == NULL)
	{	/* "RTSP://" omitted */
		hostname_begin = urlname;
		protocol_begin = NULL;
		protocol_len = 0;
	}
	else
	{
		protocol_len = (uint32_t)(hostname_begin - urlname);
		hostname_begin = hostname_begin + 3;
		protocol_begin = urlname;
	}

	hostname_len = strlen(urlname) - ((uint32_t)(hostname_begin - urlname));

	path_begin = strstr(hostname_begin, "/");
	if (path_begin == NULL)
		path_len = 0;
	else
	{
		hostname_len = (uint32_t)(path_begin - hostname_begin);
		path_len = strlen(urlname) - ((uint32_t)(path_begin - urlname));
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
			port_len = (uint32_t)(path_begin - port_begin);
		else
			port_len = strlen(urlname) - ((uint32_t)(port_begin - urlname));
		hostname_len = (uint32_t)(port_begin - hostname_begin - 1);
	}

	if (protocol_len)
		url->protocol = strndup(protocol_begin, protocol_len);

	if (port_len)
		url->port = strndup(port_begin, port_len);

	if (path_len)
		url->path = strndup(path_begin, path_len);

	url->hostname = strndup(hostname_begin, hostname_len);
}

void RTSP_Url_destroy(RTSP_Url * url)
{
	free(url->protocol);
	free(url->hostname);
	free(url->port);
	free(url->path);
	free(url);
}

RTSP_Url * RTSP_url_parse(char *urlname)
{
	RTSP_Url *url;

	url = malloc(sizeof(*url));
	if (url)
	{
		memset(url, 0, sizeof(*url));
		RTSP_Url_init(url, urlname);
	}

	return url;
}

int RTSP_url_is_complete(RTSP_Url *url)
{
	if (!url || !url->hostname || !url->port || !url->path)
		return 0;

	return 1;
}


//:~ End
