#ifndef __NMP_BUSSLOT_H__
#define __NMP_BUSSLOT_H__

#include "nmp_object.h"
#include "nmp_islot.h"
#include "nmp_modio.h"
#include "nmp_msgbus.h"

G_BEGIN_DECLS

/*
 * JpfBusSlot:
 * This describes system bus slot, it implements "JpfISlot" interface. System
 * bus use JpfBusSlot object to communicate with module. one system bus can own
 * several this type of objects.
 * 
*/

#define NMP_TYPE_BUSSLOT (jpf_bus_slot_get_type())
#define NMP_IS_BUSSLOT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_BUSSLOT))
#define NMP_IS_BUSSLOT_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_BUSSLOT))
#define NMP_BUSSLOT(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_BUSSLOT, JpfBusSlot))
#define NMP_BUSSLOT_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_BUSSLOT, JpfBusSlotClass))
#define NMP_BUSSLOT_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_BUSSLOT, JpfBusSlotClass))

typedef struct _JpfBusSlot JpfBusSlot;
typedef struct _JpfBusSlotClass JpfBusSlotClass;

struct _JpfBusSlot
{
	JpfObject parent_object;

	JpfMsgBus *owner;
	JpfISlot *i_peer;

	gboolean bus_ready;
	JpfBusSlotPos ibus_slot;
};


struct _JpfBusSlotClass
{
	JpfObjectClass parent_class;

	gint (*slot_init)(JpfBusSlot *self, GValue *parm);
	gint (*bus_snd)(JpfBusSlot *self, NmpSysMsg *p_msg);
	gint (*bus_rcv)(JpfBusSlot *self, NmpSysMsg *p_msg);
	gint (*connect)(JpfBusSlot *self, JpfISlot *i_peer);
	gint (*disconnect)(JpfBusSlot *self);
	gint (*slot_ok)(JpfBusSlot *self);
};


GType jpf_bus_slot_get_type( void );

G_END_DECLS

#endif	//__NMP_BUSSLOT_H__
