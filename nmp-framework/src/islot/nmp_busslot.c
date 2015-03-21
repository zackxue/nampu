#include "nmp_errno.h"
#include "nmp_busslot.h"

/*
 * NmpBusSlot
 * for NmpISlot interface implementation
*/

static void 
nmp_bus_slot_islot_interface_init(NmpISlotInterface *islot_iface);

G_DEFINE_TYPE_WITH_CODE(NmpBusSlot, nmp_bus_slot, NMP_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(NMP_TYPE_ISLOT, nmp_bus_slot_islot_interface_init));


static gint
nmp_bus_slot_islot_init(NmpISlot *i_self, GValue *parm)
{
	NmpBusSlot *self;

	g_return_val_if_fail(NMP_IS_BUSSLOT(i_self), -E_INVAL);

	self = NMP_BUSSLOT(i_self);

	return NMP_BUSSLOT_GET_CLASS(self)->slot_init(self, parm);
}


static gint
nmp_bus_slot_islot_send(NmpISlot *i_self, NmpData *data)
{
	NmpBusSlot *self;

	g_return_val_if_fail(NMP_IS_BUSSLOT(i_self), -E_INVAL);
	g_return_val_if_fail(NMP_IS_SYSMSG(data), -E_INVAL);

	self = NMP_BUSSLOT(i_self);
	
	return NMP_BUSSLOT_GET_CLASS(self)->bus_snd(self, NMP_SYSMSG(data));
}


static gint
nmp_bus_slot_islot_recv(NmpISlot *i_self, NmpData *data)
{
	NmpBusSlot *self;

	g_return_val_if_fail(NMP_IS_BUSSLOT(i_self), -E_INVAL);
	g_return_val_if_fail(NMP_IS_SYSMSG(data), -E_INVAL);

	self = NMP_BUSSLOT(i_self);

	return NMP_BUSSLOT_GET_CLASS(self)->bus_rcv(self, NMP_SYSMSG(data));
}


static gint
nmp_bus_slot_islot_link(NmpISlot *i_self, NmpISlot *i_peer)
{
	NmpBusSlot *self;

	g_return_val_if_fail(NMP_IS_BUSSLOT(i_self), -E_INVAL);
	g_return_val_if_fail(NMP_IS_ISLOT(i_peer), -E_INVAL);

	self = NMP_BUSSLOT(i_self);

	return NMP_BUSSLOT_GET_CLASS(self)->connect(self, i_peer);
}



static gint
nmp_bus_slot_islot_unlink(NmpISlot *i_self)
{
	NmpBusSlot *self;

	g_return_val_if_fail(NMP_IS_BUSSLOT(i_self), -E_INVAL);

	self = NMP_BUSSLOT(i_self);

	return NMP_BUSSLOT_GET_CLASS(self)->disconnect(self);
}


static gboolean
nmp_bus_slot_islot_ready(NmpISlot *i_self)
{
	NmpBusSlot *self;

	g_return_val_if_fail(NMP_IS_BUSSLOT(i_self), FALSE);

	self = NMP_BUSSLOT(i_self);

	return !NMP_BUSSLOT_GET_CLASS(self)->slot_ok(self);
}


/*
 * virtual functions of NmpBusSlot, they can be overrided for
 * enhancement.
*/
static gint
nmp_bus_slot_slot_init(NmpBusSlot *self, GValue *parm)
{
	gint err = -E_INVAL;
	NmpMsgBus *owner = (NmpMsgBus*)parm->data[0].v_pointer;

	if (G_UNLIKELY(!NMP_IS_MSGBUS(owner)))
		return err;

	if (!(err = nmp_msg_bus_request_slot(owner, self,
			(NmpBusSlotPos)parm->data[1].v_int)))
	{
		self->owner = owner;
		self->bus_ready = TRUE;
		self->ibus_slot = (NmpBusSlotPos)parm->data[1].v_int;
	}

	return err;
}


static gint
nmp_bus_slot_bus_snd(NmpBusSlot *self, NmpSysMsg *msg)
{
	if (G_UNLIKELY(!self->i_peer))
		return -E_SLOTNCONN;

	return nmp_islot_recv(self->i_peer, NMP_DATA(msg));
}


static gint
nmp_bus_slot_bus_rcv(NmpBusSlot *self, NmpSysMsg *msg)
{
	if (G_UNLIKELY(!self->bus_ready))
		return -E_SLOTNREADY;

	return  nmp_msg_bus_rcv_msg(self->owner, self->ibus_slot, msg);
}


static gint
nmp_bus_slot_connect(NmpBusSlot *self, NmpISlot *i_peer)
{
	if (G_UNLIKELY(self->i_peer))
		return -E_SLOTCONND;

	if (G_UNLIKELY(!nmp_islot_ready(i_peer)))
		return -E_SLOTNREADY;

	self->i_peer = i_peer;
	return 0;
}


static gint
nmp_bus_slot_disconnect(NmpBusSlot *self)
{
	self->i_peer = NULL;
	return 0;
}


static gint
nmp_bus_slot_slot_ok(NmpBusSlot *self)
{
	if (self->bus_ready)
		return 0;

	return -E_SLOTNREADY;
}


static void
nmp_bus_slot_init(NmpBusSlot *self)
{
	self->owner = NULL;
	self->i_peer = NULL;
	self->bus_ready = FALSE;
	self->ibus_slot = BUSSLOT_POS_MAX;
}


static void
nmp_bus_slot_class_init(NmpBusSlotClass *c_self)
{
	c_self->slot_init = nmp_bus_slot_slot_init;
	c_self->bus_snd = nmp_bus_slot_bus_snd;
	c_self->bus_rcv = nmp_bus_slot_bus_rcv;
	c_self->connect = nmp_bus_slot_connect;
	c_self->disconnect = nmp_bus_slot_disconnect;
	c_self->slot_ok = nmp_bus_slot_slot_ok;
}


static void
nmp_bus_slot_islot_interface_init(NmpISlotInterface *islot_iface)
{
	islot_iface->init = nmp_bus_slot_islot_init;
	islot_iface->send = nmp_bus_slot_islot_send;
	islot_iface->recv = nmp_bus_slot_islot_recv;
	islot_iface->link = nmp_bus_slot_islot_link;
	islot_iface->unlink = nmp_bus_slot_islot_unlink;
	islot_iface->ready = nmp_bus_slot_islot_ready;
}


//:~ End
