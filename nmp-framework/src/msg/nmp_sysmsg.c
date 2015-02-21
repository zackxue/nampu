/*
 * jpf_sysmsg.c
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
*/

#include "nmp_errno.h"
#include "nmp_sysmsg.h"
#include "nmp_memory.h"
#include "nmp_debug.h"

G_DEFINE_TYPE(JpfSysMsg, jpf_sysmsg, JPF_TYPE_DATA);
static gint total_sysmsg_count = 0;

static void
jpf_sysmsg_init(JpfSysMsg *self)
{
    G_ASSERT(self != NULL);

    INIT_MSGID(self->msg_id);
    self->flags = FLG_SYSMSG_FORWARD;
    self->packet_layer.seq = 0;
    self->from_io = NULL;
    self->dst = BUSSLOT_POS_MAX;
    self->src = BUSSLOT_POS_MAX;
    self->orig = BUSSLOT_POS_MAX;
    self->priv = jpf_new0(JpfSysMsgPriv, 1);
    BUG_ON(!self->priv);    /* glib says never */
	self->user_data = NULL;
	self->user_size = 0;

    g_atomic_int_add(&total_sysmsg_count, 1);


	{	//@{debug}
		static guint print_tag = 0;

		if (++print_tag % 1000 == 0)
		{ 
		    jpf_print(
		        "===== NOW TOTAL SYSMSG OJBECTS: %d =====",
		        g_atomic_int_get(&total_sysmsg_count)
		    );
		}
	}
}


static void
jpf_sysmsg_dispose(GObject *object)
{
//    JpfSysMsg *self = (JpfSysMsg*)object;
    g_atomic_int_add(&total_sysmsg_count, -1);
    G_OBJECT_CLASS(jpf_sysmsg_parent_class)->dispose(object);
}


static void
jpf_sysmsg_class_init(JpfSysMsgClass *c_self)
{
    GObjectClass *gobject_class = (GObjectClass*)c_self;

    gobject_class->dispose = jpf_sysmsg_dispose;
}


static void
jpf_sysmsg_default_priv_des(gpointer priv, gsize size)
{
    G_ASSERT(priv != NULL);

    jpf_mem_kfree(priv, size);
}


void 
jpf_sysmsg_attach_io(JpfSysMsg *msg, JpfNetIO *io)
{
    G_ASSERT(JPF_IS_SYSMSG(msg) && io != NULL);

    BUG_ON(msg->from_io);
    msg->from_io = io;
    jpf_net_ref_io(io);
}


void
jpf_sysmsg_detach_io(JpfSysMsg *msg)
{
    G_ASSERT(JPF_IS_SYSMSG(msg));

    if (msg->from_io)
    {
        jpf_net_unref_io(msg->from_io);
        msg->from_io = NULL;
    }
}


static __inline__ void
jpf_sysmsg_copy_user_data(JpfSysMsg *msg_cpy, JpfSysMsg *msg)
{
	if (msg->user_data)
	{
		msg_cpy->user_data = jpf_mem_kalloc(msg->user_size);
		memcpy(msg_cpy->user_data, msg->user_data, msg->user_size);
		msg_cpy->user_size = msg->user_size;
	}
}


static __inline__ void
jpf_sysmsg_free_user_data(JpfSysMsg *msg)
{
	if (msg->user_data)
	{
		jpf_mem_kfree(msg->user_data, msg->user_size);
	}
}


static __inline__ void
jpf_sysmsg_priv_init(JpfSysMsgPriv *priv, gpointer priv_data,
    gsize priv_size, JpfMsgPrivDes priv_destroy)
{
    priv->ref_count = 1;
    priv->priv_data = priv_data;
    priv->priv_size = priv_size;
    priv->priv_destroy = priv_destroy;
}


static __inline__ void
jpf_sysmsg_private_set(JpfSysMsgPriv *priv, gpointer priv_data,
    gsize priv_size, JpfMsgPrivDes priv_destroy)
{
    BUG_ON(g_atomic_int_get(&priv->ref_count) <= 0);

    if (priv->priv_destroy)
        (*priv->priv_destroy)(priv->priv_data, priv->priv_size);

    priv->priv_data = priv_data;
    priv->priv_size = priv_size;
    priv->priv_destroy = priv_destroy;
}


static __inline__ JpfSysMsgPriv *
jpf_sysmsg_ref_priv(JpfSysMsgPriv *priv)
{
    BUG_ON(!priv);
    BUG_ON(g_atomic_int_get(&priv->ref_count) <= 0);
    g_atomic_int_inc(&priv->ref_count);

    return priv;
}


static __inline__ void
jpf_sysmsg_unref_priv(JpfSysMsgPriv *priv)
{
    BUG_ON(!priv);
    BUG_ON(g_atomic_int_get(&priv->ref_count) <= 0);

    if (g_atomic_int_dec_and_test(&priv->ref_count))
    {
        if (priv->priv_destroy)
            (*priv->priv_destroy)(priv->priv_data, priv->priv_size);
	 	jpf_free(priv);
    }   
}


static __inline__ void
jpf_sysmsg_copy(JpfSysMsg *msg_cpy, JpfSysMsg *msg)
{
    G_ASSERT(msg_cpy != NULL && msg != NULL);

    msg_cpy->flags = msg->flags;
    msg_cpy->dst = msg->dst;
    msg_cpy->src = msg->src;
    msg_cpy->orig = msg->orig;
    if (msg->from_io)
    {
        jpf_sysmsg_attach_io(msg_cpy, msg->from_io);
    }

    jpf_sysmsg_unref_priv(msg_cpy->priv);
    msg_cpy->priv = jpf_sysmsg_ref_priv(msg->priv);
	jpf_sysmsg_copy_user_data(msg_cpy, msg);
}


void
jpf_sysmsg_destroy(JpfSysMsg *msg)
{
    G_ASSERT(JPF_IS_SYSMSG(msg));

    jpf_sysmsg_unref_priv(msg->priv);
    jpf_sysmsg_detach_io(msg);
    jpf_sysmsg_free_user_data(msg);
    g_object_unref(msg);
}


JpfSysMsg*
__jpf_sysmsg_new(JpfMsgID msg_id, gpointer priv, gsize size, guint seq,
    JpfMsgPrivDes priv_destroy)
{
    JpfSysMsg *msg;

    msg = g_object_new(JPF_TYPE_SYSMSG, NULL);
    if (G_UNLIKELY(!msg))
        return msg;

    MSG_SETID(msg, msg_id);
    msg->packet_layer.seq = seq;
    jpf_sysmsg_priv_init(msg->priv, priv, size, priv_destroy);

    return msg;
}


JpfSysMsg*
jpf_sysmsg_new(JpfMsgID msg_id, gpointer priv, gsize size,
    guint seq, JpfMsgPrivDes priv_destroy)
{
    if (G_UNLIKELY(priv && !priv_destroy))
        return NULL;

    return __jpf_sysmsg_new(msg_id, priv, size, seq, priv_destroy);
}


JpfSysMsg *
jpf_sysmsg_new_2(JpfMsgID msg_id, gpointer priv, gsize size, guint seq)
{
    JpfSysMsg *msg;
    JpfMsgPrivDes des = NULL;
    gpointer priv_copy = 0;

    if (priv && size)
    {
        priv_copy = jpf_mem_kalloc(size);
        if (G_UNLIKELY(!priv_copy))
            return NULL;

        memcpy(priv_copy, priv, size);
        des = jpf_sysmsg_default_priv_des;
    }

    msg = __jpf_sysmsg_new(msg_id, priv_copy, size, seq, des);
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
jpf_sysmsg_detach_shared_private(JpfSysMsg *msg)
{
	JpfSysMsgPriv *priv = msg->priv;
	BUG_ON(!priv);

	if(g_atomic_int_get(&priv->ref_count) > 1)
	{//@{someone else owns it}
		jpf_sysmsg_unref_priv(priv);
		msg->priv = jpf_new0(JpfSysMsgPriv, 1);
		BUG_ON(!msg->priv);	/* glib says never */
		jpf_sysmsg_priv_init(msg->priv, NULL, 0, NULL);
	}
}


void
jpf_sysmsg_set_private(JpfSysMsg *msg, gpointer priv, gsize size,
    JpfMsgPrivDes priv_destroy)
{
    G_ASSERT(JPF_IS_SYSMSG(msg) && !(priv && !priv_destroy));

	jpf_sysmsg_detach_shared_private(msg);
    jpf_sysmsg_private_set(msg->priv, priv, size, priv_destroy);
}


gint
jpf_sysmsg_set_private_2(JpfSysMsg *msg, gpointer priv, gsize size)
{
    JpfMsgPrivDes des = NULL;
    gpointer priv_copy = 0;
    G_ASSERT(JPF_IS_SYSMSG(msg));

    if (priv && size)
    {
        priv_copy = jpf_mem_kalloc(size);
        if (G_UNLIKELY(!priv_copy))
            return -E_NOMEM;

        memcpy(priv_copy, priv, size);
        des = jpf_sysmsg_default_priv_des;      
    }

    jpf_sysmsg_set_private(msg, priv_copy, size, des);
    return 0;
}


JpfSysMsg *
jpf_sysmsg_copy_one(JpfSysMsg *msg)
{
    JpfSysMsg *msg_cpy = NULL;

    msg_cpy = jpf_sysmsg_new_2(MSG_GETID(msg), NULL, 0, MSG_SEQ(msg));
    if (msg_cpy)
    {
        jpf_sysmsg_copy(msg_cpy, msg);
    }
    return msg_cpy;
}


gint
jpf_sysmsg_set_userdata(JpfSysMsg *msg, gpointer data, gsize size)
{
	gpointer p;
	G_ASSERT(JPF_IS_SYSMSG(msg));

	if (data && size)
	{
		p = jpf_mem_kalloc(size);
		if (!p)
		{
			return -ENOMEM;
		}

		if (msg->user_data)
		{
			jpf_mem_kfree(msg->user_data, msg->user_size);
		}

		msg->user_data = p;
		msg->user_size = size;
		memcpy(msg->user_data, data, size);
		return 0;
	}

	return -EINVAL;
}


gint
jpf_sysmsg_get_userdata(JpfSysMsg *msg, gpointer *pdata, gsize *psize)
{
	G_ASSERT(JPF_IS_SYSMSG(msg));

	if (pdata && psize)
	{
		*pdata = msg->user_data;
		*psize = msg->user_size;
		return 0;
	}

	return -EINVAL;
}


//:~ End
