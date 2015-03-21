#include "nmp_errno.h"
#include "nmp_islot.h"

/*
 *	TODO: 
 *		(1) Errno facility!!!!
 *		(2) Thread safe.
 *
*/

G_DEFINE_INTERFACE(JpfISlot, jpf_islot, G_TYPE_OBJECT);

static void
jpf_islot_default_init(JpfISlotInterface *islot_iface)
{//{Empty}
}


gint
jpf_islot_init(JpfISlot *self, GValue *parm)
{
	g_return_val_if_fail(NMP_IS_ISLOT(self), -E_INVAL);

	return NMP_ISLOT_GET_IFACE(self)->init(self, parm);
}


gint
jpf_islot_send(JpfISlot *self, JpfData *data)
{
	g_return_val_if_fail(NMP_IS_ISLOT(self), -E_INVAL);

	return NMP_ISLOT_GET_IFACE(self)->send(self, data);
}


gint
jpf_islot_recv(JpfISlot *self, JpfData *data)
{
	g_return_val_if_fail(NMP_IS_ISLOT(self), -E_INVAL);

	return NMP_ISLOT_GET_IFACE(self)->recv(self, data);
}


gint
jpf_islot_link(JpfISlot *self, JpfISlot *peer)
{
	g_return_val_if_fail(NMP_IS_ISLOT(self), -E_INVAL);
	g_return_val_if_fail(NMP_IS_ISLOT(peer), -E_INVAL);

	return NMP_ISLOT_GET_IFACE(self)->link(self, peer);
}


gint
jpf_islot_unlink(JpfISlot *self)
{
	g_return_val_if_fail(NMP_IS_ISLOT(self), -E_INVAL);

	return NMP_ISLOT_GET_IFACE(self)->unlink(self);
}


gboolean
jpf_islot_ready(JpfISlot *self)
{
	g_return_val_if_fail(NMP_IS_ISLOT(self), FALSE);

	return NMP_ISLOT_GET_IFACE(self)->ready(self);
}


//:~ End
