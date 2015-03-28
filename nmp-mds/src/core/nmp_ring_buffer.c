#include <string.h>
#include "nmp_ring_buffer.h"
#include "nmp_errno.h"
#include "nmp_debug.h"

#define ALIGNMENT				4
#define BUFF_ALIGN(size)		(((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#define nmp_mem_kmalloc(size)	g_malloc(size)
#define nmp_mem_kzalloc(size)	g_malloc0(size)
#define nmp_mem_kfree(ptr, size)	g_free(ptr)

NmpRingBuffer *
nmp_ring_buffer_new(NmpRingBufferOps *data_ops, gsize buff_size)
{
	NmpRingBuffer *rb;
	G_ASSERT(data_ops != NULL);

	rb = nmp_mem_kmalloc(sizeof(NmpRingBuffer));
	if (G_UNLIKELY(!rb))
		return NULL;
	
	rb->buff_start = nmp_mem_kmalloc(buff_size);
	if (G_UNLIKELY(!rb->buff_start))
	{
		nmp_mem_kfree(rb, sizeof(NmpRingBuffer));
		return NULL;
	}

	rb->buff_end = rb->buff_start + buff_size;
	rb->buff_left = 0;
	rb->p_reader = rb->buff_start;
	rb->p_writer = rb->buff_start;
	rb->lost_bytes = 0;
	rb->lost_packets = 0;
	rb->data_ops = data_ops;
	g_static_mutex_init(&rb->lock);

	return rb;
}


void
nmp_ring_buffer_release(NmpRingBuffer *rb)
{
	g_assert(rb != NULL);

	g_static_mutex_free(&rb->lock);
	nmp_mem_kfree(rb->buff_start, rb->buff_end - rb->buff_start);
	nmp_mem_kfree(rb, sizeof(*rb));
}


static __inline__ void
nmp_ring_buffer_reset(NmpRingBuffer *rb)
{
	rb->p_reader = rb->buff_start;
	rb->p_writer = rb->buff_start;
}


static __inline__ void
nmp_ring_buffer_adjust_wr(NmpRingBuffer *rb, gsize size)
{
	b_ptr_t w;
	gint err;
	gsize lost_bytes, lost_packets;

	w = rb->p_writer + size;
	if (G_LIKELY(w < rb->p_reader))
		return;		/* needn't to adjust reader pointer */

	if (w >= rb->buff_end - rb->buff_left)
	{
		err = (*rb->data_ops->count)(
			rb->p_reader,
			rb->buff_end - rb->buff_left,
			&lost_packets,
			&lost_bytes
		);

		BUG_ON(err);

		rb->lost_packets += lost_packets;
		rb->lost_bytes += lost_bytes;

		if (w > rb->buff_end)
		{
			/* space at the end of buffer is not enough, the writer
			 * need to go back to the beginning */
			w = rb->buff_start + size;

			if (G_UNLIKELY(w >= rb->p_writer))
			{
				err = (*rb->data_ops->count)(
					rb->buff_start,
					rb->p_writer,
					&lost_packets,
					&lost_bytes
				);

				BUG_ON(err);

				rb->p_reader = rb->buff_start;
			}
			else
			{
				rb->p_reader = rb->buff_start;

				err = (*rb->data_ops->drop)(
					w,			/* greater than w and */
					rb->p_writer,	/* smaller and equal than this */
					&rb->p_reader,
					&lost_packets,
					&lost_bytes
				);

				BUG_ON(err);

				if (G_UNLIKELY(rb->p_reader >= rb->p_writer))
				{
					/* the last packet is partially overlapped */
					rb->p_reader = rb->buff_start;
				}
			}

			rb->buff_left = rb->buff_end - rb->p_writer;
			rb->p_writer = rb->buff_start;
		}
		else
		{
			/* packets at the end of the buffer will be dropped,
			 * writer need not to go back. */
			rb->p_reader = rb->buff_start;
		}
	}
	else
	{
		/* packets at the end of the buffer are partially overlapped,
		 * we drop a lot of them, and place the reader pointer at the
		 * correct position. */
		err = (*rb->data_ops->drop)(
			w,				/* greater than w and */
			rb->buff_end - rb->buff_left,	/* smaller and equal than this */
			&rb->p_reader,
			&lost_packets,
			&lost_bytes
		);

		BUG_ON(err);
		
		if (rb->p_reader >= rb->buff_end - rb->buff_left)
		{
			/* the last packet is partially overlapped, drop */
			rb->p_reader = rb->buff_start;
		}
	}

	rb->lost_packets += lost_packets;
	rb->lost_bytes += lost_bytes;
}


static __inline__ void
nmp_ring_buffer_adjust_rw(NmpRingBuffer *rb, gsize size)
{
	gsize lost_packets, lost_bytes;
	gint err;
	b_ptr_t w;

	w = rb->p_writer + size;
	if (G_LIKELY(w <= rb->buff_end))
		return;

	w = rb->buff_start + size;

	if (G_LIKELY(w < rb->p_reader))
	{
		rb->buff_left = rb->buff_end - rb->p_writer;
		rb->p_writer = rb->buff_start;
		return;
	}

	if (G_UNLIKELY(w >= rb->p_writer))
	{
		/* all valid packets will be overlapped, we get the
		 * packets count, and adjust the reader pointer */
		err = (*rb->data_ops->count)(
			rb->p_reader,
			rb->p_writer,
			&lost_packets,
			&lost_bytes
		);

		BUG_ON(err);

		rb->p_reader = rb->buff_start;
	}
	else
	{
		/* packets are partially overlapped, drop. */
		err = (*rb->data_ops->drop)(
			w,				/* greater than w and */
			rb->p_writer,	/* smaller and equal than this */
			&rb->p_reader,
			&lost_packets,
			&lost_bytes
		);

		BUG_ON(err);

		if (G_UNLIKELY(rb->p_reader >= rb->p_writer))
		{
			/* the last packet is overlapped, that means all
			 * packets in the buffer will be dropped. */
			rb->p_reader = rb->buff_start;
		}
	}

	rb->buff_left = rb->buff_end - rb->p_writer;
	rb->p_writer = rb->buff_start;

	rb->lost_packets += lost_packets;
	rb->lost_bytes += lost_bytes;
}


static __inline__ void
nmp_ring_buffer_write(NmpRingBuffer *rb, b_ptr_t buf, gsize size)
{
	memcpy(rb->p_writer, buf, size);
	rb->p_writer += size;
	rb->total_bytes += size;
	++rb->lost_packets;
}


static __inline__ void
nmp_ring_buffer_adjust_pointer(NmpRingBuffer *rb, gsize size)
{
	gint relative_pos;

	relative_pos = (gint)(rb->p_writer - rb->p_reader);
	if (relative_pos == 0)
	{
		/* 'p_writer == p_reader' means buffer is 
		 * empty, we place them at the beginning of
		 * it. |..........rw..........|*/
		nmp_ring_buffer_reset(rb);
	}
	else if (relative_pos < 0)
	{
		/* buffer looks like this:
		 * |========w......r=======|.|
		 */
		nmp_ring_buffer_adjust_wr(rb, size);
	}
	else
	{
		/* buffer looks like this:
		 * |........r=======w........|
		 */
		nmp_ring_buffer_adjust_rw(rb, size);
	}
}


static __inline__ gint
__nmp_ring_buffer_fill_data(NmpRingBuffer *rb, gchar *data, gsize size)
{
	gsize buff_size;
	
	buff_size = (gsize)(rb->buff_end - rb->buff_start);
	if (G_UNLIKELY(size >= buff_size))
		return -E_BUFFSIZE;	/* == is not permitted */
	
	nmp_ring_buffer_adjust_pointer(rb, size);
	nmp_ring_buffer_write(rb, data, size);

	return 0;	/* always successful */
}


static __inline__ gint
__nmp_ring_buffer_get_data(NmpRingBuffer *rb, b_ptr_t buf, gsize size,
	gint *empty)
{
	gsize pack_size;
	b_ptr_t end;

	if (rb->p_reader == rb->p_writer)
	{
		if (empty)
			*empty = 1;
		/* ring buffer is empty, get nothing */
		return 0;
	}

	end = rb->p_reader < rb->p_writer ? rb->p_writer :
		rb->buff_end - rb->buff_left;

	pack_size = (*rb->data_ops->packet)(rb->p_reader, end);
	if (pack_size > 0)
	{
		if (size < pack_size)
			return -E_BUFFSIZE;

		memcpy(buf, rb->p_reader, pack_size);
		rb->p_reader += pack_size;

		if (rb->p_reader > rb->p_writer &&
			rb->p_reader >= rb->buff_end - rb->buff_left)
		{
			rb->p_reader = rb->buff_start;
		}
	}

	if (empty)
	{
		*empty = (rb->p_reader == rb->p_writer);
	}

	return pack_size;
}


__export gint
nmp_ring_buffer_fill_data(NmpRingBuffer *rb, gchar *data, gsize size)
{
	gint err;
	G_ASSERT(rb != NULL && data != NULL);

	err = (*rb->data_ops->check)(data, size);
	if (G_UNLIKELY(err))
		return err;

	g_static_mutex_lock(&rb->lock);
	err = __nmp_ring_buffer_fill_data(rb, data, size);
	g_static_mutex_unlock(&rb->lock);

	return err;
}


__export gint
nmp_ring_buffer_get_data(NmpRingBuffer *rb, b_ptr_t buf, gsize size,
	gint *empty)
{
	gint ret;
	G_ASSERT(rb != NULL && buf != NULL);

	g_static_mutex_lock(&rb->lock);
	ret = __nmp_ring_buffer_get_data(rb, buf, size, empty);
	g_static_mutex_unlock(&rb->lock);

	return ret;
}


//:~ End
