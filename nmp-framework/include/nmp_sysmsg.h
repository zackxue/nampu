/*
 * nmp_sysmsg.h
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __JPF_SYS_MESSAGE__
#define __JPF_SYS_MESSAGE__

#include <string.h>
#include "nmp_data.h"
#include "nmp_net.h"
#include "nmp_debug.h"

G_BEGIN_DECLS

#define FLG_SYSMSG_FORWARD      0x01
#define FLG_SYSMSG_RESPONSE     0x02
#define FLG_SYSMSG_GUEST        0x04    /* From other mods */


typedef enum
{
    BUSSLOT_IDX_MIN = -1,
    BUSSLOT_IDX_0 ,
    BUSSLOT_IDX_1 ,
    BUSSLOT_IDX_2 ,
    BUSSLOT_IDX_3 ,
    BUSSLOT_IDX_4 ,
    BUSSLOT_IDX_5 ,
    BUSSLOT_IDX_6 ,
    BUSSLOT_IDX_7 ,
    BUSSLOT_IDX_8 ,
    BUSSLOT_IDX_9 ,
    BUSSLOT_IDX_10 ,
    BUSSLOT_IDX_11 ,
    BUSSLOT_IDX_12 ,
    BUSSLOT_IDX_13 ,
    BUSSLOT_IDX_14 ,
    BUSSLOT_IDX_15 ,
    /* ... */
    /* Add other slot pos here */
    BUSSLOT_IDX_MAX
}JpfBusSlotIdx;

typedef enum
{
    BUSSLOT_POS_0 = 1 << BUSSLOT_IDX_0,
    BUSSLOT_POS_1 = 1 << BUSSLOT_IDX_1,
    BUSSLOT_POS_2 = 1 << BUSSLOT_IDX_2,
    BUSSLOT_POS_3 = 1 << BUSSLOT_IDX_3,
    BUSSLOT_POS_4 = 1 << BUSSLOT_IDX_4,
    BUSSLOT_POS_5 = 1 << BUSSLOT_IDX_5,
    BUSSLOT_POS_6 = 1 << BUSSLOT_IDX_6,
    BUSSLOT_POS_7 = 1 << BUSSLOT_IDX_7,
    BUSSLOT_POS_8 = 1 << BUSSLOT_IDX_8,
    BUSSLOT_POS_9 = 1 << BUSSLOT_IDX_9,
    BUSSLOT_POS_10 = 1 << BUSSLOT_IDX_10,
    BUSSLOT_POS_11 = 1 << BUSSLOT_IDX_11,
    BUSSLOT_POS_12 = 1 << BUSSLOT_IDX_12,
    BUSSLOT_POS_13 = 1 << BUSSLOT_IDX_13,
    BUSSLOT_POS_14 = 1 << BUSSLOT_IDX_14,
    BUSSLOT_POS_15 = 1 << BUSSLOT_IDX_15,
    /* ... */
    /* Add other slot pos here */
    BUSSLOT_POS_MAX = 1 << BUSSLOT_IDX_MAX
}JpfBusSlotPos;

#define SET_MSGID(id, v) ((id) = (v))
#define EQUAL_MSGID(id, msg_id) \
    ((id) == (msg_id))

#define MSGID_USE_INTEGER	/* it works. */

#ifdef MSGID_USE_INTEGER
typedef guint JpfMsgID;
#define INIT_MSGID(id) ((id) = 0)
#else
#error "error!!! type: 'JpfMsgID' haven't been implemented yet!"
#endif

typedef struct _JpfBusSlots JpfBusSlots;
struct _JpfBusSlots
{
    gpointer slots[BUSSLOT_IDX_MAX];
};

#define jpf_bus_slots_init(jbs) memset((jbs), 0, sizeof(JpfBusSlots));

static __inline__ gint
jpf_bus_slots_get_idx(JpfBusSlotPos pos)
{
    gint index = 0;
    guint _pos = (guint)pos;

    if (!_pos || _pos >= BUSSLOT_POS_MAX)
        BUG();

    if (_pos & (_pos - 1))
        BUG();

    while (!(_pos & 0x1))
    {
        ++index;
        _pos >>= 1;
    }

    return index;   
}

static __inline__ gpointer
jpf_bus_slots_get(JpfBusSlots *slots, JpfBusSlotPos pos)
{
    return slots->slots[jpf_bus_slots_get_idx(pos)];
}

static __inline__ void
jpf_bus_slots_set(JpfBusSlots *slots, JpfBusSlotPos pos, gpointer pt)
{
    slots->slots[jpf_bus_slots_get_idx(pos)] = pt;
}

#define INVALID_BUS_SLOT(pos) \
    ((pos) >= BUSSLOT_POS_MAX || (pos) <= BUSSLOT_POS_BEGIN)

#define MSG_EQUAL(sysmsg, msgid) \
    EQUAL_MSGID((sysmsg)->msg_id, (msgid))

#define MSG_FORWARD(sysmsg) \
    ((sysmsg)->flags & FLG_SYSMSG_FORWARD)

#define MSG_RESPONSE(sysmsg) \
    ((sysmsg)->flags & FLG_SYSMSG_RESPONSE)

#define MSG_GUEST(sysmsg) \
    ((sysmsg)->flags & FLG_SYSMSG_GUEST)

#define MSG_SETID(sysmsg, msgid) \
    SET_MSGID((sysmsg)->msg_id, (msgid))

#define MSG_GETID(sysmsg) \
    ((sysmsg)->msg_id)

#define MSG_SEQ(sysmsg) \
    ((sysmsg)->packet_layer.seq)

#define MSG_IO(sysmsg) \
    ((sysmsg)->from_io)

#define MSG_FROM_BUS(msg) \
do {\
    (msg)->flags &= ~FLG_SYSMSG_FORWARD; \
    (msg)->flags |= FLG_SYSMSG_GUEST; \
}while (0)

#define MSG_GET_DATA(msg)           ((msg)->priv->priv_data)
#define MSG_DATA_SIZE(msg)          ((msg)->priv->priv_size)
#define MSG_SET_DSTPOS(msg, pos)    ((msg)->dst = (pos))
#define MSG_SET_SRCPOS(msg, pos)    ((msg)->src = (pos))
#define MSG_SET_ORIGPOS(msg, pos)   ((msg)->orig = (pos))
#define MSG_GET_SRCPOS(msg)    		((msg)->src)
#define MSG_GET_ORIGPOS(msg)   		((msg)->orig)
#define MSG_SET_RESPONSE(msg)       ((msg)->flags |= FLG_SYSMSG_RESPONSE)

#define JPF_TYPE_SYSMSG (jpf_sysmsg_get_type())
#define JPF_IS_SYSMSG(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_SYSMSG))
#define JPF_IS_SYSMSG_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_SYSMSG))
#define JPF_SYSMSG(o) (G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_SYSMSG, JpfSysMsg))
#define JPF_SYSMSG_CLASS(c) \
    (G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_SYSMSG, JpfSysMsgClass))
#define JPF_SYSMSG_GET_CLASS(o) \
    (G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_SYSMSG, JpfSysMsgClass))

typedef struct _JpfSysMsg JpfSysMsg;
typedef struct _JpfSysMsgClass JpfSysMsgClass;

typedef void (*JpfMsgPrivDes)(gpointer priv, gsize size);

typedef struct _JpfSysMsgPriv JpfSysMsgPriv;
struct _JpfSysMsgPriv
{
    gint                ref_count;
    gpointer            priv_data;
    gsize               priv_size;

    JpfMsgPrivDes       priv_destroy;
};

struct _JpfSysMsg
{
    JpfData             parent_object;

    struct {
        guint               seq;
    }packet_layer;      /* Low level layer */

    JpfMsgID            msg_id;
    guint               flags;

    JpfNetIO            *from_io;

    JpfBusSlotPos       dst;
    JpfBusSlotPos       src;
    JpfBusSlotPos       orig;

    JpfSysMsgPriv       *priv;			/* MSG data */

    gpointer			user_data;		/* User data*/
    gsize				user_size;
};


struct _JpfSysMsgClass
{
    JpfDataClass parent_class;
};

GType jpf_sysmsg_get_type( void );

JpfSysMsg *jpf_sysmsg_new(JpfMsgID msg_id, gpointer priv, gsize size,
    guint seq, JpfMsgPrivDes priv_destroy);

JpfSysMsg *jpf_sysmsg_new_2(JpfMsgID msg_id, gpointer priv, gsize size,
    guint seq);

void jpf_sysmsg_destroy(JpfSysMsg *msg);

void jpf_sysmsg_attach_io(JpfSysMsg *msg, JpfNetIO *io);
void jpf_sysmsg_detach_io(JpfSysMsg *msg);

void jpf_sysmsg_set_private(JpfSysMsg *msg, gpointer priv, gsize size,
    JpfMsgPrivDes priv_destroy);
gint jpf_sysmsg_set_private_2(JpfSysMsg *msg, gpointer priv, gsize size);

JpfSysMsg *jpf_sysmsg_copy_one(JpfSysMsg *msg);

gint jpf_sysmsg_set_userdata(JpfSysMsg *msg, gpointer data, gsize size);
gint jpf_sysmsg_get_userdata(JpfSysMsg *msg, gpointer *pdata, gsize *psize);

G_END_DECLS

#endif  //__JPF_SYS_MESSAGE__
