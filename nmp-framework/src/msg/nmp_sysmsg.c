/*
 * nmp_sysmsg.c
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include "nmp_share_errno.h"
#include "nmp_sysmsg.h"
#include "nmp_memory.h"
#include "nmp_share_debug.h"

G_DEFINE_TYPE(NmpSysMsg, nmp_sysmsg, NMP_TYPE_DATA);
static gint total_sysmsg_count = 0;

static void
nmp_sysmsg_init(NmpSysMsg *self)
{
    G_ASSERT(self != NULL);

    INIT_MSGID(self->msg_id);
    self->flags = FLG_SYSMSG_FORWARD;
    self->packet_layer.seq = 0;
    self->from_io = NULL;
    self->dst = BUSSLOT_POS_MAX;
    self->src = BUSSLOT_POS_MAX;
    self->orig = BUSSLOT_POS_MAX;
    self->priv = nmp_new0(NmpSysMsgPriv, 1);
    BUG_ON(!self->priv);    /* glib says never */
	self->user_data = NULL;
	self->user_size = 0;

    g_atomic_int_add(&total_sysmsg_count, 1);


	{	//@{debug}
		static guint print_tag = 0;

		if (++print_tag % 1000 == 0)
		{ 
		    nmp_print(
		        "===== NOW TOTAL SYSMSG OJBECTS: %d =====",
		        g_atomic_int_get(&total_sysmsg_count)
		    );
		}
	}
}


static void
nmp_sysmsg_dispose(GObject *object)
{
//    NmpSysMsg *self = (NmpSysMsg*)object;
    g_atomic_int_add(&total_sysmsg_count, -1);
    G_OBJECT_CLASS(nmp_sysmsg_parent_class)->dispose(object);
}


static void
nmp_sysmsg_class_init(NmpSysMsgClass *c_self)
{
    GObjectClass *gobject_class = (GObjectClass*)c_self;

    gobject_class->dispose = nmp_sysmsg_dispose;
}


static void
nmp_sysmsg_default_priv_des(gpointer priv, gsize size)
{
    G_ASSERT(priv != NULL);

    nmp_mem_kfree(priv, size);
}


void 
nmp_sysmsg_attach_io(NmpSysMsg *msg, NmpNetIO *io)
{
    G_ASSERT(NMP_IS_SYSMSG(msg) && io != NULL);

    BUG_ON(msg->from_io);
    msg->from_io = io;
    nmp_net_ref_io(io);
}


void
nmp_sysmsg_detach_io(NmpSysMsg *msg)
{
    G_ASSERT(NMP_IS_SYSMSG(msg));

    if (msg->from_io)
    {
        nmp_net_unref_io(msg->from_io);
        msg->from_io = NULL;
    }
}


static __inline__ void
nmp_sysmsg_copy_user_data(NmpSysMsg *msg_cpy, NmpSysMsg *msg)
{
	if (msg->user_data)
	{
		msg_cpy->user_data = nmp_mem_kalloc(msg->user_size);
		memcpy(msg_cpy->user_data, msg->user_data, msg->user_size);
		msg_cpy->user_size = msg->user_size;
	}
}


static __inline__ void
nmp_sysmsg_free_user_data(NmpSysMsg *msg)
{
	if (msg->user_data)
	{
		nmp_mem_kfree(msg->user_data, msg->user_size);
	}
}


static __inline__ void
nmp_sysmsg_priv_init(NmpSysMsgPriv *priv, gpointer priv_data,
    gsize priv_size, NmpMsgPrivDes priv_destroy)
{
    priv->ref_count = 1;
    priv->priv_data = priv_data;
    priv->priv_size = priv_size;
    priv->priv_destroy = priv_destroy;
}


static __inline__ void
nmp_sysmsg_private_set(NmpSysMsgPriv *priv, gpointer priv_data,
    gsize priv_size, NmpMsgPrivDes priv_destroy)
{
    BUG_ON(g_atomic_int_get(&priv->ref_count) <= 0);

    if (priv->priv_destroy)
        (*priv->priv_destroy)(priv->priv_data, priv->priv_size);

    priv->priv_data = priv_data;
    priv->priv_size = priv_size;
    priv->priv_destroy = priv_destroy;
}


static __inline__ NmpSysMsgPriv *
nmp_sysmsg_ref_priv(NmpSysMsgPriv *priv)
{
    BUG_ON(!priv);
    BUG_ON(g_atomic_int_get(&priv->ref_count) <= 0);
    g_atomic_int_inc(&priv->ref_count);

    return priv;
}


static __inline__ void
nmp_sysmsg_unref_priv(NmpSysMsgPriv *priv)
{
    BUG_ON(!priv);
    BUG_ON(g_atomic_int_get(&priv->ref_count) <= 0);

    if (g_atomic_int_dec_and_test(&priv->ref_count))
    {
        if (priv->priv_destroy)
            (*priv->priv_destroy)(priv->priv_data, priv->priv_size);
	 	nmp_free(priv);
    }   
}


static __inline__ void
nmp_sysmsg_copy(NmpSysMsg *msg_cpy, NmpSysMsg *msg)
{
    G_ASSERT(msg_cpy != NULL && msg != NULL);

    msg_cpy->flags = msg->flags;
    msg_cpy->dst = msg->dst;
    msg_cpy->src = msg->src;
    msg_cpy->orig = msg->orig;
    if (msg->from_io)
    {
        nmp_sysmsg_attach_io(msg_cpy, msg->from_io);
    }

    nmp_sysmsg_unref_priv(msg_cpy->priv);
    msg_cpy->priv = nmp_sysmsg_ref_priv(msg->priv);
	nmp_sysmsg_copy_user_data(msg_cpy, msg);
}


void
nmp_sysmsg_destroy(NmpSysMsg *msg)
{
    G_ASSERT(NMP_IS_SYSMSG(msg));

    nmp_sysmsg_unref_priv(msg->priv);
    nmp_sysmsg_detach_io(msg);
    nmp_sysmsg_free_user_data(msg);
    g_object_unref(msg);
}


NmpSysMsg*
__nmp_sysmsg_new(NmpMsgID msg_id, gpointer priv, gsize size, guint seq,
    NmpMsgPrivDes priv_destroy)
{
    NmpSysMsg *msg;

    msg = g_object_new(NMP_TYPE_SYSMSG, NULL);
    if (G_UNLIKELY(!msg))
        return msg;

    MSG_SETID(msg, msg_id);
    msg->packet_layer.seq = seq;
    nmp_sysmsg_priv_init(msg->priv, priv, size, priv_destroy);

    return msg;
}


NmpSysMsg*
nmp_sysmsg_new(NmpMsgID msg_id, gpointer priv, gsize size,
    guint seq, NmpMsgPrivDes priv_destroy)
{
    if (G_UNLIKELY(priv && !priv_destroy))
        return NULL;

    return __nmp_sysmsg_new(msg_id, priv, size, seq, priv_destroy);
}


NmpSysMsg *
nmp_sysmsg_new_2(NmpMsgID msg_id, gpointer priv, gsize size, guint seq)
{
    NmpSysMsg *msg;
    NmpMsgPrivDes des = NULL;
    gpointer priv_copy = 0;

    if (priv && size)
    {
        priv_copy = nmp_mem_kalloc(size);
        if (G_UNLIKELY(!priv_copy))
            return NULL;

        memcpy(priv_copy, priv, size);
        des = nmp_sysmsg_default_priv_des;
    }

    msg = __nmp_sysmsg_new(msg_id, priv_copy, size, seq, des);
    if (G_UNLIKELY(!msg))
    {
        if (des)
        {
            (*des)(priv_copy, size);
        }
        return msg;
    }

    return msg;
}


static __inline__ void
nmp_sysmsg_detach_shared_private(NmpSysMsg *msg)
{
	NmpSysMsgPriv *priv = msg->priv;
	BUG_ON(!priv);

	if(g_atomic_int_get(&priv->ref_count) > 1)
	{//@{someone else owns it}
		nmp_sysmsg_unref_priv(priv);
		msg->priv = nmp_new0(NmpSysMsgPriv, 1);
		BUG_ON(!msg->priv);	/* glib says never */
		nmp_sysmsg_priv_init(msg->priv, NULL, 0, NULL);
	}
}


void
nmp_sysmsg_set_private(NmpSysMsg *msg, gpointer priv, gsize size,
    NmpMsgPrivDes priv_destroy)
{
    G_ASSERT(NMP_IS_SYSMSG(msg) && !(priv && !priv_destroy));

	nmp_sysmsg_detach_shared_private(msg);
    nmp_sysmsg_private_set(msg->priv, priv, size, priv_destroy);
}


gint
nmp_sysmsg_set_private_2(NmpSysMsg *msg, gpointer priv, gsize size)
{
    NmpMsgPrivDes des = NULL;
    gpointer priv_copy = 0;
    G_ASSERT(NMP_IS_SYSMSG(msg));

    if (priv && size)
    {
        priv_copy = nmp_mem_kalloc(size);
        if (G_UNLIKELY(!priv_copy))
            return -E_NOMEM;

        memcpy(priv_copy, priv, size);
        des = nmp_sysmsg_default_priv_des;      
    }

    nmp_sysmsg_set_private(msg, priv_copy, size, des);
    return 0;
}


NmpSysMsg *
nmp_sysmsg_copy_one(NmpSysMsg *msg)
{
    NmpSysMsg *msg_cpy = NULL;

    msg_cpy = nmp_sysmsg_new_2(MSG_GETID(msg), NULL, 0, MSG_SEQ(msg));
    if (msg_cpy)
    {
        nmp_sysmsg_copy(msg_cpy, msg);
    }
    return msg_cpy;
}


gint
nmp_sysmsg_set_userdata(NmpSysMsg *msg, gpointer data, gsize size)
{
	gpointer p;
	G_ASSERT(NMP_IS_SYSMSG(msg));

	if (data && size)
	{
		p = nmp_mem_kalloc(size);
		if (!p)
		{
			return -ENOMEM;
		}

		if (msg->user_data)
		{
			nmp_mem_kfree(msg->user_data, msg->user_size);
		}

		msg->user_data = p;
		msg->user_size = size;
		memcpy(msg->user_data, data, size);
		return 0;
	}

	return -EINVAL;
}


gint
nmp_sysmsg_get_userdata(NmpSysMsg *msg, gpointer *pdata, gsize *psize)
{
	G_ASSERT(NMP_IS_SYSMSG(msg));

	if (pdata && psize)
	{
		*pdata = msg->user_data;
		*psize = msg->user_size;
		return 0;
	}

	return -EINVAL;
}


//:~ End
