/*
 * jpf_appmod.c
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
*/

#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_appmod.h"

G_DEFINE_TYPE(JpfAppMod, jpf_app_mod, JPF_TYPE_APPOBJ);


void
jpf_app_mod_set_name(JpfAppMod *self, const gchar *name)
{
	G_ASSERT(self != NULL && name != NULL);

	self->name[MAX_MOD_NAME_LEN - 1] = 0;
	strncpy(self->name, name, MAX_MOD_NAME_LEN - 1);
	jpf_msg_engine_set_name(self->msg_engine, name);
}


const gchar *
jpf_app_mod_get_name(JpfAppMod *self)
{
	G_ASSERT(self != NULL);

	return self->name;
}


static __inline__ void
jpf_app_mod_init_zero(JpfAppMod *self)
{
    self->io_mod = NULL;
    self->msg_engine = NULL;
}


static void
jpf_app_mod_init(JpfAppMod *self)
{
    GValue v = {0, };
    jpf_app_mod_init_zero(self);

    self->io_mod = g_object_new(JPF_TYPE_MODIO, NULL);
    if (G_UNLIKELY(!self->io_mod))
    {
        jpf_error("<JpfAppMod> alloc MOD-I/O object failed!");
        FATAL_ERROR_EXIT;
    }
    
    self->event_table = jpf_event_table_new(self);
    if (G_UNLIKELY(!self->event_table))
    {
         jpf_error("<JpfAppMod> alloc etable object failed!");
        FATAL_ERROR_EXIT;   	
    }

    self->msg_engine = g_object_new(JPF_TYPE_MSGENGINE, NULL);
    if (G_UNLIKELY(!self->msg_engine))
    {
        jpf_error("<JpfAppMod> alloc msg engine failed!");
        FATAL_ERROR_EXIT;
    }

#ifdef MOD_HAS_TIMER
	self->timer_engine = jpf_timer_engine_new();
	if (G_UNLIKELY(!self->timer_engine))
	{
		jpf_error("<JpfAppMod> alloc time engine failed!");
		FATAL_ERROR_EXIT;
	}
#endif

	jpf_app_mod_set_name(self, "mod-x");

    jpf_msg_engine_set_owner(self->msg_engine, (JpfAppObj*)self);

    v.data[0].v_pointer = self;
    jpf_islot_init(JPF_ISLOT(self->io_mod), &v);
}


static void
jpf_app_mod_dispose(GObject *object)
{
    JpfAppMod *self = (JpfAppMod*)object;

    if (self->io_mod)
    {
        g_object_unref(self->io_mod);
        self->io_mod = NULL;
    }

    if (self->event_table)
    {
    	jpf_event_table_destroy(self->event_table);
    	self->event_table = NULL;
    }

    if (self->msg_engine)
    {
        g_object_unref(self->msg_engine);
        self->msg_engine = NULL;
    }

    G_OBJECT_CLASS(jpf_app_mod_parent_class)->dispose(object);
}


static void 
jpf_app_mod_deliver_out(JpfAppObj *s_self, JpfSysMsg *msg)
{
    JpfAppMod *self = (JpfAppMod*)s_self;

    BUG_ON(!JPF_IS_APPMOD(self));
    BUG_ON(!JPF_IS_SYSMSG(msg));

    if (G_UNLIKELY(!self->io_mod))
    {
    	jpf_warning("<JpfAppObj> deliver_out() failed, no mod-io!");
    	goto deliver_out_err;
    }

    if (jpf_islot_send(JPF_ISLOT(self->io_mod), JPF_DATA(msg)))
    {
    	jpf_warning("<JpfAppObj> deliver_out() failed, can't send to bus!");
    	goto deliver_out_err;
    }

	return ;

deliver_out_err:
	jpf_sysmsg_destroy(msg);
}


gint
jpf_app_mod_hook_from_bus(JpfAppObj *s_self, JpfSysMsg *msg)
{
	JpfAppMod *self;
	G_ASSERT(s_self != NULL && msg != NULL);
	
	self = (JpfAppMod*)s_self;
	return jpf_event_response(self->event_table, msg);
}


static void 
jpf_app_mod_deliver_in(JpfAppObj *s_self, JpfSysMsg *msg)
{
	JpfAppMod *self = (JpfAppMod*)s_self;
	
	BUG_ON(!JPF_IS_APPMOD(self));
	
	JPF_APPMOD_GET_CLASS(self)->consume_msg(self, msg);
}


gint
jpf_app_mod_register_msg(JpfAppMod *self, JpfMsgID msg_id,
	JpfMsgFun f_fun, JpfMsgFun b_fun, guint flags)
{
	G_ASSERT(self != NULL);
	
	return jpf_msg_engine_register_msg(
		self->msg_engine, msg_id, f_fun, b_fun, flags);
}


static void
jpf_app_mod_consume_msg(JpfAppMod *self, JpfSysMsg *msg)
{
	jpf_warning("<JpfAppMod> default consume_msg().");
	jpf_sysmsg_destroy(msg);
}


static gint
jpf_app_mod_setup_default(JpfAppMod *self)
{
	jpf_warning("<JpfAppMod> default setup_mod().");
	return 0;
}


void
jpf_app_mod_setup(JpfAppMod *self)
{
	G_ASSERT(JPF_IS_APPMOD(self));

	JPF_APPMOD_GET_CLASS(self)->setup_mod(self);
}


static void
jpf_app_mod_class_init(JpfAppModClass *c_self)
{
    GObjectClass *object_class = (GObjectClass*)c_self;
    JpfAppObjClass *appobj_class = (JpfAppObjClass*)c_self;

    object_class->dispose = jpf_app_mod_dispose;

    appobj_class->deliver_in = jpf_app_mod_deliver_in;
    appobj_class->deliver_out = jpf_app_mod_deliver_out;
    appobj_class->hook_from_bus = jpf_app_mod_hook_from_bus;

    c_self->consume_msg = jpf_app_mod_consume_msg;
    c_self->setup_mod = jpf_app_mod_setup_default;
}


gint
jpf_app_mod_snd(JpfAppMod *self, JpfSysMsg *msg)
{
    g_return_val_if_fail(JPF_IS_APPMOD(self), -E_INVAL);

    if (G_UNLIKELY(!self->msg_engine))
        return -E_NOMSGENG;

    jpf_app_mod_deliver_out((JpfAppObj*)self, msg);
    return 0;
}


gint
jpf_app_mod_rcv_f(JpfAppMod *self, JpfSysMsg *msg)
{
    g_return_val_if_fail(JPF_IS_APPMOD(self), -E_INVAL);

    if (G_UNLIKELY(!self->msg_engine))
        return -E_NOMSGENG;

    return jpf_msg_engine_push_msg_f(self->msg_engine, msg);
}


gint
jpf_app_mod_rcv_b(JpfAppMod *self, JpfSysMsg *msg)
{
    g_return_val_if_fail(JPF_IS_APPMOD(self), -E_INVAL);

    if (G_UNLIKELY(!self->msg_engine))
        return -E_NOMSGENG;

    return jpf_msg_engine_push_msg_b(self->msg_engine, msg);
}


#ifdef MOD_HAS_TIMER

guint
jpf_app_mod_set_timer(JpfAppMod *self, guint interval,
	JpfTimerFun fun, gpointer data)
{
	G_ASSERT(self != NULL);
	
	return jpf_timer_engine_set_timer(self->timer_engine,
		interval, fun, data);
}


void
jpf_app_mod_del_timer(JpfAppMod *self, gint id)
{
	G_ASSERT(self != NULL);
	
	jpf_timer_engine_del_timer(self->timer_engine, id);
}

#endif	//MOD_HAS_TIMER


gint
jpf_app_mod_sync_request(JpfAppMod *self, JpfSysMsg **msg)
{
	G_ASSERT(self != NULL);

	return jpf_event_request(self->event_table, msg);
}


//:~ End
