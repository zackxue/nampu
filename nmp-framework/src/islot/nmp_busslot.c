#include "nmp_errno.h"
#include "nmp_busslot.h"

/*
 * JpfBusSlot
 * for JpfISlot interface implementation
*/

static void 
jpf_bus_slot_islot_interface_init(JpfISlotInterface *islot_iface);

G_DEFINE_TYPE_WITH_CODE(JpfBusSlot, jpf_bus_slot, NMP_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(NMP_TYPE_ISLOT, jpf_bus_slot_islot_interface_init));


static gint
jpf_bus_slot_islot_init(JpfISlot *i_self, GValue *parm)
{
	JpfBusSlot *self;

	g_return_val_if_fail(NMP_IS_BUSSLOT(i_self), -E_INVAL);

	self = NMP_BUSSLOT(i_self);

	return NMP_BUSSLOT_GET_CLASS(self)->slot_init(self, parm);
}


static gint
jpf_bus_slot_islot_send(JpfISlot *i_self, JpfData *data)
{
	JpfBusSlot *self;

	g_return_val_if_fail(NMP_IS_BUSSLOT(i_self), -E_INVAL);
	g_return_val_if_fail(NMP_IS_SYSMSG(data), -E_INVAL);

	self = NMP_BUSSLOT(i_self);
	
	return NMP_BUSSLOT_GET_CLASS(self)->bus_snd(self, NMP_SYSMSG(data));
}


static gint
jpf_bus_slot_islot_recv(JpfISlot *i_self, JpfData *data)
{
	JpfBusSlot *self;

	g_return_val_if_fail(NMP_IS_BUSSLOT(i_self), -E_INVAL);
	g_return_val_if_fail(NMP_IS_SYSMSG(data), -E_INVAL);

	self = NMP_BUSSLOT(i_self);

	return NMP_BUSSLOT_GET_CLASS(self)->bus_rcv(self, NMP_SYSMSG(data));
}


static gint
jpf_bus_slot_islot_link(JpfISlot *i_self, JpfISlot *i_peer)
{
	JpfBusSlot *self;

	g_return_val_if_fail(NMP_IS_BUSSLOT(i_self), -E_INVAL);
	g_return_val_if_fail(NMP_IS_ISLOT(i_peer), -E_INVAL);

	self = NMP_BUSSLOT(i_self);

	return NMP_BUSSLOT_GET_CLASS(self)->connect(self, i_peer);
}



static gint
jpf_bus_slot_islot_unlink(JpfISlot *i_self)
{
	JpfBusSlot *self;

	g_return_val_if_fail(NMP_IS_BUSSLOT(i_self), -E_INVAL);

	self = NMP_BUSSLOT(i_self);

	return NMP_BUSSLOT_GET_CLASS(self)->disconnect(self);
}


static gboolean
jpf_bus_slot_islot_ready(JpfISlot *i_self)
{
	JpfBusSlot *self;

	g_return_val_if_fail(NMP_IS_BUSSLOT(i_self), FALSE);

	self = NMP_BUSSLOT(i_self);

	return !NMP_BUSSLOT_GET_CLASS(self)->slot_ok(self);
}


/*
 * virtual functions of JpfBusSlot, they can be overrided for
 * enhancement.
*/
static gint
jpf_bus_slot_slot_init(JpfBusSlot *self, GValue *parm)
{
	gint err = -E_INVAL;
	JpfMsgBus *owner = (JpfMsgBus*)parm->data[0].v_pointer;

	if (G_UNLIKELY(!NMP_IS_MSGBUS(owner)))
		return err;

	if (!(err = jpf_msg_bus_request_slot(owner, self,
			(JpfBusSlotPos)parm->data[1].v_int)))
	{
		self->owner = owner;
		self->bus_ready = TRUE;
		self->ibus_slot = (JpfBusSlotPos)parm->data[1].v_int;
	}

	return err;
}


static gint
jpf_bus_slot_bus_snd(JpfBusSlot *self, NmpSysMsg *msg)
{
	if (G_UNLIKELY(!self->i_peer))
		return -E_SLOTNCONN;

	return jpf_islot_recv(self->i_peer, NMP_DATA(msg));
}


static gint
jpf_bus_slot_bus_rcv(JpfBusSlot *self, NmpSysMsg *msg)
{
	if (G_UNLIKELY(!self->bus_ready))
		return -E_SLOTNREADY;

	return  jpf_msg_bus_rcv_msg(self->owner, self->ibus_slot, msg);
}


static gint
jpf_bus_slot_connect(JpfBusSlot *self, JpfISlot *i_peer)
{
	if (G_UNLIKELY(self->i_peer))
		return -E_SLOTCONND;

	if (G_UNLIKELY(!jpf_islot_ready(i_peer)))
		return -E_SLOTNREADY;

	self->i_peer = i_peer;
	return 0;
}


static gint
jpf_bus_slot_disconnect(JpfBusSlot *self)
{
	self->i_peer = NULL;
	return 0;
}


static gint
jpf_bus_slot_slot_ok(JpfBusSlot *self)
{
	if (self->bus_ready)
		return 0;

	return -E_SLOTNREADY;
}


static void
jpf_bus_slot_init(JpfBusSlot *self)
{
	self->owner = NULL;
	self->i_peer = NULL;
	self->bus_ready = FALSE;
	self->ibus_slot = BUSSLOT_POS_MAX;
}


static void
jpf_bus_slot_class_init(JpfBusSlotClass *c_self)
{
	c_self->slot_init = jpf_bus_slot_slot_init;
	c_self->bus_snd = jpf_bus_slot_bus_snd;
	c_self->bus_rcv = jpf_bus_slot_bus_rcv;
	c_self->connect = jpf_bus_slot_connect;
	c_self->disconnect = jpf_bus_slot_disconnect;
	c_self->slot_ok = jpf_bus_slot_slot_ok;
}


static void
jpf_bus_slot_islot_interface_init(JpfISlotInterface *islot_iface)
{
	islot_iface->init = jpf_bus_slot_islot_init;
	islot_iface->send = jpf_bus_slot_islot_send;
	islot_iface->recv = jpf_bus_slot_islot_recv;
	islot_iface->link = jpf_bus_slot_islot_link;
	islot_iface->unlink = jpf_bus_slot_islot_unlink;
	islot_iface->ready = jpf_bus_slot_islot_ready;
}


//:~ End
