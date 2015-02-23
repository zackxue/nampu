/*
 * j_netbuf.c
 *
 * This file implements net I/O ring buffers
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
 *
 * JNetRealBuf supplies a classic model for production and 
 * consumption issues. Each object contains several recycling
 * I/O bufferes, which are organized as a big ring buffer.
 * 
*/

#include <string.h>
#include "nmplib.h"
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_netbuf.h"


/*
 * This describes one I/O buffer block.
*/

typedef struct _JBufBlock JBufBlock;
struct _JBufBlock
{
    int            is_busy;        //@{buf in use}
    int            start_pos;      //@{index: [start_pos, end_pos)}
    int            end_pos;        //@{ditto}
    char           raw_data[MAX_IO_BUFFER_SIZE];   //@{buf memory}
};


/*
 * Generic buff head, for buffering management. 
*/

struct _JNetBuf
{
    int            object_size;    //@{sizeof the entity object}
    int            num_buffers;    //@{buf counts}
    int            no_blocks;      //@{all buf blocks are in use}
    int            next_use;       //@{next buf to use}
    int            next_deal;      //@{next buf to flush}
    int            buffer_bytes;   //@{bytes in all buffers}
    JMutex         *buf_lock;       //@{protect all above}
    JNetBufFlush	flush;			//@{flush worker}
};


/*
 * Net real buffer description
*/

typedef struct _JNetRealBuf JNetRealBuf;
struct _JNetRealBuf
{
    JNetBuf		head;
    JBufBlock		buff[0];
};


static __inline__ void
j_net_buf_init(JNetBuf *buff, size_t size, int n_blocks,
	JNetBufFlush flush)
{
	memset(buff, 0, size);

    buff->object_size = size;
    buff->num_buffers = n_blocks;
    buff->no_blocks = FALSE;
    buff->next_use = 0;
    buff->next_deal = buff->num_buffers;
    buff->buffer_bytes = 0;
    buff->buf_lock = j_mutex_new();
    buff->flush = flush;
}


/*
 * Release a JNetRealBuf object.
*/
__export void
j_net_buf_free(JNetBuf *buf)
{
    J_ASSERT(buf != NULL);

    j_mutex_free(buf->buf_lock);
	j_dealloc(buf, buf->object_size);
}


/*
 * Alloc a new JNetRealBuf object.
 *
 * $n_blocks: buf blocks count we want to use.
 * $flush: flusher
*/
JNetBuf *
j_net_buf_alloc(int n_blocks, JNetBufFlush flush)
{
    JNetBuf *buf;
    size_t size;

	if (n_blocks <= 0 || !flush)
		return NULL;

    size = sizeof(JNetRealBuf) + (n_blocks * sizeof(JBufBlock));
	buf = (JNetBuf*)j_alloc(size);	/* jlib has its own OOM facility */
	j_net_buf_init(buf, size, n_blocks, flush);

    return buf;
}


/*
 * Get index of last used I/O buffer.
*/
static __inline__ int
j_net_buf_last_pos(JNetBuf *buff)
{
    JBufBlock *b;
    int last;
    J_ASSERT(buff != NULL);

    last = buff->next_use;
    if (--last < 0)
        last = buff->num_buffers - 1;

    b = &((JNetRealBuf*)buff)->buff[last];
    if (b->is_busy)
        return last;

    return -1;
}


static __inline__ void
j_net_buf_write_ok(JNetBuf *buff)
{
    if (buff->next_deal == buff->num_buffers)
        buff->next_deal = buff->next_use;

    if (++buff->next_use >= buff->num_buffers)
        buff->next_use = 0;

    if (buff->next_use == buff->next_deal)
        buff->no_blocks = 1;
}


static __inline__ void
j_net_buf_flush_ok(JNetBuf *buff)
{
    JBufBlock *b;

    b = &((JNetRealBuf*)buff)->buff[buff->next_deal];

    b->is_busy = 0;
    b->start_pos = 0;
    b->end_pos = 0;

    buff->no_blocks = 0;

    if (++buff->next_deal >= buff->num_buffers)
        buff->next_deal = 0;

    b = &((JNetRealBuf*)buff)->buff[buff->next_deal];
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
int
__j_net_buf_flush(JNetBuf *buff, void *user_data)
{
    JBufBlock *b;
    int left, ret;

    while ( TRUE )
    {
        /* buffer empty */
        if (buff->next_deal == buff->num_buffers)
            return 0;

        b = &((JNetRealBuf*)buff)->buff[buff->next_deal];
        if (J_UNLIKELY(!b->is_busy))
            BUG();

        left = b->end_pos - b->start_pos;
        if (J_UNLIKELY(left <= 0))
            BUG();

        ret = (*buff->flush)(&b->raw_data[b->start_pos], left, user_data);
        if (ret >= 0)
        {
            buff->buffer_bytes -= ret;

            if (ret == left)
                j_net_buf_flush_ok(buff);
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
int
j_net_buf_flush(JNetBuf *buff, void *user_data)
{
    int ret;
    J_ASSERT(buff != NULL);

    j_mutex_lock(buff->buf_lock);
    ret = __j_net_buf_flush(buff, user_data);
    j_mutex_unlock(buff->buf_lock);

    return ret;
}


/*
 * try to place data at the end of last used block.
 * @ret: >= 0, bytes appended.
 *       < 0, error code.
*/
static __inline__ int
j_net_buf_append(JNetBuf *buff, char *buf, size_t count,
	void *user_data, int *pending)
{
    int last, left;
    JBufBlock *b;

	if (count > MAX_IO_BUFFER_SIZE)
		return -E_PACKET2LONG;

    if (J_UNLIKELY(!count))
        return 0;

	*pending = 1;	/* we assume buffer is not empty */

    last = j_net_buf_last_pos(buff);
    if (last < 0)	/* no data in buffer, try to send */
    {
    	left = (*buff->flush)(buf, count, user_data);
    	if (left == count)
    		*pending = 0;

    	return left;
    }

    b = &((JNetRealBuf*)buff)->buff[last];
    BUG_ON(!b->is_busy);

    left = MAX_IO_BUFFER_SIZE - b->end_pos;
    if (left >= count)
    {
        memcpy(&b->raw_data[b->end_pos], buf, count);
        b->end_pos += count;
        buff->buffer_bytes += count;

        return count;
    }

    return 0;
}


static __inline__ int
__j_net_buf_write(JNetBuf *buff, char *buf, size_t count,
	void *user_data, int *pending)
{
	size_t size;
	int ret;
    JBufBlock *b;

    ret = j_net_buf_append(buff, buf, count, user_data, pending);
    if (ret < 0)
        return ret;

	size = count - ret;
	buf += ret;

	if (!size)
		return count;

    if (buff->no_blocks)
        return 0;

    b = &((JNetRealBuf*)buff)->buff[buff->next_use];
    if (J_UNLIKELY(b->is_busy))
        BUG();

    b->is_busy = 1;
    b->start_pos = 0;
    b->end_pos = b->start_pos + size;
    memcpy(&b->raw_data[b->start_pos], buf, size);

    buff->buffer_bytes += size;
    j_net_buf_write_ok(buff);

    return count;
}


/*
 * Fill the buffer, this is the producer of the buffer.
 *
 * @Ret: > 0, bytes written.
 *       = 0, buffer is full, or 'count' is 0.
 *       < 0, error.
*/
int
j_net_buf_write(JNetBuf *buff, char *buf, size_t count,
	void *user_data, int *pending)
{
    int ret;
    J_ASSERT(buff != NULL && buf != NULL && pending != NULL);

    j_mutex_lock(buff->buf_lock);
    ret = __j_net_buf_write(buff, buf, count, user_data,
    	pending);
    j_mutex_unlock(buff->buf_lock);

    return ret;
}


//:~ End
