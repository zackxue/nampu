/*
 * j_io.h
 *
 * This file declares low level io, packet layer.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_IO_H__
#define __NMP_IO_H__

#include "nmp_watch.h"
#include "nmp_netproto.h"

typedef struct _JIO JIO;
typedef struct _JIOFuncs JIOFuncs;

struct _JIOFuncs
{
	JWatch *(*create)(JIO *w, JConnection *conn);

	int (*recv)(JIO *io, char *start, size_t size, void *from_lower);
	void (*error)(JIO *io, int rw, int err);
	void (*close)(JIO *io, int async);

	int (*format)(JIO *io, void *msg, char buf[], size_t size);
};


struct _JIO			/* basic IO, packet layer */
{
	JWatch		watch;

	JPacketProto	*proto;
	JIOFuncs		*funcs;

	char			*buffer;

	int			buff_size;
	int			start_pos;
	int			end_pos;
};

#ifdef __cplusplus
extern "C" {
#endif

JIO *j_io_new(JConnection *conn, JPacketProto *proto,
	JIOFuncs *funcs, size_t size);

JIO *j_listen_io_new(JConnection *conn, JPacketProto *proto,
	JIOFuncs *funcs, size_t size);

#ifdef __cplusplus
}
#endif

#endif	//__NMP_IO_H__
