/*
 *	author:	zyt
 *	time:	begin in 2012/8/15
 */
#ifndef __RTSP_URL_H__
#define __RTSP_URL_H__

#include "rtsp_mem.h"
#include "rtsp_defs.h"


#define DEF_RTSP_STRING_LEN		(64)

/**
 * RTSP_DEFAULT_PORT:
 *
 * The default RTSP port to connect to.
 */
#define RTSP_DEFAULT_PORT			(554)


typedef struct
{
	uint32	str_len;	//长度包括结束符'\0'
	char		*str;
	char		init[DEF_RTSP_STRING_LEN];
} rtsp_string;


typedef struct
{
	rtsp_string	user;
	rtsp_string	passwd;
	rtsp_string	host;
	uint16		port;
	rtsp_string	abspath;
	rtsp_string	query;
}rtsp_url;


RTSP_RESULT rtsp_url_parse(const char *url_str, rtsp_url **url);

rtsp_url *rtsp_url_copy(const rtsp_url *url);

void rtsp_url_free(rtsp_url *url);

char *rtsp_url_get_request_uri(const rtsp_url *url);

char **rtsp_url_decode_path_components(const rtsp_url *url);

RTSP_RESULT rtsp_url_set_port(rtsp_url *url, uint16 port);

RTSP_RESULT rtsp_url_get_port(const rtsp_url *url, uint16 *port);


#endif

