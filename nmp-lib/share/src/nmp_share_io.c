/*
 * nmp_io.c
 *
 * This file implements low level io, packet layer.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include <string.h>
#include "nmp_share_debug.h"
#include "nmp_share_errno.h"
#include "nmp_share_io.h"
#include "nmp_share_fragments.h"


static __inline__ gint
nmp_io_recv_packet(NmpIO *io, gchar *buf, gsize size);


static __inline__ void
__nmp_io_finalize(NmpIO *io);


static NmpWatch *
nmp_io_create(NmpWatch *w, NmpConnection *conn)
{
	NmpIOFuncs *funcs;
	NmpIO *io = (NmpIO*)w;

	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->create)
	{
		return (*funcs->create)(io, conn);
	}

	nmp_connection_close(conn);
	return NULL;
}


static void
nmp_io_on_listen_error(NmpWatch *w, gint rw, gint why)
{
	NmpIOFuncs *funcs;
	NmpIO *io = (NmpIO*)w;

	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->error)
	{
		(*funcs->error)(io, rw, why);
	}
}


static void
nmp_io_finalize(NmpWatch *w)
{
	NmpIO *io = (NmpIO*)w;

	if (io->funcs && io->funcs->finalize)
	{
		(*io->funcs->finalize)(io);
	}

	__nmp_io_finalize(io);
}


static NmpWatchFuncs nmp_listen_io_watch_funcs =
{
	.create		= nmp_io_create,
	.error		= nmp_io_on_listen_error,
	.finalize	= nmp_io_finalize
};


static gint
nmp_io_recv_data(NmpWatch *w, gchar *buf, gsize size)
{
	NmpIO *io = (NmpIO*)w;

	return nmp_io_recv_packet(io, buf, size);
}


static gint
nmp_io_format_data(NmpWatch *w, gpointer msg, gchar buf[],
	gsize size)
{
	NmpIOFuncs *funcs;
	NmpPacketProto *proto;
	gint pack_head_len, phl, payload_len = 0;
	NmpIO *io = (NmpIO*)w;

	proto = io->proto;
	BUG_ON(!proto);

	funcs = io->funcs;
	BUG_ON(!funcs);

	if (!proto->pack)
		return 0;

	pack_head_len = (*proto->pack)(msg, -1, buf, size);
	if (pack_head_len < 0)
		return pack_head_len;

	BUG_ON(pack_head_len >= size);

	if (funcs->format)
	{
		payload_len = (*funcs->format)(
			io, msg, buf + pack_head_len, size - pack_head_len);
		if (payload_len < 0)
			return payload_len;

		phl = (*proto->pack)(msg, payload_len, buf, pack_head_len);
		if (phl < 0)
			return phl;

		BUG_ON(phl > pack_head_len);

		if (phl != pack_head_len)
			memcpy(buf + phl, buf + pack_head_len, payload_len);

		pack_head_len = phl;
	}

	return pack_head_len + payload_len;
}


static void
nmp_io_on_error(NmpWatch *w, gint rw, gint why)
{
	NmpIO *io;
	NmpIOFuncs *funcs;

	G_ASSERT(w != NULL);

	io = (NmpIO*)w;
	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->error)
	{
		(*funcs->error)(io, rw, why);
	}
}


static void
nmp_io_on_close(NmpWatch *w, gint async)
{
	NmpIO *io;
	NmpIOFuncs *funcs;

	G_ASSERT(w != NULL);

	io = (NmpIO*)w;
	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->close)
	{
		(*funcs->close)(io, async);
	}
}


static NmpWatchFuncs nmp_io_watch_funcs =
{
	.recv		= nmp_io_recv_data,
	.format		= nmp_io_format_data,
	.error		= nmp_io_on_error,
	.close		= nmp_io_on_close,
	.finalize	= nmp_io_finalize
};


static __inline__ gint
nmp_io_initialize(NmpIO *io, gint listen)
{
	gint buffer_size;
	NmpWatch *watch = (NmpWatch*)io;

	if (listen)
	{
		io->buffer = NULL;
		io->buff_size = 0;
		io->start_pos = 0;
		io->end_pos = 0;
	}
	else
	{
		buffer_size = watch->block_size;
		if (buffer_size <= 0)
			buffer_size = MAX_IO_BUFFER_SIZE;
		io->buffer = g_malloc(buffer_size);
		io->buff_size = buffer_size;
		io->start_pos = 0;
		io->end_pos = 0;
	}

	return 0;
}


static __inline__ void
__nmp_io_finalize(NmpIO *io)
{
	if (io->buffer)
	{
		g_free(io->buffer);
		io->buffer = NULL;
	}
};


__export NmpIO *
nmp_io_new(NmpConnection *conn, NmpPacketProto *proto, 
	NmpIOFuncs *funcs, gsize size)
{
	NmpIO *io;
	G_ASSERT(conn != NULL && proto != NULL && funcs != NULL);

	if (nmp_connection_is_blocked(conn))
	{
		nmp_warning(
			"Net create io on blocked connection '%p'.",
			conn
		);
		return NULL;		
	}

	io = (NmpIO*)nmp_watch_create(
		conn, &nmp_io_watch_funcs, size);
	if (!io)
	{
		nmp_warning(
			"Net create watch failed."
		);
		return NULL;
	}

	nmp_io_initialize(io, 0);

	io->proto = proto;
	io->funcs = funcs;

	return io;
}


__export NmpIO *
nmp_listen_io_new(NmpConnection *conn, NmpPacketProto *proto,
	NmpIOFuncs *funcs, gsize size)
{
	NmpIO *io;
	G_ASSERT(conn != NULL && proto != NULL && funcs != NULL);

	io = (NmpIO*)nmp_listen_watch_create(
		conn, &nmp_listen_io_watch_funcs, size);
	if (!io)
	{
		nmp_warning(
			"Net create watch failed."
		);
		return NULL;
	}

	nmp_io_initialize(io, 1);

	io->proto = proto;
	io->funcs = funcs;

	return io;
}


static __inline__ gint
nmp_io_packet_proto_check(NmpIO *io)
{
	NmpPacketProto *proto;
	gint effective, ret;
    NmpNetPackInfo payload_raw, *npi;
    NmpIOFuncs *funcs;

	proto = io->proto;
	BUG_ON(!proto);

	funcs = io->funcs;
	BUG_ON(!funcs);

    while( TRUE )
    {
        effective = (*proto->check)(
        	&io->buffer[io->start_pos],
            &io->buffer[io->end_pos]
        );

        if (effective > 0)
        {
            BUG_ON(effective > io->end_pos - io->start_pos);

            ret = (*proto->unpack)(
            	&io->buffer[io->start_pos],
                &io->buffer[io->start_pos] + effective,
                &payload_raw
            );

            if (G_UNLIKELY(ret))
                return ret;

			npi = nmp_net_packet_defrag(&payload_raw);
			if (G_LIKELY(npi))
			{
				if (funcs->recv)
				{
					ret = (*funcs->recv)(
						io,
						npi->start,
						npi->size,
						npi->private_from_low_layer
					);
				}

				if (G_UNLIKELY(npi != &payload_raw))
					nmp_net_packet_release_npi(npi);

				if (G_UNLIKELY(ret))
					return ret;

				if (G_UNLIKELY(!io->buffer))
					return -E_PACKET;
			}

            io->start_pos += effective;

			if (io->start_pos >= io->end_pos)
			{
				io->start_pos = io->end_pos = 0;
				return 0;
			}

#ifdef __CPU_FORCE_ADDRESS_ALIGN
			if (io->start_pos % sizeof(void*))
			{
				memcpy(&io->buffer[0],
	            	&io->buffer[io->start_pos],
	            	io->end_pos - io->start_pos);			

				io->end_pos -= io->start_pos;
				io->start_pos = 0;				
			}
#endif
            continue;
        }
        else
        {
            if (effective < 0)
                return -E_PACKET;

			if (io->start_pos)
			{
				if (io->end_pos - io->start_pos > 0)
				{
					memcpy(&io->buffer[0],
	                	&io->buffer[io->start_pos],
	                	io->end_pos - io->start_pos);
	            }
                io->end_pos -= io->start_pos;
                io->start_pos = 0;
			}

            return 0;
        }
    }

    return -E_PACKET;	
}


static __inline__ gint
nmp_io_recv_packet(NmpIO *io, gchar *buf, gsize size)
{
	gint ret, left;

	while (size > 0)
	{
		left = io->buff_size - io->end_pos;
		if (left <= 0)
			return -E_PACKET;

		if (size < left)
		{
			memcpy(&io->buffer[io->end_pos], buf, size);
			io->end_pos += size;
			size = 0;
		}
		else
		{
			memcpy(&io->buffer[io->end_pos], buf, left);
			io->end_pos += left;
			size -= left;
			buf += left;
		}

		if (G_UNLIKELY((ret = nmp_io_packet_proto_check(io))))
			return ret;
	}

	return 0;	
}


//:~ End
