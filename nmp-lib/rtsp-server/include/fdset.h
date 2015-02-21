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

#ifndef __FD_SET_H__
#define __FD_SET_H__

typedef struct _FD_set FD_set;
typedef void (*fd_callback_t)(gint fd, gpointer user_data);


FD_set *fd_set_new( void );
void fd_set_insert(FD_set *set, gint fd, gpointer user_data,
	fd_callback_t call, fd_callback_t fin);
gboolean fd_set_remove(FD_set *set, gint fd);
void fd_set_call(FD_set *set);
void fd_set_free(FD_set *set);

#endif	/* __FD_SET_H__ */
