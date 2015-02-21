/*
 * nmp_netbuf.c
 *
 * This file implements net I/O ring buffers
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
 *
 * HmNetRealBuf supplies a classic model for production and 
 * consumption issues. Each object contains several recycling
 * I/O bufferes, which are organized as a big ring buffer.
 * 
*/

#include <string.h>
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_netbuf.h"


/*
 * This describes one I/O buffer block.
*/

typedef struct _HmBufBlock HmBufBlock;
struct _HmBufBlock
{
    gint            is_busy;        //@{buf in use}
    gint            start_pos;      //@{index: [start_pos, end_pos)}
    gint            end_pos;        //@{ditto}
    gint            block_size;     //@{buffer block size}
    gchar          *raw_data;   //@{buf memory}
};


/*
 * Generic buff head, for buffering management. 
*/

struct _HmNetBuf
{
    gint            object_size;    //@{sizeof the entity object}
    gint            num_buffers;    //@{buf counts}
    gint            no_blocks;      //@{all buf blocks are in use}
    gint            next_use;       //@{next buf to use}
    gint            next_deal;      //@{next buf to flush}
    gint            buffer_bytes;   //@{bytes in all buffers}
    GMutex         *buf_lock;       //@{protect all above}
    HmNetBufFlush	flush;			//@{flush worker}
};


/*
 * Net real buffer description
*/

typedef struct _HmNetRealBuf HmNetRealBuf;
struct _HmNetRealBuf
{
    HmNetBuf		head;
    HmBufBlock		buff[0];
};


static __inline__ void
hm_net_buf_init(HmNetBuf *buff, gsize size, gint n_blocks,
	gint block_size, HmNetBufFlush flush)
{
	HmBufBlock *block;
	gint index;
	memset(buff, 0, size);

    buff->object_size = size;
    buff->num_buffers = n_blocks;
    buff->no_blocks = FALSE;
    buff->next_use = 0;
    buff->next_deal = buff->num_buffers;
    buff->buffer_bytes = 0;
    buff->buf_lock = g_mutex_new();
    buff->flush = flush;

	if (block_size <= 0 || block_size > MAX_IO_BUFFER_SIZE)
		block_size = MAX_IO_BUFFER_SIZE;

	for (index = 0; index < n_blocks; ++index)
	{
		block = &((HmNetRealBuf*)buff)->buff[index];
		block->raw_data = g_malloc(block_size);
		block->block_size = block_size;
	}
}


static __inline__ void
hm_net_buf_finalize(HmNetBuf *buff)
{
	gint index;
	HmBufBlock *block;

	for (index = buff->num_buffers -1; index >= 0; --index)
	{
		block = &((HmNetRealBuf*)buff)->buff[index];
		g_free(block->raw_data);
	}
}


/*
 * Release a HmNetRealBuf object.
*/
__export void
hm_net_buf_free(HmNetBuf *buf)
{
    G_ASSERT(buf != NULL);

    hm_net_buf_finalize(buf);
	g_mutex_free(buf->buf_lock);
	g_free(buf);
}


/*
 * Alloc a new HmNetRealBuf object.
 *
 * $n_blocks: buf blocks count we want to use.
 * $flush: flusher
*/
HmNetBuf *
hm_net_buf_alloc(gint n_blocks, gint block_size,
	HmNetBufFlush flush)
{
    HmNetBuf *buf;
    gsize size;

	if (n_blocks <= 0 || !flush)
		return NULL;

    size = sizeof(HmNetRealBuf) + (n_blocks * sizeof(HmBufBlock));
	buf = (HmNetBuf*)g_malloc(size);	/* glib has its own OOM facility */
	hm_net_buf_init(buf, size, n_blocks, block_size, flush);

    return buf;
}


/*
 * Get index of last used I/O buffer.
*/
static __inline__ gint
hm_net_buf_last_pos(HmNetBuf *buff)
{
    HmBufBlock *b;
    gint last;
    G_ASSERT(buff != NULL);

    last = buff->next_use;
    if (--last < 0)
        last = buff->num_buffers - 1;

    b = &((HmNetRealBuf*)buff)->buff[last];
    if (b->is_busy)
        return last;

    return -1;
}


static __inline__ void
hm_net_buf_write_ok(HmNetBuf *buff)
{
    if (buff->next_deal == buff->num_buffers)
        buff->next_deal = buff->next_use;

    if (++buff->next_use >= buff->num_buffers)
        buff->next_use = 0;

    if (buff->next_use == buff->next_deal)
        buff->no_blocks = 1;
}


static __inline__ void
hm_net_buf_flush_ok(HmNetBuf *buff)
{
    HmBufBlock *b;

    b = &((HmNetRealBuf*)buff)->buff[buff->next_deal];

    b->is_busy = 0;
    b->start_pos = 0;
    b->end_pos = 0;

    buff->no_blocks = 0;

    if (++buff->next_deal >= buff->num_buffers)
        buff->next_deal = 0;

    b = &((HmNetRealBuf*)buff)->buff[buff->next_deal];
    if (!b->is_busy)
        buff->next_deal = buff->num_buffers;
}


/*
 * Flush a buffer. This is the consumer of the buffer.
 * we try to write all data in this buffer to network, using
 * the given function.
 *
 * @Ret: < 0, error.
 *       = 0, all data has been flushed out.
 *       > 0, bytes left in buffer.
*/
gint
__hm_net_buf_flush(HmNetBuf *buff, gpointer user_data)
{
    HmBufBlock *b;
    gint left, ret;

    while ( TRUE )
    {
        /* buffer empty */
        if (buff->next_deal == buff->num_buffers)
            return 0;

        b = &((HmNetRealBuf*)buff)->buff[buff->next_deal];
        if (G_UNLIKELY(!b->is_busy))
            BUG();

        left = b->end_pos - b->start_pos;
        if (G_UNLIKELY(left <= 0))
            BUG();

        ret = (*buff->flush)(&b->raw_data[b->start_pos], left, user_data);
        if (ret >= 0)
        {
            buff->buffer_bytes -= ret;

            if (ret == left)
                hm_net_buf_flush_ok(buff);
            else
            {
                BUG_ON(ret > left);
                b->start_pos += ret;

                return buff->buffer_bytes;
            }
        }
        else
            break;
    }

    return ret;
}


/*
 * Flush a net buffer.
 * @ret: < 0, error code.
 *       >=0, bytes left in buffer.
*/
gint
hm_net_buf_flush(HmNetBuf *buff, gpointer user_data)
{
    gint ret;
    G_ASSERT(buff != NULL);

    g_mutex_lock(buff->buf_lock);
    ret = __hm_net_buf_flush(buff, user_data);
    g_mutex_unlock(buff->buf_lock);

    return ret;
}


/*
 * try to place data at the end of last used block.
 * @ret: >= 0, bytes appended.
 *       < 0, error code.
*/
static __inline__ gint
hm_net_buf_append(HmNetBuf *buff, gchar *buf, gsize count,
	gpointer user_data, gint *pending)
{
    gint last, left;
    HmBufBlock *b;

	if (count > MAX_IO_BUFFER_SIZE)
		return -E_PACKET2LONG;

    if (G_UNLIKELY(!count))
        return 0;

	*pending = 1;	/* we assume buffer is not empty */

    last = hm_net_buf_last_pos(buff);
    if (last < 0)	/* no data in buffer, try to send */
    {
    	left = (*buff->flush)(buf, count, user_data);
    	if (left == count)
    		*pending = 0;

    	return left;
    }

    b = &((HmNetRealBuf*)buff)->buff[last];
    BUG_ON(!b->is_busy);

    left = b->block_size - b->end_pos;
    if (left >= count)
    {
        memcpy(&b->raw_data[b->end_pos], buf, count);
        b->end_pos += count;
        buff->buffer_bytes += count;

        return count;
    }

    return 0;
}


static __inline__ gint
__hm_net_buf_write(HmNetBuf *buff, gchar *buf, gsize count,
	gpointer user_data, gint *pending)
{
	gsize size;
	gint ret;
    HmBufBlock *b;

    ret = hm_net_buf_append(buff, buf, count, user_data, pending);
    if (ret < 0)
        return ret;

	size = count - ret;
	buf += ret;

	if (!size)
		return count;

    if (buff->no_blocks)
        return 0;

    b = &((HmNetRealBuf*)buff)->buff[buff->next_use];
    if (G_UNLIKELY(b->is_busy))
        BUG();

    b->is_busy = 1;
    b->start_pos = 0;
    b->end_pos = b->start_pos + size;
    memcpy(&b->raw_data[b->start_pos], buf, size);

    buff->buffer_bytes += size;
    hm_net_buf_write_ok(buff);

    return count;
}


/*
 * Fill the buffer, this is the producer of the buffer.
 *
 * @Ret: > 0, bytes written.
 *       = 0, buffer is full, or 'count' is 0.
 *       < 0, error.
*/
gint
hm_net_buf_write(HmNetBuf *buff, gchar *buf, gsize count,
	gpointer user_data, gint *pending)
{
    gint ret;
    G_ASSERT(buff != NULL && buf != NULL && pending != NULL);

    g_mutex_lock(buff->buf_lock);
    ret = __hm_net_buf_write(buff, buf, count, user_data,
    	pending);
    g_mutex_unlock(buff->buf_lock);

    return ret;
}


//:~ End
