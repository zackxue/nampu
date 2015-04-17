#include "nmp_share_errno.h"
#include "nmp_modio.h"
#include "nmp_appmod.h"

static void 
nmp_mod_io_islot_interface_init(NmpISlotInterface *islot_iface);

G_DEFINE_TYPE_WITH_CODE(NmpModIO, nmp_mod_io, NMP_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(NMP_TYPE_ISLOT, nmp_mod_io_islot_interface_init));


static gint
nmp_mod_io_islot_init(NmpISlot *i_self, GValue *parm)
{
    NmpModIO *self;

    g_return_val_if_fail(NMP_IS_MODIO(i_self), -E_INVAL);

    self = NMP_MODIO(i_self);

    return NMP_MODIO_GET_CLASS(self)->slot_init(self, parm);
}


static gint
nmp_mod_io_islot_send(NmpISlot *i_self, NmpData *data)
{
    NmpModIO *self;

    g_return_val_if_fail(NMP_IS_MODIO(i_self), -E_INVAL);
    g_return_val_if_fail(NMP_IS_SYSMSG(data), -E_INVAL);

    self = NMP_MODIO(i_self);
    
    return NMP_MODIO_GET_CLASS(self)->mod_snd(self, NMP_SYSMSG(data));
}


static gint
nmp_mod_io_islot_recv(NmpISlot *i_self, NmpData *data)
{
    NmpModIO *self;

    g_return_val_if_fail(NMP_IS_MODIO(i_self), -E_INVAL);
    g_return_val_if_fail(NMP_IS_SYSMSG(data), -E_INVAL);

    self = NMP_MODIO(i_self);

    return NMP_MODIO_GET_CLASS(self)->mod_rcv(self, NMP_SYSMSG(data));
}


static gint
nmp_mod_io_islot_link(NmpISlot *i_self, NmpISlot *i_peer)
{
    NmpModIO *self;

    g_return_val_if_fail(NMP_IS_MODIO(i_self), -E_INVAL);
    g_return_val_if_fail(NMP_IS_ISLOT(i_peer), -E_INVAL);

    self = NMP_MODIO(i_self);

    return NMP_MODIO_GET_CLASS(self)->connect(self, i_peer);
}



static gint
nmp_mod_io_islot_unlink(NmpISlot *i_self)
{
    NmpModIO *self;

    g_return_val_if_fail(NMP_IS_MODIO(i_self), -E_INVAL);

    self = NMP_MODIO(i_self);

    return NMP_MODIO_GET_CLASS(self)->disconnect(self);
}


static gboolean
nmp_mod_io_islot_ready(NmpISlot *i_self)
{
    NmpModIO *self;

    g_return_val_if_fail(NMP_IS_MODIO(i_self), FALSE);

    self = NMP_MODIO(i_self);

    return !NMP_MODIO_GET_CLASS(self)->slot_ok(self);
}

/*
 * NmpModIO
 * virtual functions of NmpModIO class.
*/
static gint
nmp_mod_io_slot_init(NmpModIO *self, GValue *parm)
{
    gpointer pt = parm->data[0].v_pointer;

    g_return_val_if_fail(NMP_IS_APPMOD(pt), -E_INVAL);

    self->owner = pt;

    return 0;
}


static gint
nmp_mod_io_mod_snd(NmpModIO *self, NmpSysMsg *msg)
{
    if (G_UNLIKELY(!self->i_peer))
        return -E_SLOTNCONN;

    return nmp_islot_recv(self->i_peer, NMP_DATA(msg));
}


static gint
nmp_mod_io_mod_rcv(NmpModIO *self, NmpSysMsg *msg)
{
    NmpAppMod *owner = self->owner;

    if (G_UNLIKELY(!owner))
        return -E_OWNER;

	MSG_FROM_BUS(msg);

    return nmp_app_mod_rcv_b(owner, msg);
}


static gint
nmp_mod_io_connect(NmpModIO *self, NmpISlot *i_peer)
{
    if (G_UNLIKELY(self->i_peer))
        return -E_SLOTCONND;

    if (G_UNLIKELY(!nmp_islot_ready(i_peer)))
        return -E_SLOTNREADY;

    self->i_peer = i_peer;

    return 0;   
}


static gint
nmp_mod_io_disconnect(NmpModIO *self)
{
    self->i_peer = NULL;

    return 0;   
}


static gint
nmp_mod_io_slot_ok(NmpModIO *self)
{
    if (self->owner)
        return 0;

    return -E_SLOTNREADY;
}


static void
nmp_mod_io_init(NmpModIO *self)
{
    self->owner = NULL;
    self->i_peer = NULL;
}


static void
nmp_mod_io_class_init(NmpModIOClass *c_self)
{
    c_self->slot_init = nmp_mod_io_slot_init;
    c_self->mod_snd = nmp_mod_io_mod_snd;
    c_self->mod_rcv = nmp_mod_io_mod_rcv;
    c_self->connect = nmp_mod_io_connect;
    c_self->disconnect = nmp_mod_io_disconnect;
    c_self->slot_ok = nmp_mod_io_slot_ok;
}


static void 
nmp_mod_io_islot_interface_init(NmpISlotInterface *islot_iface)
{
    islot_iface->init = nmp_mod_io_islot_init;
    islot_iface->send = nmp_mod_io_islot_send;
    islot_iface->recv = nmp_mod_io_islot_recv;
    islot_iface->link = nmp_mod_io_islot_link;
    islot_iface->unlink = nmp_mod_io_islot_unlink;
    islot_iface->ready = nmp_mod_io_islot_ready;    
}


//:~ End
