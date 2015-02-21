#ifndef __RTSP_URL_H_2__
#define __RTSP_URL_H_2__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _RTSP_Url RTSP_Url;
struct _RTSP_Url
{
	gchar *protocol;	/* 	RTSP */
	gchar *hostname;	/* 192.168.1.12 */
	gchar *port;		/* 554 */
	gchar *path;		/* /home/fangyi/media/kiss.mp4*/
};

RTSP_Url * RTSP_url_parse(gchar *urlname);
void RTSP_Url_destroy(RTSP_Url * url);
gboolean RTSP_url_is_complete(RTSP_Url *url);

G_END_DECLS

#endif	/* __RTSP_URL_H__ */
