#ifndef __NMP_RING_BUFFER_H__
#define __NMP_RING_BUFFER_H__

#include <glib.h>

typedef gchar*	b_ptr_t;

typedef struct _JpfRingBufferOps JpfRingBufferOps;
struct _JpfRingBufferOps
{
	gint (*check)(b_ptr_t start, gsize size);	/* sanity check */
	gint (*count)(b_ptr_t start, b_ptr_t end, gsize *lp, gsize *lb);	/* get the count of packets */

	/* drop packets, till the first address which is greater than 'point' found */
	gint (*drop)(b_ptr_t point, b_ptr_t upper, b_ptr_t *adjust, gsize *lp, gsize *lb);
	gsize (*packet)(b_ptr_t start, b_ptr_t end);	/* get a complete packet, return its size */
};


typedef struct _JpfRingBuffer JpfRingBuffer;
struct _JpfRingBuffer
{
	b_ptr_t			buff_start;		/* buffer start */
	b_ptr_t			buff_end;		/* buffer end */
	gsize			buff_left;		/* unused bytes of buffer */

	b_ptr_t			p_reader;		/* current position of reader */
	b_ptr_t			p_writer;		/* current position of writer */

	gsize			lost_bytes;		/* bytes this reader lost */
	gsize			lost_packets;	/* packets this reader lost */
	gsize			total_bytes;	/* total bytes received */
	gsize			total_packets;	/* total packets received */

	JpfRingBufferOps *data_ops;		/* hooks to handle data */

	GStaticMutex	lock;			/* protect read/writer pointer */
};


JpfRingBuffer *nmp_ring_buffer_new(JpfRingBufferOps *data_ops, gsize buff_size);
void nmp_ring_buffer_release(JpfRingBuffer *rb);
gint nmp_ring_buffer_fill_data(JpfRingBuffer *rb, gchar *data, gsize size);
gint nmp_ring_buffer_get_data(JpfRingBuffer *rb, b_ptr_t buf, gsize size, gint *empty);

#endif	//__NMP_RING_BUFFER_H__
