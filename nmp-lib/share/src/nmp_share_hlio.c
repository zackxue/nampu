/*
 * nmp_hlio.c
 *
 * This file implements high level io, payload layer.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include "nmp_share_hlio.h"
#include "nmp_share_debug.h"
#include "nmp_share_errno.h"


static NmpWatch *
nmp_hl_io_create(NmpIO *io, NmpConnection *conn)
{
	NmpHlIO *listen_io;

	listen_io = (NmpHlIO*)io;
	return (NmpWatch*)nmp_hl_io_new(
		conn,
		io->proto,
		listen_io->proto
	);
}


static void
nmp_hl_listen_io_on_error(NmpIO *io, gint rw, gint why)
{
	nmp_error(
		"Net Listen IO '%p' error, Err: '%d'.", NET_IO(io), why
	);

	FATAL_ERROR_EXIT;
}


static NmpIOFuncs nmp_high_level_listen_io_funcs =
{
	.create		= nmp_hl_io_create,
	.error		= nmp_hl_listen_io_on_error
};


static gint
nmp_hl_io_recv(NmpIO *io, gchar *start, gsize size,
	gpointer from_lower)
{
	gint rc;
	NmpPayloadProto *proto;
	NmpHlIO *hl_io;
	gpointer msg;

	hl_io = (NmpHlIO*)io;
	proto = hl_io->proto;
	BUG_ON(!proto);

	if (!proto->get_payload)
		return -E_PRONOTIMPL;

	if ((msg = (*proto->get_payload)(start, size, from_lower)))
	{
		rc = nmp_watch_recv_message((NmpWatch*)hl_io, msg);
		if (rc)
		{
			BUG_ON(!proto->destroy_pointer);
			(*proto->destroy_pointer)(msg, rc);
		}
	}

	return 0;
}


static gint
nmp_hl_io_format(NmpIO *io, gpointer msg, gchar buf[],
	gsize size)
{
	NmpPayloadProto *proto;
	NmpHlIO *hl_io;

	hl_io = (NmpHlIO*)io;
	proto = hl_io->proto;

	BUG_ON(!proto);

	if (proto->put_payload)
	{
		return (*proto->put_payload)(msg, buf, size);
	}

	return 0;
}


static void
nmp_hl_io_on_error(NmpIO *io, gint rw, gint why)
{
	nmp_warning(
		"Net IO '%p' %s error, Err: '%d', report from hl layer.",
		NET_IO(io), rw ? "write" : "read", why
	);
}


static void
nmp_hl_io_on_close(NmpIO *io, gint async)
{
	nmp_print(
		"Net IO '%p' closed%s, report from hl layer.",
		NET_IO(io), async ? " by peer" : ""
	);
}


static NmpIOFuncs nmp_high_level_io_funcs =
{
	.recv		= nmp_hl_io_recv,
	.format		= nmp_hl_io_format,
	.error		= nmp_hl_io_on_error,
	.close		= nmp_hl_io_on_close
};


NmpHlIO *
nmp_hl_io_new(NmpConnection *conn, NmpPacketProto *ll_proto,
	NmpPayloadProto *hl_proto)
{
	NmpHlIO *hl_io;
	G_ASSERT(conn != NULL && ll_proto != NULL && hl_proto != NULL);

	hl_io = (NmpHlIO*)nmp_io_new(
		conn,
		ll_proto,
		&nmp_high_level_io_funcs,
		sizeof(NmpHlIO)
	);

	if (hl_io)
	{
		hl_io->proto = hl_proto;
	}

	return hl_io;
}


NmpHlIO *
nmp_hl_listen_io_new(NmpConnection *conn, NmpPacketProto *ll_proto,
	NmpPayloadProto *hl_proto)
{
	NmpHlIO *hl_io;
	G_ASSERT(conn != NULL && ll_proto != NULL && hl_proto != NULL);

	hl_io = (NmpHlIO*)nmp_listen_io_new(
		conn,
		ll_proto,
		&nmp_high_level_listen_io_funcs,
		sizeof(NmpHlIO)
	);

	if (hl_io)
	{
		hl_io->proto = hl_proto;
	}

	return hl_io;
}


//:~ End
