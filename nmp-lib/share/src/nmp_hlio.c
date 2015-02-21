/*
 * hm_hlio.c
 *
 * This file implements high level io, payload layer.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include "nmp_hlio.h"
#include "nmp_debug.h"
#include "nmp_errno.h"


static HmWatch *
hm_hl_io_create(HmIO *io, HmConnection *conn)
{
	HmHlIO *listen_io;

	listen_io = (HmHlIO*)io;
	return (HmWatch*)hm_hl_io_new(
		conn,
		io->proto,
		listen_io->proto
	);
}


static void
hm_hl_listen_io_on_error(HmIO *io, gint rw, gint why)
{
	hm_error(
		"Net Listen IO '%p' error, Err: '%d'.", NET_IO(io), why
	);

	FATAL_ERROR_EXIT;
}


static HmIOFuncs jxj_high_level_listen_io_funcs =
{
	.create		= hm_hl_io_create,
	.error		= hm_hl_listen_io_on_error
};


static gint
hm_hl_io_recv(HmIO *io, gchar *start, gsize size,
	gpointer from_lower)
{
	gint rc;
	HmPayloadProto *proto;
	HmHlIO *hl_io;
	gpointer msg;

	hl_io = (HmHlIO*)io;
	proto = hl_io->proto;
	BUG_ON(!proto);

	if (!proto->get_payload)
		return -E_PRONOTIMPL;

	if ((msg = (*proto->get_payload)(start, size, from_lower)))
	{
		rc = hm_watch_recv_message((HmWatch*)hl_io, msg);
		if (rc)
		{
			BUG_ON(!proto->destroy_pointer);
			(*proto->destroy_pointer)(msg, rc);
		}
	}

	return 0;
}


static gint
hm_hl_io_format(HmIO *io, gpointer msg, gchar buf[],
	gsize size)
{
	HmPayloadProto *proto;
	HmHlIO *hl_io;

	hl_io = (HmHlIO*)io;
	proto = hl_io->proto;

	BUG_ON(!proto);

	if (proto->put_payload)
	{
		return (*proto->put_payload)(msg, buf, size);
	}

	return 0;
}


static void
hm_hl_io_on_error(HmIO *io, gint rw, gint why)
{
	hm_warning(
		"Net IO '%p' %s error, Err: '%d', report from hl layer.",
		NET_IO(io), rw ? "write" : "read", why
	);
}


static void
hm_hl_io_on_close(HmIO *io, gint async)
{
	hm_print(
		"Net IO '%p' closed%s, report from hl layer.",
		NET_IO(io), async ? " by peer" : ""
	);
}


static HmIOFuncs jxj_high_level_io_funcs =
{
	.recv		= hm_hl_io_recv,
	.format		= hm_hl_io_format,
	.error		= hm_hl_io_on_error,
	.close		= hm_hl_io_on_close
};


HmHlIO *
hm_hl_io_new(HmConnection *conn, HmPacketProto *ll_proto,
	HmPayloadProto *hl_proto)
{
	HmHlIO *hl_io;
	G_ASSERT(conn != NULL && ll_proto != NULL && hl_proto != NULL);

	hl_io = (HmHlIO*)hm_io_new(
		conn,
		ll_proto,
		&jxj_high_level_io_funcs,
		sizeof(HmHlIO)
	);

	if (hl_io)
	{
		hl_io->proto = hl_proto;
	}

	return hl_io;
}


HmHlIO *
hm_hl_listen_io_new(HmConnection *conn, HmPacketProto *ll_proto,
	HmPayloadProto *hl_proto)
{
	HmHlIO *hl_io;
	G_ASSERT(conn != NULL && ll_proto != NULL && hl_proto != NULL);

	hl_io = (HmHlIO*)hm_listen_io_new(
		conn,
		ll_proto,
		&jxj_high_level_listen_io_funcs,
		sizeof(HmHlIO)
	);

	if (hl_io)
	{
		hl_io->proto = hl_proto;
	}

	return hl_io;
}


//:~ End
