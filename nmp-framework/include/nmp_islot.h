/*
 * jxj platform - cms -  Slot Interface
*/

#ifndef __JPF_ISLOT_H__
#define __JPF_ISLOT_H__

#include <glib-object.h>
#include "nmp_data.h"

G_BEGIN_DECLS

/*
 * JpfISlot:
 * Interface of slot, must be implemented by all sort of slots.
 * Of cause it can not be instantiated directly.
*/
#define JPF_TYPE_ISLOT (jpf_islot_get_type())
#define JPF_IS_ISLOT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),  JPF_TYPE_ISLOT))
#define JPF_ISLOT(o) (G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_ISLOT, JpfISlot))
#define JPF_ISLOT_GET_IFACE(o) \
	(G_TYPE_INSTANCE_GET_INTERFACE((o), JPF_TYPE_ISLOT, JpfISlotInterface))

typedef struct _JpfISlot JpfISlot;	/* dummy */
typedef struct _JpfISlotInterface JpfISlotInterface;

struct _JpfISlotInterface
{
	GTypeInterface g_iface;

	gint (*init)(JpfISlot *self, GValue *parm);
	gint (*send)(JpfISlot *self, JpfData *data);
	gint (*recv)(JpfISlot *self, JpfData *data);
	gint (*link)(JpfISlot *self, JpfISlot *peer);
	gint (*unlink)(JpfISlot *self);
	gboolean (*ready)(JpfISlot *self);
};

/* interface init */
gint jpf_islot_init(JpfISlot *self, GValue *parm);

/* send data to peer, in other word, peer will receive our data. */
gint jpf_islot_send(JpfISlot *self, JpfData *data);

/* recv data from peer */
gint jpf_islot_recv(JpfISlot *self, JpfData *data);

/* link to peer, data can already be sent out if this operation
 * successfull, but may not receive untill peer linked us. */
gint jpf_islot_link(JpfISlot *self, JpfISlot *peer);

/* local unlink, after this operation data can not be sent out,
 * but still can be received. */
gint jpf_islot_unlink(JpfISlot *self);

/* ready ?? can receive data now ?? */
gboolean jpf_islot_ready(JpfISlot *self);

GType jpf_islot_get_type();

G_END_DECLS

#endif	//__JPF_ISLOT_H__
