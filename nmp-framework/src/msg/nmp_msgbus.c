#include <stdlib.h>
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_msgbus.h"
#include "nmp_busslot.h"

G_DEFINE_TYPE(JpfMsgBus, jpf_msg_bus, NMP_TYPE_OBJECT);

#define MAX_MSG_HOOKS		8

typedef struct __JpfBusHook JpfBusHook;
struct __JpfBusHook
{
	JpfBusSlotPos	who;
	JpfMsgHookFn	fun;
};

static JpfBusHook msg_hooks_array[MAX_MSG_HOOKS];
static guint msg_hooks_count = 0;

static gpointer
jpf_msg_bus_thread_worker(gpointer user_data);

static gint
jpf_msg_bus_setup_slots(JpfMsgBus *self);

static JpfBusHookRet
jpf_msg_bus_default_hook(JpfMsgBus *bus, NmpSysMsg *msg);

static void
jpf_msg_bus_init_zero(JpfMsgBus *self)
{
    self->msg_queue = NULL;
    self->msg_hook = jpf_msg_bus_default_hook;
    self->bus_thread = NULL;
    jpf_bus_slots_init(&self->bus_slots);
}


static void
jpf_msg_bus_init(JpfMsgBus *self)
{
    jpf_msg_bus_init_zero(self);

    self->msg_queue = g_async_queue_new();
    if (G_UNLIKELY(!self->msg_queue))
    {
        jpf_error("<JpfMsgBus> alloc async queue failed!");
        FATAL_ERROR_EXIT;
    }

	self->bus_bypass = FALSE;
    jpf_msg_bus_setup_slots(self);

    self->bus_thread = g_thread_create(
        jpf_msg_bus_thread_worker,
        self,
        FALSE,
        NULL
    );

    if (G_UNLIKELY(!self->bus_thread))
    {
        jpf_error("<JpfMsgBus> alloc thread obj failed!");
        FATAL_ERROR_EXIT;
    }
}


static void
jpf_msg_bus_dispose(GObject *object)
{
    exit(-1);
    G_OBJECT_CLASS(jpf_msg_bus_parent_class)->dispose(object);
}


static void
jpf_msg_bus_class_init(JpfMsgBusClass *c_self)
{
    GObjectClass *gobject_class;

    gobject_class = (GObjectClass*)c_self;
    gobject_class->dispose = jpf_msg_bus_dispose;
}


static void
jpf_msg_bus_destroy_slots(JpfMsgBus *self)
{

}


static gint
jpf_msg_bus_setup_slots(JpfMsgBus *self)
{
    JpfBusSlot *slot;
    GValue v = {0, };
    JpfBusSlotIdx idx = BUSSLOT_IDX_MIN + 1;

    for (; idx < BUSSLOT_IDX_MAX; ++idx)
    {
        slot = g_object_new(NMP_TYPE_BUSSLOT, NULL);
        if (G_UNLIKELY(!slot))
        {
            jpf_error("<JpfMsgBus> alloc bus slot failed!");
            jpf_msg_bus_destroy_slots(self);
            return -E_NOMEM;
        }

        v.data[0].v_pointer = self;
        v.data[1].v_int = 1 << idx;
        jpf_islot_init(NMP_ISLOT(slot), &v);
    }

    return 0;
}


gint
jpf_msg_bus_request_slot(JpfMsgBus *self, gpointer slot, JpfBusSlotPos i_slot)
{
    if (!NMP_IS_BUSSLOT(slot))
        return -E_INVAL;

    if (jpf_bus_slots_get(&self->bus_slots, i_slot))
        return -E_SLOTEXIST;

    jpf_bus_slots_set(&self->bus_slots, i_slot, slot);
    return 0;
}


gint
jpf_msg_bus_slot_link(JpfMsgBus *self, JpfBusSlotPos i_slot, NmpModIO *modio)
{
    gint err;
    JpfBusSlot *slot;

    g_return_val_if_fail(NMP_IS_MSGBUS(self), EINVAL);
    g_return_val_if_fail(NMP_IS_MODIO(modio), EINVAL);

    slot = jpf_bus_slots_get(&self->bus_slots, i_slot);
    if (G_UNLIKELY(!slot))
        return -E_SLOTNEXIST;

    err = jpf_islot_link(NMP_ISLOT(slot), NMP_ISLOT(modio));
    if (G_UNLIKELY(err))
    {
        jpf_warning("<JpfMsgBus> link failed, bus slot isn't ready!");
        return err;
    }

    err = jpf_islot_link(NMP_ISLOT(modio), NMP_ISLOT(slot));
    if (G_UNLIKELY(err))
    {
        jpf_warning("<JpfMsgBus> link failed, MOD-IO isn't ready!");
        jpf_islot_unlink(NMP_ISLOT(slot));
        return err;
    }

    return 0;
}


JpfBusMsgHook	/* implict */
jpf_msg_bus_set_hook(JpfMsgBus *self, JpfBusMsgHook hook)
{
    g_return_val_if_fail(NMP_IS_MSGBUS(self), NULL);

    JpfBusMsgHook old = self->msg_hook;
    self->msg_hook = hook;
    return old;
}


static void
jpf_msg_bus_destroy_msg(JpfMsgBus *self, NmpSysMsg *msg)
{
    g_return_if_fail(NMP_IS_MSGBUS(self));
    g_return_if_fail(NMP_IS_SYSMSG(msg));

    jpf_sysmsg_destroy(msg);
}


static __inline__ gint
jpf_msg_bus_deliver_unicast(JpfMsgBus *self, NmpSysMsg *msg)
{
	gpointer next_slot;

	if (!(next_slot = jpf_bus_slots_get(&self->bus_slots, msg->dst)))
	    return -E_SLOTNEXIST;

	return jpf_islot_send(NMP_ISLOT(next_slot), NMP_DATA(msg));			
}


static __inline__ gint
jpf_msg_bus_deliver_multicast(JpfMsgBus *self, NmpSysMsg *msg)
{
    JpfBusSlotIdx idx;
    NmpSysMsg *msg_cpy;
    gint msgs = 0, err;
    guint next = (guint)msg->dst;

	for (idx = BUSSLOT_IDX_MIN + 1; idx < BUSSLOT_IDX_MAX; ++idx)
	{
		if (next & (1 << idx))
			++msgs;
	}

	for (idx = BUSSLOT_IDX_MIN + 1; idx < BUSSLOT_IDX_MAX; ++idx)
	{
		if (next & (1 << idx))
		{
			if (--msgs > 0)
			{
				msg_cpy = jpf_sysmsg_copy_one(msg);
				if (G_LIKELY(msg_cpy))
				{
					MSG_SET_DSTPOS(msg_cpy, 1 << idx);
					err = jpf_msg_bus_deliver_unicast(self, msg_cpy);
					if (err)
					{
						jpf_warning(
							"Error while bus multicast delivering, deliver sysmsg failed."
						);
						/* orignal msg will be destroyed by caller */
						jpf_msg_bus_destroy_msg(self, msg_cpy);
						return err;
					}
				}
				else
				{
					jpf_warning(
						"Error while bus multicast delivering, copy sysmsg failed."
					);
					MSG_SET_DSTPOS(msg, 1 << idx);
					return jpf_msg_bus_deliver_unicast(self, msg);
				}
			}
			else
			{
				MSG_SET_DSTPOS(msg, 1 << idx);
				return jpf_msg_bus_deliver_unicast(self, msg);
			}
		}
	}

	BUG();
	return 0;
}


static gint
jpf_msg_bus_deliver(JpfMsgBus *self, NmpSysMsg *msg)
{
    guint next = (guint)msg->dst;

	if (!next || next >= BUSSLOT_POS_MAX)
		BUG();

	if (G_LIKELY(!(next & (next - 1))))
	{	/* fast path */
		return jpf_msg_bus_deliver_unicast(self, msg);
	}

	return jpf_msg_bus_deliver_multicast(self, msg);
}


gint
jpf_msg_bus_rcv_msg(JpfMsgBus *self, JpfBusSlotPos i_slot, NmpSysMsg *msg)
{
    g_return_val_if_fail(NMP_IS_MSGBUS(self), EINVAL);
    g_return_val_if_fail(NMP_IS_SYSMSG(msg), EINVAL);

	BUG_ON(!jpf_bus_slots_get(&self->bus_slots, i_slot));
	MSG_SET_SRCPOS(msg, i_slot);

	if (self->bus_bypass)
	{
        if (jpf_msg_bus_deliver(self, msg))
        {
            jpf_warning("<JpfMsgBus> deliver msg failed!");
            jpf_msg_bus_destroy_msg(self, msg);
        }
        return 0;		
	}

    g_async_queue_push(self->msg_queue, msg);
    return 0;
}


static gpointer
jpf_msg_bus_thread_worker(gpointer user_data)
{
    NmpSysMsg *msg;
    JpfMsgBus *self = (JpfMsgBus*)user_data;

    for (;;)
    {
        msg = g_async_queue_pop(self->msg_queue);
        if (G_LIKELY(self->msg_hook))
        {
            switch ((self->msg_hook)(self, msg))
            {
            case BHR_ACCEPT:
                break;

            case BHR_DROP:
                jpf_msg_bus_destroy_msg(self, msg);
                continue;

            case BHR_QUEUE:
                g_async_queue_push(self->msg_queue, msg);
                continue;

            default:
                break;
            }
        }

        if (jpf_msg_bus_deliver(self, msg))
        {
            jpf_warning("<JpfMsgBus> deliver msg failed!");
            jpf_msg_bus_destroy_msg(self, msg);
        }
    }

    jpf_error("<JpfMsgBus> worker thread exit loop!");
    return NULL;
}


void
jpf_msg_bus_set_bypass(JpfMsgBus *self)
{
	g_return_if_fail(NMP_IS_MSGBUS(self));
	self->bus_bypass = TRUE;
}


static JpfBusHookRet
jpf_msg_bus_default_hook(JpfMsgBus *bus, NmpSysMsg *msg)
{
	JpfBusHook *hook;
	gint ihook = 0;
	NmpSysMsg *msg_copy;

	for (; ihook < msg_hooks_count; ++ihook)
	{
		hook = &msg_hooks_array[ihook];
		if (hook->fun && !(*hook->fun)(msg))
		{
			msg_copy = jpf_sysmsg_copy_one(msg);
			BUG_ON(!msg_copy);
			MSG_SET_DSTPOS(msg_copy, hook->who);

		    if (jpf_msg_bus_deliver(bus, msg_copy))
		    {
		        jpf_warning("<JpfMsgBus> deliver msg failed!");
		        jpf_msg_bus_destroy_msg(bus, msg_copy);
		    }
		}
	}

	return BHR_ACCEPT;
}


gint
jpf_msg_bus_add_msg_hook(JpfBusSlotPos who, JpfMsgHookFn fun)
{
	JpfBusHook *hook;

	if (msg_hooks_count >= MAX_MSG_HOOKS)
		return -ENOMEM;

	hook = 	&msg_hooks_array[msg_hooks_count];
	hook->who = who;
	hook->fun = fun;
	++msg_hooks_count;

	return 0;
}


//:~ End
