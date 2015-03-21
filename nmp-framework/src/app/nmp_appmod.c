/*
 * jpf_appmod.c
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_appmod.h"

G_DEFINE_TYPE(NmpAppMod, nmp_app_mod, NMP_TYPE_APPOBJ);


void
nmp_app_mod_set_name(NmpAppMod *self, const gchar *name)
{
	G_ASSERT(self != NULL && name != NULL);

	self->name[MAX_MOD_NAME_LEN - 1] = 0;
	strncpy(self->name, name, MAX_MOD_NAME_LEN - 1);
	jpf_msg_engine_set_name(self->msg_engine, name);
}


const gchar *
nmp_app_mod_get_name(NmpAppMod *self)
{
	G_ASSERT(self != NULL);

	return self->name;
}


static __inline__ void
nmp_app_mod_init_zero(NmpAppMod *self)
{
    self->io_mod = NULL;
    self->msg_engine = NULL;
}


static void
nmp_app_mod_init(NmpAppMod *self)
{
    GValue v = {0, };
    nmp_app_mod_init_zero(self);

    self->io_mod = g_object_new(NMP_TYPE_MODIO, NULL);
    if (G_UNLIKELY(!self->io_mod))
    {
        jpf_error("<NmpAppMod> alloc MOD-I/O object failed!");
        FATAL_ERROR_EXIT;
    }
    
    self->event_table = jpf_event_table_new(self);
    if (G_UNLIKELY(!self->event_table))
    {
         jpf_error("<NmpAppMod> alloc etable object failed!");
        FATAL_ERROR_EXIT;   	
    }

    self->msg_engine = g_object_new(NMP_TYPE_MSGENGINE, NULL);
    if (G_UNLIKELY(!self->msg_engine))
    {
        jpf_error("<NmpAppMod> alloc msg engine failed!");
        FATAL_ERROR_EXIT;
    }

#ifdef MOD_HAS_TIMER
	self->timer_engine = jpf_timer_engine_new();
	if (G_UNLIKELY(!self->timer_engine))
	{
		jpf_error("<NmpAppMod> alloc time engine failed!");
		FATAL_ERROR_EXIT;
	}
#endif

	nmp_app_mod_set_name(self, "mod-x");

    jpf_msg_engine_set_owner(self->msg_engine, (NmpAppObj*)self);

    v.data[0].v_pointer = self;
    jpf_islot_init(NMP_ISLOT(self->io_mod), &v);
}


static void
nmp_app_mod_dispose(GObject *object)
{
    NmpAppMod *self = (NmpAppMod*)object;

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

    G_OBJECT_CLASS(nmp_app_mod_parent_class)->dispose(object);
}


static void 
nmp_app_mod_deliver_out(NmpAppObj *s_self, NmpSysMsg *msg)
{
    NmpAppMod *self = (NmpAppMod*)s_self;

    BUG_ON(!NMP_IS_APPMOD(self));
    BUG_ON(!NMP_IS_SYSMSG(msg));

    if (G_UNLIKELY(!self->io_mod))
    {
    	jpf_warning("<NmpAppObj> deliver_out() failed, no mod-io!");
    	goto deliver_out_err;
    }

    if (jpf_islot_send(NMP_ISLOT(self->io_mod), NMP_DATA(msg)))
    {
    	jpf_warning("<NmpAppObj> deliver_out() failed, can't send to bus!");
    	goto deliver_out_err;
    }

	return ;

deliver_out_err:
	jpf_sysmsg_destroy(msg);
}


gint
nmp_app_mod_hook_from_bus(NmpAppObj *s_self, NmpSysMsg *msg)
{
	NmpAppMod *self;
	G_ASSERT(s_self != NULL && msg != NULL);
	
	self = (NmpAppMod*)s_self;
	return jpf_event_response(self->event_table, msg);
}


static void 
nmp_app_mod_deliver_in(NmpAppObj *s_self, NmpSysMsg *msg)
{
	NmpAppMod *self = (NmpAppMod*)s_self;
	
	BUG_ON(!NMP_IS_APPMOD(self));
	
	NMP_APPMOD_GET_CLASS(self)->consume_msg(self, msg);
}


gint
nmp_app_mod_register_msg(NmpAppMod *self, NmpMsgID msg_id,
	NmpMsgFun f_fun, NmpMsgFun b_fun, guint flags)
{
	G_ASSERT(self != NULL);
	
	return jpf_msg_engine_register_msg(
		self->msg_engine, msg_id, f_fun, b_fun, flags);
}


static void
nmp_app_mod_consume_msg(NmpAppMod *self, NmpSysMsg *msg)
{
	jpf_warning("<NmpAppMod> default consume_msg().");
	jpf_sysmsg_destroy(msg);
}


static gint
nmp_app_mod_setup_default(NmpAppMod *self)
{
	jpf_warning("<NmpAppMod> default setup_mod().");
	return 0;
}


void
nmp_app_mod_setup(NmpAppMod *self)
{
	G_ASSERT(NMP_IS_APPMOD(self));

	NMP_APPMOD_GET_CLASS(self)->setup_mod(self);
}


static void
nmp_app_mod_class_init(NmpAppModClass *c_self)
{
    GObjectClass *object_class = (GObjectClass*)c_self;
    NmpAppObjClass *appobj_class = (NmpAppObjClass*)c_self;

    object_class->dispose = nmp_app_mod_dispose;

    appobj_class->deliver_in = nmp_app_mod_deliver_in;
    appobj_class->deliver_out = nmp_app_mod_deliver_out;
    appobj_class->hook_from_bus = nmp_app_mod_hook_from_bus;

    c_self->consume_msg = nmp_app_mod_consume_msg;
    c_self->setup_mod = nmp_app_mod_setup_default;
}


gint
nmp_app_mod_snd(NmpAppMod *self, NmpSysMsg *msg)
{
    g_return_val_if_fail(NMP_IS_APPMOD(self), -E_INVAL);

    if (G_UNLIKELY(!self->msg_engine))
        return -E_NOMSGENG;

    nmp_app_mod_deliver_out((NmpAppObj*)self, msg);
    return 0;
}


gint
nmp_app_mod_rcv_f(NmpAppMod *self, NmpSysMsg *msg)
{
    g_return_val_if_fail(NMP_IS_APPMOD(self), -E_INVAL);

    if (G_UNLIKELY(!self->msg_engine))
        return -E_NOMSGENG;

    return jpf_msg_engine_push_msg_f(self->msg_engine, msg);
}


gint
nmp_app_mod_rcv_b(NmpAppMod *self, NmpSysMsg *msg)
{
    g_return_val_if_fail(NMP_IS_APPMOD(self), -E_INVAL);

    if (G_UNLIKELY(!self->msg_engine))
        return -E_NOMSGENG;

    return jpf_msg_engine_push_msg_b(self->msg_engine, msg);
}


#ifdef MOD_HAS_TIMER

guint
nmp_app_mod_set_timer(NmpAppMod *self, guint interval,
	NmpTimerFun fun, gpointer data)
{
	G_ASSERT(self != NULL);
	
	return jpf_timer_engine_set_timer(self->timer_engine,
		interval, fun, data);
}


void
nmp_app_mod_del_timer(NmpAppMod *self, gint id)
{
	G_ASSERT(self != NULL);
	
	jpf_timer_engine_del_timer(self->timer_engine, id);
}

#endif	//MOD_HAS_TIMER


gint
nmp_app_mod_sync_request(NmpAppMod *self, NmpSysMsg **msg)
{
	G_ASSERT(self != NULL);

	return jpf_event_request(self->event_table, msg);
}


//:~ End
