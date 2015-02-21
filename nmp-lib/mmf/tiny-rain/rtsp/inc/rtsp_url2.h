/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __RTSP_URL2_H__
#define __RTSP_URL2_H__		//移到RTSP协议解析库中

#include "def.h"

BEGIN_NAMESPACE

typedef struct _RTSP_Url RTSP_Url;
struct _RTSP_Url
{
	char *protocol;	/* 	RTSP */
	char *hostname;	/* 192.168.1.12 */
	char *port;		/* 554 */
	char *path;		/* /home/fangyi/media/kiss.mp4*/
};

RTSP_Url * RTSP_url_parse(char *urlname);
void RTSP_Url_destroy(RTSP_Url * url);
int RTSP_url_is_complete(RTSP_Url *url);

END_NAMESPACE

#endif	//__RTSP_URL_H__
