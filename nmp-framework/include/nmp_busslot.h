#ifndef __NMP_BUSSLOT_H__
#define __NMP_BUSSLOT_H__

#include "nmp_object.h"
#include "nmp_islot.h"
#include "nmp_modio.h"
#include "nmp_msgbus.h"

G_BEGIN_DECLS

/*
 * NmpBusSlot:
 * This describes system bus slot, it implements "NmpISlot" interface. System
 * bus use NmpBusSlot object to communicate with module. one system bus can own
 * several this type of objects.
 * 
*/

#define NMP_TYPE_BUSSLOT (nmp_bus_slot_get_type())
#define NMP_IS_BUSSLOT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_BUSSLOT))
#define NMP_IS_BUSSLOT_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_BUSSLOT))
#define NMP_BUSSLOT(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_BUSSLOT, NmpBusSlot))
#define NMP_BUSSLOT_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_BUSSLOT, NmpBusSlotClass))
#define NMP_BUSSLOT_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_BUSSLOT, NmpBusSlotClass))

typedef struct _NmpBusSlot NmpBusSlot;
typedef struct _NmpBusSlotClass NmpBusSlotClass;

struct _NmpBusSlot
{
	NmpObject parent_object;

	NmpMsgBus *owner;
	NmpISlot *i_peer;

	gboolean bus_ready;
	NmpBusSlotPos ibus_slot;
};


struct _NmpBusSlotClass
{
	NmpObjectClass parent_class;

	gint (*slot_init)(NmpBusSlot *self, GValue *parm);
	gint (*bus_snd)(NmpBusSlot *self, NmpSysMsg *p_msg);
	gint (*bus_rcv)(NmpBusSlot *self, NmpSysMsg *p_msg);
	gint (*connect)(NmpBusSlot *self, NmpISlot *i_peer);
	gint (*disconnect)(NmpBusSlot *self);
	gint (*slot_ok)(NmpBusSlot *self);
};


GType nmp_bus_slot_get_type( void );

G_END_DECLS

#endif	//__NMP_BUSSLOT_H__
