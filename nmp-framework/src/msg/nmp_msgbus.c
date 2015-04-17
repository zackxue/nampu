#include <stdlib.h>
#include "nmp_share_errno.h"
#include "nmp_share_debug.h"
#include "nmp_msgbus.h"
#include "nmp_busslot.h"

G_DEFINE_TYPE(NmpMsgBus, nmp_msg_bus, NMP_TYPE_OBJECT);

#define MAX_MSG_HOOKS		8

typedef struct __NmpBusHook NmpBusHook;
struct __NmpBusHook
{
	NmpBusSlotPos	who;
	NmpMsgHookFn	fun;
};

static NmpBusHook msg_hooks_array[MAX_MSG_HOOKS];
static guint msg_hooks_count = 0;

static gpointer
nmp_msg_bus_thread_worker(gpointer user_data);

static gint
nmp_msg_bus_setup_slots(NmpMsgBus *self);

static NmpBusHookRet
nmp_msg_bus_default_hook(NmpMsgBus *bus, NmpSysMsg *msg);

static void
nmp_msg_bus_init_zero(NmpMsgBus *self)
{
    self->msg_queue = NULL;
    self->msg_hook = nmp_msg_bus_default_hook;
    self->bus_thread = NULL;
    nmp_bus_slots_init(&self->bus_slots);
}


static void
nmp_msg_bus_init(NmpMsgBus *self)
{
    nmp_msg_bus_init_zero(self);

    self->msg_queue = g_async_queue_new();
    if (G_UNLIKELY(!self->msg_queue))
    {
        nmp_error("<NmpMsgBus> alloc async queue failed!");
        FATAL_ERROR_EXIT;
    }

	self->bus_bypass = FALSE;
    nmp_msg_bus_setup_slots(self);

    self->bus_thread = g_thread_create(
        nmp_msg_bus_thread_worker,
        self,
        FALSE,
        NULL
    );

    if (G_UNLIKELY(!self->bus_thread))
    {
        nmp_error("<NmpMsgBus> alloc thread obj failed!");
        FATAL_ERROR_EXIT;
    }
}


static void
nmp_msg_bus_dispose(GObject *object)
{
    exit(-1);
    G_OBJECT_CLASS(nmp_msg_bus_parent_class)->dispose(object);
}


static void
nmp_msg_bus_class_init(NmpMsgBusClass *c_self)
{
    GObjectClass *gobject_class;

    gobject_class = (GObjectClass*)c_self;
    gobject_class->dispose = nmp_msg_bus_dispose;
}


static void
nmp_msg_bus_destroy_slots(NmpMsgBus *self)
{

}


static gint
nmp_msg_bus_setup_slots(NmpMsgBus *self)
{
    NmpBusSlot *slot;
    GValue v = {0, };
    NmpBusSlotIdx idx = BUSSLOT_IDX_MIN + 1;

    for (; idx < BUSSLOT_IDX_MAX; ++idx)
    {
        slot = g_object_new(NMP_TYPE_BUSSLOT, NULL);
        if (G_UNLIKELY(!slot))
        {
            nmp_error("<NmpMsgBus> alloc bus slot failed!");
            nmp_msg_bus_destroy_slots(self);
            return -E_NOMEM;
        }

        v.data[0].v_pointer = self;
        v.data[1].v_int = 1 << idx;
        nmp_islot_init(NMP_ISLOT(slot), &v);
    }

    return 0;
}


gint
nmp_msg_bus_request_slot(NmpMsgBus *self, gpointer slot, NmpBusSlotPos i_slot)
{
    if (!NMP_IS_BUSSLOT(slot))
        return -E_INVAL;

    if (nmp_bus_slots_get(&self->bus_slots, i_slot))
        return -E_SLOTEXIST;

    nmp_bus_slots_set(&self->bus_slots, i_slot, slot);
    return 0;
}


gint
nmp_msg_bus_slot_link(NmpMsgBus *self, NmpBusSlotPos i_slot, NmpModIO *modio)
{
    gint err;
    NmpBusSlot *slot;

    g_return_val_if_fail(NMP_IS_MSGBUS(self), EINVAL);
    g_return_val_if_fail(NMP_IS_MODIO(modio), EINVAL);

    slot = nmp_bus_slots_get(&self->bus_slots, i_slot);
    if (G_UNLIKELY(!slot))
        return -E_SLOTNEXIST;

    err = nmp_islot_link(NMP_ISLOT(slot), NMP_ISLOT(modio));
    if (G_UNLIKELY(err))
    {
        nmp_warning("<NmpMsgBus> link failed, bus slot isn't ready!");
        return err;
    }

    err = nmp_islot_link(NMP_ISLOT(modio), NMP_ISLOT(slot));
    if (G_UNLIKELY(err))
    {
        nmp_warning("<NmpMsgBus> link failed, MOD-IO isn't ready!");
        nmp_islot_unlink(NMP_ISLOT(slot));
        return err;
    }

    return 0;
}


NmpBusMsgHook	/* implict */
nmp_msg_bus_set_hook(NmpMsgBus *self, NmpBusMsgHook hook)
{
    g_return_val_if_fail(NMP_IS_MSGBUS(self), NULL);

    NmpBusMsgHook old = self->msg_hook;
    self->msg_hook = hook;
    return old;
}


static void
nmp_msg_bus_destroy_msg(NmpMsgBus *self, NmpSysMsg *msg)
{
    g_return_if_fail(NMP_IS_MSGBUS(self));
    g_return_if_fail(NMP_IS_SYSMSG(msg));

    nmp_sysmsg_destroy(msg);
}


static __inline__ gint
nmp_msg_bus_deliver_unicast(NmpMsgBus *self, NmpSysMsg *msg)
{
	gpointer next_slot;

	if (!(next_slot = nmp_bus_slots_get(&self->bus_slots, msg->dst)))
	    return -E_SLOTNEXIST;

	return nmp_islot_send(NMP_ISLOT(next_slot), NMP_DATA(msg));			
}


static __inline__ gint
nmp_msg_bus_deliver_multicast(NmpMsgBus *self, NmpSysMsg *msg)
{
    NmpBusSlotIdx idx;
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
				msg_cpy = nmp_sysmsg_copy_one(msg);
				if (G_LIKELY(msg_cpy))
				{
					MSG_SET_DSTPOS(msg_cpy, 1 << idx);
					err = nmp_msg_bus_deliver_unicast(self, msg_cpy);
					if (err)
					{
						nmp_warning(
							"Error while bus multicast delivering, deliver sysmsg failed."
						);
						/* orignal msg will be destroyed by caller */
						nmp_msg_bus_destroy_msg(self, msg_cpy);
						return err;
					}
				}
				else
				{
					nmp_warning(
						"Error while bus multicast delivering, copy sysmsg failed."
					);
					MSG_SET_DSTPOS(msg, 1 << idx);
					return nmp_msg_bus_deliver_unicast(self, msg);
				}
			}
			else
			{
				MSG_SET_DSTPOS(msg, 1 << idx);
				return nmp_msg_bus_deliver_unicast(self, msg);
			}
		}
	}

	BUG();
	return 0;
}


static gint
nmp_msg_bus_deliver(NmpMsgBus *self, NmpSysMsg *msg)
{
    guint next = (guint)msg->dst;

	if (!next || next >= BUSSLOT_POS_MAX)
		BUG();

	if (G_LIKELY(!(next & (next - 1))))
	{	/* fast path */
		return nmp_msg_bus_deliver_unicast(self, msg);
	}

	return nmp_msg_bus_deliver_multicast(self, msg);
}


gint
nmp_msg_bus_rcv_msg(NmpMsgBus *self, NmpBusSlotPos i_slot, NmpSysMsg *msg)
{
    g_return_val_if_fail(NMP_IS_MSGBUS(self), EINVAL);
    g_return_val_if_fail(NMP_IS_SYSMSG(msg), EINVAL);

	BUG_ON(!nmp_bus_slots_get(&self->bus_slots, i_slot));
	MSG_SET_SRCPOS(msg, i_slot);

	if (self->bus_bypass)
	{
        if (nmp_msg_bus_deliver(self, msg))
        {
            nmp_warning("<NmpMsgBus> deliver msg failed!");
            nmp_msg_bus_destroy_msg(self, msg);
        }
        return 0;		
	}

    g_async_queue_push(self->msg_queue, msg);
    return 0;
}


static gpointer
nmp_msg_bus_thread_worker(gpointer user_data)
{
    NmpSysMsg *msg;
    NmpMsgBus *self = (NmpMsgBus*)user_data;

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
                nmp_msg_bus_destroy_msg(self, msg);
                continue;

            case BHR_QUEUE:
                g_async_queue_push(self->msg_queue, msg);
                continue;

            default:
                break;
            }
        }

        if (nmp_msg_bus_deliver(self, msg))
        {
            nmp_warning("<NmpMsgBus> deliver msg failed!");
            nmp_msg_bus_destroy_msg(self, msg);
        }
    }

    nmp_error("<NmpMsgBus> worker thread exit loop!");
    return NULL;
}


void
nmp_msg_bus_set_bypass(NmpMsgBus *self)
{
	g_return_if_fail(NMP_IS_MSGBUS(self));
	self->bus_bypass = TRUE;
}


static NmpBusHookRet
nmp_msg_bus_default_hook(NmpMsgBus *bus, NmpSysMsg *msg)
{
	NmpBusHook *hook;
	gint ihook = 0;
	NmpSysMsg *msg_copy;

	for (; ihook < msg_hooks_count; ++ihook)
	{
		hook = &msg_hooks_array[ihook];
		if (hook->fun && !(*hook->fun)(msg))
		{
			msg_copy = nmp_sysmsg_copy_one(msg);
			BUG_ON(!msg_copy);
			MSG_SET_DSTPOS(msg_copy, hook->who);

		    if (nmp_msg_bus_deliver(bus, msg_copy))
		    {
		        nmp_warning("<NmpMsgBus> deliver msg failed!");
		        nmp_msg_bus_destroy_msg(bus, msg_copy);
		    }
		}
	}

	return BHR_ACCEPT;
}


gint
nmp_msg_bus_add_msg_hook(NmpBusSlotPos who, NmpMsgHookFn fun)
{
	NmpBusHook *hook;

	if (msg_hooks_count >= MAX_MSG_HOOKS)
		return -ENOMEM;

	hook = 	&msg_hooks_array[msg_hooks_count];
	hook->who = who;
	hook->fun = fun;
	++msg_hooks_count;

	return 0;
}


//:~ End
