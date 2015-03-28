#include "nmp_msg.h"
#include "nmp_debug.h"

static __inline__ void
nmp_free_mem(gpointer private, gsize size)
{
	G_ASSERT(private != NULL);

	g_free(private);
}

NmpMdsMsg *
nmp_alloc_msg(gint msg_id, gpointer data, gsize size, guint seq)
{
	NmpMdsMsg *msg;
	G_ASSERT(!(data && size <= 0));

	msg = g_new0(NmpMdsMsg, 1);
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

NmpMdsMsg *
nmp_alloc_msg_2(gint msg_id, gpointer data, gsize size,
	NmpMsgFin fin, guint seq)
{
	NmpMdsMsg *msg;
	G_ASSERT(!(data && size <= 0) && !(data && !fin));

	msg = g_new0(NmpMdsMsg, 1);
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

void nmp_attach_msg_io(NmpMdsMsg *msg, NmpNetIO *io)
{
	G_ASSERT(msg != NULL && io != NULL);

	BUG_ON(msg->io);
	if (io)
		msg->io = nmp_net_ref_io(io);
}

void nmp_set_msg_data(NmpMdsMsg *msg, gpointer data, gsize size)
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

void nmp_set_msg_data_2(NmpMdsMsg *msg, gpointer data, gsize size,
	NmpMsgFin fin)
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

void nmp_free_msg(NmpMdsMsg *msg)
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
