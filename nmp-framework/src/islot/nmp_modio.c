#include "nmp_errno.h"
#include "nmp_modio.h"
#include "nmp_appmod.h"

static void 
jpf_mod_io_islot_interface_init(JpfISlotInterface *islot_iface);

G_DEFINE_TYPE_WITH_CODE(JpfModIO, jpf_mod_io, JPF_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(JPF_TYPE_ISLOT, jpf_mod_io_islot_interface_init));


static gint
jpf_mod_io_islot_init(JpfISlot *i_self, GValue *parm)
{
    JpfModIO *self;

    g_return_val_if_fail(JPF_IS_MODIO(i_self), -E_INVAL);

    self = JPF_MODIO(i_self);

    return JPF_MODIO_GET_CLASS(self)->slot_init(self, parm);
}


static gint
jpf_mod_io_islot_send(JpfISlot *i_self, JpfData *data)
{
    JpfModIO *self;

    g_return_val_if_fail(JPF_IS_MODIO(i_self), -E_INVAL);
    g_return_val_if_fail(JPF_IS_SYSMSG(data), -E_INVAL);

    self = JPF_MODIO(i_self);
    
    return JPF_MODIO_GET_CLASS(self)->mod_snd(self, JPF_SYSMSG(data));
}


static gint
jpf_mod_io_islot_recv(JpfISlot *i_self, JpfData *data)
{
    JpfModIO *self;

    g_return_val_if_fail(JPF_IS_MODIO(i_self), -E_INVAL);
    g_return_val_if_fail(JPF_IS_SYSMSG(data), -E_INVAL);

    self = JPF_MODIO(i_self);

    return JPF_MODIO_GET_CLASS(self)->mod_rcv(self, JPF_SYSMSG(data));
}


static gint
jpf_mod_io_islot_link(JpfISlot *i_self, JpfISlot *i_peer)
{
    JpfModIO *self;

    g_return_val_if_fail(JPF_IS_MODIO(i_self), -E_INVAL);
    g_return_val_if_fail(JPF_IS_ISLOT(i_peer), -E_INVAL);

    self = JPF_MODIO(i_self);

    return JPF_MODIO_GET_CLASS(self)->connect(self, i_peer);
}



static gint
jpf_mod_io_islot_unlink(JpfISlot *i_self)
{
    JpfModIO *self;

    g_return_val_if_fail(JPF_IS_MODIO(i_self), -E_INVAL);

    self = JPF_MODIO(i_self);

    return JPF_MODIO_GET_CLASS(self)->disconnect(self);
}


static gboolean
jpf_mod_io_islot_ready(JpfISlot *i_self)
{
    JpfModIO *self;

    g_return_val_if_fail(JPF_IS_MODIO(i_self), FALSE);

    self = JPF_MODIO(i_self);

    return !JPF_MODIO_GET_CLASS(self)->slot_ok(self);
}

/*
 * JpfModIO
 * virtual functions of JpfModIO class.
*/
static gint
jpf_mod_io_slot_init(JpfModIO *self, GValue *parm)
{
    gpointer pt = parm->data[0].v_pointer;

    g_return_val_if_fail(JPF_IS_APPMOD(pt), -E_INVAL);

    self->owner = pt;

    return 0;
}


static gint
jpf_mod_io_mod_snd(JpfModIO *self, JpfSysMsg *msg)
{
    if (G_UNLIKELY(!self->i_peer))
        return -E_SLOTNCONN;

    return jpf_islot_recv(self->i_peer, JPF_DATA(msg));
}


static gint
jpf_mod_io_mod_rcv(JpfModIO *self, JpfSysMsg *msg)
{
    JpfAppMod *owner = self->owner;

    if (G_UNLIKELY(!owner))
        return -E_OWNER;

	MSG_FROM_BUS(msg);

    return jpf_app_mod_rcv_b(owner, msg);
}


static gint
jpf_mod_io_connect(JpfModIO *self, JpfISlot *i_peer)
{
    if (G_UNLIKELY(self->i_peer))
        return -E_SLOTCONND;

    if (G_UNLIKELY(!jpf_islot_ready(i_peer)))
        return -E_SLOTNREADY;

    self->i_peer = i_peer;

    return 0;   
}


static gint
jpf_mod_io_disconnect(JpfModIO *self)
{
    self->i_peer = NULL;

    return 0;   
}


static gint
jpf_mod_io_slot_ok(JpfModIO *self)
{
    if (self->owner)
        return 0;

    return -E_SLOTNREADY;
}


static void
jpf_mod_io_init(JpfModIO *self)
{
    self->owner = NULL;
    self->i_peer = NULL;
}


static void
jpf_mod_io_class_init(JpfModIOClass *c_self)
{
    c_self->slot_init = jpf_mod_io_slot_init;
    c_self->mod_snd = jpf_mod_io_mod_snd;
    c_self->mod_rcv = jpf_mod_io_mod_rcv;
    c_self->connect = jpf_mod_io_connect;
    c_self->disconnect = jpf_mod_io_disconnect;
    c_self->slot_ok = jpf_mod_io_slot_ok;
}


static void 
jpf_mod_io_islot_interface_init(JpfISlotInterface *islot_iface)
{
    islot_iface->init = jpf_mod_io_islot_init;
    islot_iface->send = jpf_mod_io_islot_send;
    islot_iface->recv = jpf_mod_io_islot_recv;
    islot_iface->link = jpf_mod_io_islot_link;
    islot_iface->unlink = jpf_mod_io_islot_unlink;
    islot_iface->ready = jpf_mod_io_islot_ready;    
}


//:~ End
