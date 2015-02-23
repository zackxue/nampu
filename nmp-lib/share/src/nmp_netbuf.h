/*
 * jpf_netbuf.h
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

typedef struct _JpfNetBuf JpfNetBuf;

typedef gint (*JpfNetBufFlush)(
	gchar *buf, gsize count, gpointer user_data);

JpfNetBuf *jpf_net_buf_alloc(gint n_blocks, gint block_size,
	JpfNetBufFlush flush);
void jpf_net_buf_free(JpfNetBuf *buff);

gint jpf_net_buf_write(JpfNetBuf *buff, gchar *buf, gsize count,
	gpointer user_data, gint *pending);

gint jpf_net_buf_flush(JpfNetBuf *buff, gpointer user_data);


G_END_DECLS

#endif  //__NMP_NETBUF_H__
