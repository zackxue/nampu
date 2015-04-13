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


static __inline__ int
nmp_io_recv_packet(nmpio_t *io, char *buf, size_t size);


static __inline__ void
nmp_io_finalize(nmpio_t *io);


static nmp_watch_t *
nmp_io_create(nmp_watch_t *w, nmp_conn_t *conn)
{
	nmpio_funcs *funcs;
	nmpio_t *io = (nmpio_t*)w;

	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->create)
	{
		return (*funcs->create)(io, conn);
	}

	nmp_conn_close(conn);
	return NULL;
}


static void
nmp_io_on_listen_error(nmp_watch_t *w, int rw, int why)
{
	nmpio_funcs *funcs;
	nmpio_t *io = (nmpio_t*)w;

	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->error)
	{
		(*funcs->error)(io, rw, why);
	}
}


static nmp_watch_funcs j_listen_io_watch_funcs =
{
	.create		= nmp_io_create,
	.error		= nmp_io_on_listen_error
};


static int
nmp_io_recv_data(nmp_watch_t *w, char *buf, size_t size)
{
	nmpio_t *io = (nmpio_t*)w;

	if (io->buffer)	/* likely */
	{
		return nmp_io_recv_packet(io, buf, size);
	}
	return 0;	/* nothing to do */
}


static int
nmp_io_format_data(nmp_watch_t *w, void *msg, char buf[],
	size_t size)
{
	nmpio_funcs *funcs;
	nmp_packet_proto_t *proto;
	int pack_head_len, phl, payload_len = 0;
	nmpio_t *io = (nmpio_t*)w;

	proto = io->proto;
	BUG_ON(!proto);

	funcs = io->funcs;
	BUG_ON(!funcs);

	if (!proto->pack)
		return 0;

	pack_head_len = (*proto->pack)(msg, -1, buf, size);//获取协议头长度
	if (pack_head_len < 0)
		return pack_head_len;

	BUG_ON(pack_head_len >= size);

	if (funcs->format)
	{
		payload_len = (*funcs->format)(
			io, msg, buf + pack_head_len, size - pack_head_len);
		if (payload_len < 0)
			return payload_len;

		phl = (*proto->pack)(msg, payload_len, buf, pack_head_len);//协议头长度+ 有效负载长度
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
nmp_io_on_error(nmp_watch_t *w, int rw, int why)
{
	nmpio_t *io;
	nmpio_funcs *funcs;

	NMP_ASSERT(w != NULL);

	io = (nmpio_t*)w;
	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->error)
	{
		(*funcs->error)(io, rw, why);
	}

	nmp_io_finalize(io);
}


static void
nmp_io_on_close(nmp_watch_t *w, int async)
{
	nmpio_t *io;
	nmpio_funcs *funcs;

	NMP_ASSERT(w != NULL);

	io = (nmpio_t*)w;
	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->close)
	{
		(*funcs->close)(io, async);
	}

	nmp_io_finalize(io);
}


static nmp_watch_funcs nmp_io_watch_funcs =
{
	.recv		= nmp_io_recv_data,
	.format		= nmp_io_format_data,
	.error		= nmp_io_on_error,
	.close		= nmp_io_on_close
};


static __inline__ int
nmp_io_initialize(nmpio_t *io, int listen)
{
	if (listen)
	{
		io->buffer = NULL;
		io->buff_size = 0;
		io->start_pos = 0;
		io->end_pos = 0;
	}
	else
	{
		io->buffer = nmp_alloc(MAX_IO_BUFFER_SIZE);
		io->buff_size = MAX_IO_BUFFER_SIZE;
		io->start_pos = 0;
		io->end_pos = 0;
	}

	return 0;
}


static __inline__ void
nmp_io_finalize(nmpio_t *io)
{
	if (io->buffer)
	{
		nmp_dealloc(io->buffer, MAX_IO_BUFFER_SIZE);
		io->buffer = NULL;
	}
};


__export nmpio_t *
nmp_io_new(nmp_conn_t *conn, nmp_packet_proto_t *proto, 
	nmpio_funcs *funcs, size_t size)
{
	nmpio_t *io;
	NMP_ASSERT(conn != NULL && proto != NULL && funcs != NULL);

	if (nmp_conn_is_blocked(conn))
	{
		nmp_warning(
			"Net create io on blocked connection '%p'.\n",
			conn
		);
		return NULL;		
	}

	io = (nmpio_t*)nmp_watch_create(
		conn, &nmp_io_watch_funcs, size);
	if (!io)
	{
		nmp_warning(
			"Net create watch failed.\n"
		);
		return NULL;
	}

	nmp_io_initialize(io, 0);

	io->proto = proto;
	io->funcs = funcs;

	return io;
}



__export nmpio_t *
j_listen_io_new(nmp_conn_t *conn, nmp_packet_proto_t *proto,
	nmpio_funcs *funcs, size_t size)
{
	nmpio_t *io;
	NMP_ASSERT(conn != NULL && proto != NULL && funcs != NULL);

	io = (nmpio_t*)j_listen_watch_create(
		conn, &j_listen_io_watch_funcs, size);
	if (!io)
	{
		nmp_warning(
			"Net create watch failed.\n"
		);
		return NULL;
	}

	nmp_io_initialize(io, 1);

	io->proto = proto;
	io->funcs = funcs;

	return io;
}


static __inline__ int
nmp_io_packet_proto_check(nmpio_t *io)
{
	nmp_packet_proto_t *proto;
	int effective, ret;
    nmp_net_packinfo_t payload_raw, *npi;
    nmpio_funcs *funcs;

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

            if (NMP_UNLIKELY(ret))
                return ret;

			npi = nmp_net_packet_defrag(&payload_raw);
			if (NMP_LIKELY(npi))
			{
				if (funcs->recv)
				{
					ret = (*funcs->recv)( //hlio->recv
						io,
						npi->start,
						npi->size,
						npi->private_from_low_layer
					);
				}

				if (NMP_UNLIKELY(npi != &payload_raw))
					nmp_net_packet_release_npi(npi);

				if (NMP_UNLIKELY(ret))
					return ret;

				if (!io->buffer)		/* killed when we just dropped the watch lock in (->recv())*/
					return -E_PACKET;

			}

            io->start_pos += effective;

			if (io->start_pos >= io->end_pos)
			{
				io->start_pos = io->end_pos = 0;
				return 0;
			}

			if (io->start_pos % sizeof(void*))
			{
				memcpy(&io->buffer[0],			/* move, in order to keep starting address alignment. */
	            	&io->buffer[io->start_pos],
	            	io->end_pos - io->start_pos);			

				io->end_pos -= io->start_pos;
				io->start_pos = 0;
			}
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


static __inline__ int
nmp_io_recv_packet(nmpio_t *io, char *buf, size_t size)
{
	int ret, left;

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

		if (NMP_UNLIKELY((ret = nmp_io_packet_proto_check(io))))
			return ret;
	}

	return 0;	
}


//:~ End
