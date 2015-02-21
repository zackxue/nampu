/*
 * nmp_io.c
 *
 * This file implements low level io, packet layer.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include <string.h>
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_io.h"
#include "nmp_fragments.h"


static __inline__ gint
hm_io_recv_packet(HmIO *io, gchar *buf, gsize size);


static __inline__ void
__hm_io_finalize(HmIO *io);


static HmWatch *
hm_io_create(HmWatch *w, HmConnection *conn)
{
	HmIOFuncs *funcs;
	HmIO *io = (HmIO*)w;

	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->create)
	{
		return (*funcs->create)(io, conn);
	}

	hm_connection_close(conn);
	return NULL;
}


static void
hm_io_on_listen_error(HmWatch *w, gint rw, gint why)
{
	HmIOFuncs *funcs;
	HmIO *io = (HmIO*)w;

	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->error)
	{
		(*funcs->error)(io, rw, why);
	}
}


static void
hm_io_finalize(HmWatch *w)
{
	HmIO *io = (HmIO*)w;

	if (io->funcs && io->funcs->finalize)
	{
		(*io->funcs->finalize)(io);
	}

	__hm_io_finalize(io);
}


static HmWatchFuncs hm_listen_io_watch_funcs =
{
	.create		= hm_io_create,
	.error		= hm_io_on_listen_error,
	.finalize	= hm_io_finalize
};


static gint
hm_io_recv_data(HmWatch *w, gchar *buf, gsize size)
{
	HmIO *io = (HmIO*)w;

	return hm_io_recv_packet(io, buf, size);
}


static gint
hm_io_format_data(HmWatch *w, gpointer msg, gchar buf[],
	gsize size)
{
	HmIOFuncs *funcs;
	HmPacketProto *proto;
	gint pack_head_len, phl, payload_len = 0;
	HmIO *io = (HmIO*)w;

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
hm_io_on_error(HmWatch *w, gint rw, gint why)
{
	HmIO *io;
	HmIOFuncs *funcs;

	G_ASSERT(w != NULL);

	io = (HmIO*)w;
	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->error)
	{
		(*funcs->error)(io, rw, why);
	}
}


static void
hm_io_on_close(HmWatch *w, gint async)
{
	HmIO *io;
	HmIOFuncs *funcs;

	G_ASSERT(w != NULL);

	io = (HmIO*)w;
	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->close)
	{
		(*funcs->close)(io, async);
	}
}


static HmWatchFuncs hm_io_watch_funcs =
{
	.recv		= hm_io_recv_data,
	.format		= hm_io_format_data,
	.error		= hm_io_on_error,
	.close		= hm_io_on_close,
	.finalize	= hm_io_finalize
};


static __inline__ gint
hm_io_initialize(HmIO *io, gint listen)
{
	gint buffer_size;
	HmWatch *watch = (HmWatch*)io;

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
__hm_io_finalize(HmIO *io)
{
	if (io->buffer)
	{
		g_free(io->buffer);
		io->buffer = NULL;
	}
};


__export HmIO *
hm_io_new(HmConnection *conn, HmPacketProto *proto, 
	HmIOFuncs *funcs, gsize size)
{
	HmIO *io;
	G_ASSERT(conn != NULL && proto != NULL && funcs != NULL);

	if (hm_connection_is_blocked(conn))
	{
		hm_warning(
			"Net create io on blocked connection '%p'.",
			conn
		);
		return NULL;		
	}

	io = (HmIO*)hm_watch_create(
		conn, &hm_io_watch_funcs, size);
	if (!io)
	{
		hm_warning(
			"Net create watch failed."
		);
		return NULL;
	}

	hm_io_initialize(io, 0);

	io->proto = proto;
	io->funcs = funcs;

	return io;
}


__export HmIO *
hm_listen_io_new(HmConnection *conn, HmPacketProto *proto,
	HmIOFuncs *funcs, gsize size)
{
	HmIO *io;
	G_ASSERT(conn != NULL && proto != NULL && funcs != NULL);

	io = (HmIO*)hm_listen_watch_create(
		conn, &hm_listen_io_watch_funcs, size);
	if (!io)
	{
		hm_warning(
			"Net create watch failed."
		);
		return NULL;
	}

	hm_io_initialize(io, 1);

	io->proto = proto;
	io->funcs = funcs;

	return io;
}


static __inline__ gint
hm_io_packet_proto_check(HmIO *io)
{
	HmPacketProto *proto;
	gint effective, ret;
    HmNetPackInfo payload_raw, *npi;
    HmIOFuncs *funcs;

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

			npi = hm_net_packet_defrag(&payload_raw);
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
					hm_net_packet_release_npi(npi);

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
hm_io_recv_packet(HmIO *io, gchar *buf, gsize size)
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

		if (G_UNLIKELY((ret = hm_io_packet_proto_check(io))))
			return ret;
	}

	return 0;	
}


//:~ End
