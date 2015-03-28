/*
 * nmp_io.h
 *
 * This file declares low level io, packet layer.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_IO_H__
#define __NMP_IO_H__

#include <glib.h>
#include "nmp_watch.h"
#include "nmp_netproto.h"


G_BEGIN_DECLS

typedef struct _NmpIO NmpIO;
typedef struct _NmpIOFuncs NmpIOFuncs;

struct _NmpIOFuncs
{
	NmpWatch *(*create)(NmpIO *w, NmpConnection *conn);

	gint (*recv)(NmpIO *io, gchar *start, gsize size, gpointer from_lower);
	void (*error)(NmpIO *io, gint rw, gint err);
	void (*close)(NmpIO *io, gint async);

	gint (*format)(NmpIO *io, gpointer msg, gchar buf[], gsize size);
	void (*finalize)(NmpIO *io);
};


struct _NmpIO			/* basic IO, packet layer */
{
	NmpWatch		watch;

	NmpPacketProto	*proto;
	NmpIOFuncs		*funcs;

	gchar			*buffer;

	gint			buff_size;
	gint			start_pos;
	gint			end_pos;
};


NmpIO *nmp_io_new(NmpConnection *conn, NmpPacketProto *proto,
	NmpIOFuncs *funcs, gsize size);

NmpIO *nmp_listen_io_new(NmpConnection *conn, NmpPacketProto *proto,
	NmpIOFuncs *funcs, gsize size);

G_END_DECLS

#endif	//__NMP_IO_H__
