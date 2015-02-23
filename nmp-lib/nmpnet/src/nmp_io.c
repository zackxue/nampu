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
j_io_recv_packet(JIO *io, char *buf, size_t size);


static __inline__ void
j_io_finalize(JIO *io);


static JWatch *
j_io_create(JWatch *w, JConnection *conn)
{
	JIOFuncs *funcs;
	JIO *io = (JIO*)w;

	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->create)
	{
		return (*funcs->create)(io, conn);
	}

	j_connection_close(conn);
	return NULL;
}


static void
j_io_on_listen_error(JWatch *w, int rw, int why)
{
	JIOFuncs *funcs;
	JIO *io = (JIO*)w;

	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->error)
	{
		(*funcs->error)(io, rw, why);
	}
}


static JWatchFuncs j_listen_io_watch_funcs =
{
	.create		= j_io_create,
	.error		= j_io_on_listen_error
};


static int
j_io_recv_data(JWatch *w, char *buf, size_t size)
{
	JIO *io = (JIO*)w;

	if (io->buffer)	/* likely */
	{
		return j_io_recv_packet(io, buf, size);
	}
	return 0;	/* nothing to do */
}


static int
j_io_format_data(JWatch *w, void *msg, char buf[],
	size_t size)
{
	JIOFuncs *funcs;
	JPacketProto *proto;
	int pack_head_len, phl, payload_len = 0;
	JIO *io = (JIO*)w;

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
j_io_on_error(JWatch *w, int rw, int why)
{
	JIO *io;
	JIOFuncs *funcs;

	J_ASSERT(w != NULL);

	io = (JIO*)w;
	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->error)
	{
		(*funcs->error)(io, rw, why);
	}

	j_io_finalize(io);
}


static void
j_io_on_close(JWatch *w, int async)
{
	JIO *io;
	JIOFuncs *funcs;

	J_ASSERT(w != NULL);

	io = (JIO*)w;
	funcs = io->funcs;
	BUG_ON(!funcs);

	if (funcs->close)
	{
		(*funcs->close)(io, async);
	}

	j_io_finalize(io);
}


static JWatchFuncs j_io_watch_funcs =
{
	.recv		= j_io_recv_data,
	.format		= j_io_format_data,
	.error		= j_io_on_error,
	.close		= j_io_on_close
};


static __inline__ int
j_io_initialize(JIO *io, int listen)
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
		io->buffer = j_alloc(MAX_IO_BUFFER_SIZE);
		io->buff_size = MAX_IO_BUFFER_SIZE;
		io->start_pos = 0;
		io->end_pos = 0;
	}

	return 0;
}


static __inline__ void
j_io_finalize(JIO *io)
{
	if (io->buffer)
	{
		j_dealloc(io->buffer, MAX_IO_BUFFER_SIZE);
		io->buffer = NULL;
	}
};


__export JIO *
j_io_new(JConnection *conn, JPacketProto *proto, 
	JIOFuncs *funcs, size_t size)
{
	JIO *io;
	J_ASSERT(conn != NULL && proto != NULL && funcs != NULL);

	if (j_connection_is_blocked(conn))
	{
		j_warning(
			"Net create io on blocked connection '%p'.\n",
			conn
		);
		return NULL;		
	}

	io = (JIO*)j_watch_create(
		conn, &j_io_watch_funcs, size);
	if (!io)
	{
		j_warning(
			"Net create watch failed.\n"
		);
		return NULL;
	}

	j_io_initialize(io, 0);

	io->proto = proto;
	io->funcs = funcs;

	return io;
}



__export JIO *
j_listen_io_new(JConnection *conn, JPacketProto *proto,
	JIOFuncs *funcs, size_t size)
{
	JIO *io;
	J_ASSERT(conn != NULL && proto != NULL && funcs != NULL);

	io = (JIO*)j_listen_watch_create(
		conn, &j_listen_io_watch_funcs, size);
	if (!io)
	{
		j_warning(
			"Net create watch failed.\n"
		);
		return NULL;
	}

	j_io_initialize(io, 1);

	io->proto = proto;
	io->funcs = funcs;

	return io;
}


static __inline__ int
j_io_packet_proto_check(JIO *io)
{
	JPacketProto *proto;
	int effective, ret;
    JNetPackInfo payload_raw, *npi;
    JIOFuncs *funcs;

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

            if (J_UNLIKELY(ret))
                return ret;

			npi = j_net_packet_defrag(&payload_raw);
			if (J_LIKELY(npi))
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

				if (J_UNLIKELY(npi != &payload_raw))
					j_net_packet_release_npi(npi);

				if (J_UNLIKELY(ret))
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
j_io_recv_packet(JIO *io, char *buf, size_t size)
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

		if (J_UNLIKELY((ret = j_io_packet_proto_check(io))))
			return ret;
	}

	return 0;	
}


//:~ End
