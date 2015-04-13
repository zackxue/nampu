/*
 * nmp_hlio.c
 *
 * This file implements high level io, payload layer.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include "nmp_hlio.h"
#include "nmp_debug.h"
#include "nmp_errno.h"


static nmp_watch_t *
nmp_hlio_io_create(nmpio_t *io, nmp_conn_t *conn)
{
	nmp_hlio_t *listen_io;

	listen_io = (nmp_hlio_t*)io;
	return (nmp_watch_t*)nmp_hlio_io_new(
		conn,
		io->proto,
		listen_io->proto
	);
}


static void
nmp_hlio_listen_io_on_error(nmpio_t *io, int rw, int why)
{
	nmp_error(
		"Net Listen IO '%p' error, Err: '%d'.\n", io, why
	);

	FATAL_ERROR_EXIT;
}


static nmpio_funcs nmp_high_level_listen_io_funcs =
{
	.create		= nmp_hlio_io_create,
	.error		= nmp_hlio_listen_io_on_error
};


static int
nmp_hlio_io_recv(nmpio_t *io, char *start, size_t size,
	void *from_lower)
{
	int rc;
	nmp_payload_proto_t *proto;
	nmp_hlio_t *hl_io;
	void *msg;

	hl_io = (nmp_hlio_t*)io;
	proto = hl_io->proto;
	BUG_ON(!proto);

	if (!proto->get_payload)
		return -E_PRONOTIMPL;

	if ((msg = (*proto->get_payload)(start, size, from_lower)))
	{
		rc = nmp_watch_recv_message((nmp_watch_t*)hl_io, msg);
		if (rc)
		{
			BUG_ON(!proto->destroy_pointer);
			(*proto->destroy_pointer)(msg, rc);
		}
	}

	return 0;
}


static int
nmp_hlio_io_format(nmpio_t *io, void *msg, char buf[],
	size_t size)
{
	nmp_payload_proto_t *proto;
	nmp_hlio_t *hl_io;

	hl_io = (nmp_hlio_t*)io;
	proto = hl_io->proto;

	BUG_ON(!proto);

	if (proto->put_payload)
	{
		return (*proto->put_payload)(msg, buf, size);
	}

	return 0;
}


static void
nmp_hlio_io_on_error(nmpio_t *io, int rw, int why)
{
	nmp_warning(
		"Net IO '%p' %s error, Err: '%d', report from hl layer.\n",
		io, rw ? "write" : "read", why
	);
}


static void
nmp_hlio_io_on_close(nmpio_t *io, int async)
{
	nmp_print(
		"Net IO '%p' closed%s, report from hl layer.\n",
		io, async ? " by peer" : ""
	);
}


static nmpio_funcs jxj_high_level_io_funcs =
{
	.recv		= nmp_hlio_io_recv,
	.format		= nmp_hlio_io_format,
	.error		= nmp_hlio_io_on_error,
	.close		= nmp_hlio_io_on_close
};


nmp_hlio_t *
nmp_hlio_io_new(nmp_conn_t *conn, nmp_packet_proto_t *ll_proto,
	nmp_payload_proto_t *hl_proto)
{
	nmp_hlio_t *hl_io;
	NMP_ASSERT(conn != NULL && ll_proto != NULL && hl_proto != NULL);

	hl_io = (nmp_hlio_t*)nmp_io_new(
		conn,
		ll_proto,
		&jxj_high_level_io_funcs,
		sizeof(nmp_hlio_t)
	);

	if (hl_io)
	{
		hl_io->proto = hl_proto;
	}

	return hl_io;
}


nmp_hlio_t *
nmp_hlio_listen_io_new(nmp_conn_t *conn, nmp_packet_proto_t *ll_proto,
	nmp_payload_proto_t *hl_proto)
{
	nmp_hlio_t *hl_io;
	NMP_ASSERT(conn != NULL && ll_proto != NULL && hl_proto != NULL);

	hl_io = (nmp_hlio_t*)j_listen_io_new(
		conn,
		ll_proto,
		&nmp_high_level_listen_io_funcs,
		sizeof(nmp_hlio_t)
	);

	if (hl_io)
	{
		hl_io->proto = hl_proto;
	}

	return hl_io;
}


//:~ End
