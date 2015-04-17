#include "nmp_share_errno.h"
#include "nmp_islot.h"

/*
 *	TODO: 
 *		(1) Errno facility!!!!
 *		(2) Thread safe.
 *
*/

G_DEFINE_INTERFACE(NmpISlot, nmp_islot, G_TYPE_OBJECT);

static void
nmp_islot_default_init(NmpISlotInterface *islot_iface)
{//{Empty}
}


gint
nmp_islot_init(NmpISlot *self, GValue *parm)
{
	g_return_val_if_fail(NMP_IS_ISLOT(self), -E_INVAL);

	return NMP_ISLOT_GET_IFACE(self)->init(self, parm);
}


gint
nmp_islot_send(NmpISlot *self, NmpData *data)
{
	g_return_val_if_fail(NMP_IS_ISLOT(self), -E_INVAL);

	return NMP_ISLOT_GET_IFACE(self)->send(self, data);
}


gint
nmp_islot_recv(NmpISlot *self, NmpData *data)
{
	g_return_val_if_fail(NMP_IS_ISLOT(self), -E_INVAL);

	return NMP_ISLOT_GET_IFACE(self)->recv(self, data);
}


gint
nmp_islot_link(NmpISlot *self, NmpISlot *peer)
{
	g_return_val_if_fail(NMP_IS_ISLOT(self), -E_INVAL);
	g_return_val_if_fail(NMP_IS_ISLOT(peer), -E_INVAL);

	return NMP_ISLOT_GET_IFACE(self)->link(self, peer);
}


gint
nmp_islot_unlink(NmpISlot *self)
{
	g_return_val_if_fail(NMP_IS_ISLOT(self), -E_INVAL);

	return NMP_ISLOT_GET_IFACE(self)->unlink(self);
}


gboolean
nmp_islot_ready(NmpISlot *self)
{
	g_return_val_if_fail(NMP_IS_ISLOT(self), FALSE);

	return NMP_ISLOT_GET_IFACE(self)->ready(self);
}


//:~ End
