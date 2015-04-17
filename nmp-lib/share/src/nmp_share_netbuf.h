/*
 * nmp_share_netbuf.h
 *
 * This file describes net ring buffer.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
 *
*/

#ifndef __NMP_NETBUF_H__
#define __NMP_NETBUF_H__

#include <glib.h>

#define _PAGE_SIZE				4096
#define MAX_IO_BUFFER_SIZE		(16 * _PAGE_SIZE)		/* Max packet size limited */

G_BEGIN_DECLS

typedef struct _NmpNetBuf NmpNetBuf;

typedef gint (*NmpNetBufFlush)(
	gchar *buf, gsize count, gpointer user_data);

NmpNetBuf *nmp_net_buf_alloc(gint n_blocks, gint block_size,
	NmpNetBufFlush flush);
void nmp_net_buf_free(NmpNetBuf *buff);

gint nmp_net_buf_write(NmpNetBuf *buff, gchar *buf, gsize count,
	gpointer user_data, gint *pending);

gint nmp_net_buf_flush(NmpNetBuf *buff, gpointer user_data);


G_END_DECLS

#endif  //__NMP_NETBUF_H__
