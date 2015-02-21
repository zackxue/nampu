/*
 * hm_netbuf.h
 *
 * This file describes net ring buffer.
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
 *
*/

#ifndef __HM_NETBUF_H__
#define __HM_NETBUF_H__

#include <glib.h>

#define _PAGE_SIZE				4096
#define MAX_IO_BUFFER_SIZE		(16 * _PAGE_SIZE)		/* Max packet size limited */

G_BEGIN_DECLS

typedef struct _HmNetBuf HmNetBuf;

typedef gint (*HmNetBufFlush)(
	gchar *buf, gsize count, gpointer user_data);

HmNetBuf *hm_net_buf_alloc(gint n_blocks, gint block_size,
	HmNetBufFlush flush);
void hm_net_buf_free(HmNetBuf *buff);

gint hm_net_buf_write(HmNetBuf *buff, gchar *buf, gsize count,
	gpointer user_data, gint *pending);

gint hm_net_buf_flush(HmNetBuf *buff, gpointer user_data);


G_END_DECLS

#endif  //__HM_NETBUF_H__
