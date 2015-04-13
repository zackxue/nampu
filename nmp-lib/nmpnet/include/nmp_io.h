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

#include "nmp_watch.h"
#include "nmp_netproto.h"

typedef struct _nmpio nmpio_t;
typedef struct _nmpio_funcs nmpio_funcs;

struct _nmpio_funcs
{
	nmp_watch_t *(*create)(nmpio_t *w, nmp_conn_t *conn);

	int (*recv)(nmpio_t *io, char *start, size_t size, void *from_lower);
	void (*error)(nmpio_t *io, int rw, int err);
	void (*close)(nmpio_t *io, int async);

	int (*format)(nmpio_t *io, void *msg, char buf[], size_t size);
};


struct _nmpio			/* basic IO, packet layer */
{
	nmp_watch_t		watch;

	nmp_packet_proto_t	*proto;
	nmpio_funcs		*funcs;

	char			*buffer;

	int			buff_size;
	int			start_pos;
	int			end_pos;
};

#ifdef __cplusplus
extern "C" {
#endif

nmpio_t *nmp_io_new(nmp_conn_t *conn, nmp_packet_proto_t *proto,
	nmpio_funcs *funcs, size_t size);

nmpio_t *j_listen_io_new(nmp_conn_t *conn, nmp_packet_proto_t *proto,
	nmpio_funcs *funcs, size_t size);

#ifdef __cplusplus
}
#endif

#endif	//__NMP_IO_H__
