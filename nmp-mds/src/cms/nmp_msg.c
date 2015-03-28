#include "nmp_msg.h"
#include "nmp_debug.h"

static __inline__ void
nmp_free_mem(gpointer private, gsize size)
{
	G_ASSERT(private != NULL);

	g_free(private);
}

JpfMdsMsg *
nmp_alloc_msg(gint msg_id, gpointer data, gsize size, guint seq)
{
	JpfMdsMsg *msg;
	G_ASSERT(!(data && size <= 0));

	msg = g_new0(JpfMdsMsg, 1);
	msg->msg_Id = msg_id;
	msg->seq = seq;
	if (data)
	{
		msg->private = g_malloc(size);
		msg->priv_size = size;
		memcpy(msg->private, data, size);
		msg->fin = nmp_free_mem;
	}

	return msg;
}

JpfMdsMsg *
nmp_alloc_msg_2(gint msg_id, gpointer data, gsize size,
	JpfMsgFin fin, guint seq)
{
	JpfMdsMsg *msg;
	G_ASSERT(!(data && size <= 0) && !(data && !fin));

	msg = g_new0(JpfMdsMsg, 1);
	msg->msg_Id = msg_id;
	msg->seq = seq;
	if (data)
	{
		msg->private = data;
		msg->priv_size = size;
		msg->fin = fin;
	}

	return msg;
}

void nmp_attach_msg_io(JpfMdsMsg *msg, NmpNetIO *io)
{
	G_ASSERT(msg != NULL && io != NULL);

	BUG_ON(msg->io);
	if (io)
		msg->io = nmp_net_ref_io(io);
}

void nmp_set_msg_data(JpfMdsMsg *msg, gpointer data, gsize size)
{
	G_ASSERT(msg && !(data && size <= 0));

	if (msg->private)
	{
		BUG_ON(!msg->fin);
		(*msg->fin)(msg->private, msg->priv_size);
		msg->private = NULL;
		msg->priv_size = 0;
		msg->fin = NULL;
	}

	if (data)
	{
		msg->private = g_malloc(size);
		msg->priv_size = size;
		memcpy(msg->private, data, size);
		msg->fin = nmp_free_mem;
	}
}

void nmp_set_msg_data_2(JpfMdsMsg *msg, gpointer data, gsize size,
	JpfMsgFin fin)
{
	G_ASSERT(!(data && size <= 0) && !(data && !fin));

	if (msg->private)
	{
		BUG_ON(!msg->fin);
		(*msg->fin)(msg->private, msg->priv_size);
		msg->private = NULL;
		msg->priv_size = 0;
		msg->fin = NULL;
	}

	if (data)
	{
		msg->private = data;
		msg->priv_size = size;
		msg->fin = fin;
	}
}

void nmp_free_msg(JpfMdsMsg *msg)
{
	G_ASSERT(msg != NULL);

	if (msg->io)
		nmp_net_unref_io(msg->io);

	if (msg->private)
	{
		BUG_ON(!msg->fin);
		(*msg->fin)(msg->private, msg->priv_size);
	}

	g_free(msg);
}

//:~ End
