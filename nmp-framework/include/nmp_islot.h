/*
 * nmp platform - cms -  Slot Interface
*/

#ifndef __NMP_ISLOT_H__
#define __NMP_ISLOT_H__

#include <glib-object.h>
#include "nmp_data.h"

G_BEGIN_DECLS

/*
 * NmpISlot:
 * Interface of slot, must be implemented by all sort of slots.
 * Of cause it can not be instantiated directly.
*/
#define NMP_TYPE_ISLOT (nmp_islot_get_type())
#define NMP_IS_ISLOT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),  NMP_TYPE_ISLOT))
#define NMP_ISLOT(o) (G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_ISLOT, NmpISlot))
#define NMP_ISLOT_GET_IFACE(o) \
	(G_TYPE_INSTANCE_GET_INTERFACE((o), NMP_TYPE_ISLOT, NmpISlotInterface))

typedef struct _NmpISlot NmpISlot;	/* dummy */
typedef struct _NmpISlotInterface NmpISlotInterface;

struct _NmpISlotInterface
{
	GTypeInterface g_iface;

	gint (*init)(NmpISlot *self, GValue *parm);
	gint (*send)(NmpISlot *self, NmpData *data);
	gint (*recv)(NmpISlot *self, NmpData *data);
	gint (*link)(NmpISlot *self, NmpISlot *peer);
	gint (*unlink)(NmpISlot *self);
	gboolean (*ready)(NmpISlot *self);
};

/* interface init */
gint nmp_islot_init(NmpISlot *self, GValue *parm);

/* send data to peer, in other word, peer will receive our data. */
gint nmp_islot_send(NmpISlot *self, NmpData *data);

/* recv data from peer */
gint nmp_islot_recv(NmpISlot *self, NmpData *data);

/* link to peer, data can already be sent out if this operation
 * successfull, but may not receive untill peer linked us. */
gint nmp_islot_link(NmpISlot *self, NmpISlot *peer);

/* local unlink, after this operation data can not be sent out,
 * but still can be received. */
gint nmp_islot_unlink(NmpISlot *self);

/* ready ?? can receive data now ?? */
gboolean nmp_islot_ready(NmpISlot *self);

GType nmp_islot_get_type();

G_END_DECLS

#endif	//__NMP_ISLOT_H__
