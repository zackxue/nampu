/*
 * nmp_msgbus.h
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_MSG_BUS_H__
#define __NMP_MSG_BUS_H__

#include "nmp_object.h"
#include "nmp_sysmsg.h"
#include "nmp_modio.h"

G_BEGIN_DECLS

typedef enum        /* ret-value type of bus hooks */
{
    BHR_ACCEPT = 0,
    BHR_DROP,
    BHR_QUEUE
}NmpBusHookRet;

#define NMP_TYPE_MSGBUS (nmp_msg_bus_get_type())
#define NMP_IS_MSGBUS(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MSGBUS))
#define NMP_IS_MSGBUS_CLASS(c) \
    (G_TYPE_CHECK_CALSS_TYPE((c), NMP_TYPE_MSGBUS))
#define NMP_MSGBUS(o) (G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MSGBUS, NmpMsgBus))
#define NMP_MSGBUS_CLASS(c) \
    (G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MSGBUS, NmpMsgBusClass))
#define NMP_MSGBUS_GET_CLASS(o) \
    (G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MSGBUS, NmpMsgBusClass))

typedef struct _NmpMsgBus NmpMsgBus;
typedef struct _NmpMsgBusClass NmpMsgBusClass;
typedef gint (*NmpMsgHookFn)(NmpSysMsg *msg);
typedef NmpBusHookRet (*NmpBusMsgHook)(NmpMsgBus *bus, NmpSysMsg *msg);

struct _NmpMsgBus
{
    NmpObject parent_object;

    GAsyncQueue *msg_queue;
    NmpBusMsgHook msg_hook;
	gboolean bus_bypass;
    NmpBusSlots bus_slots;
    GThread *bus_thread;    /* msg dispatcher */
};


struct _NmpMsgBusClass
{
    NmpObjectClass parent_class;
};


gint nmp_msg_bus_request_slot(NmpMsgBus *self, 
    gpointer slot, NmpBusSlotPos i_slot);

gint nmp_msg_bus_slot_link(NmpMsgBus *self,
    NmpBusSlotPos i_slot, NmpModIO *modio);

void nmp_msg_bus_set_bypass(NmpMsgBus *self);

gint nmp_msg_bus_add_msg_hook(NmpBusSlotPos who, NmpMsgHookFn fun);
gint nmp_msg_bus_rcv_msg(NmpMsgBus *self, NmpBusSlotPos i_slot, NmpSysMsg *msg);

GType nmp_msg_bus_get_type( void );

G_END_DECLS

#endif  //__NMP_MSG_BUS_H__
