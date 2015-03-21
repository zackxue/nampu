/*
 * nmp_sysmsg.h
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_SYS_MESSAGE__
#define __NMP_SYS_MESSAGE__

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
}NmpBusSlotIdx;

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
}NmpBusSlotPos;

#define SET_MSGID(id, v) ((id) = (v))
#define EQUAL_MSGID(id, msg_id) \
    ((id) == (msg_id))

#define MSGID_USE_INTEGER	/* it works. */

#ifdef MSGID_USE_INTEGER
typedef guint NmpMsgID;
#define INIT_MSGID(id) ((id) = 0)
#else
#error "error!!! type: 'NmpMsgID' haven't been implemented yet!"
#endif

typedef struct _NmpBusSlots NmpBusSlots;
struct _NmpBusSlots
{
    gpointer slots[BUSSLOT_IDX_MAX];
};

#define nmp_bus_slots_init(jbs) memset((jbs), 0, sizeof(NmpBusSlots));

static __inline__ gint
nmp_bus_slots_get_idx(NmpBusSlotPos pos)
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
nmp_bus_slots_get(NmpBusSlots *slots, NmpBusSlotPos pos)
{
    return slots->slots[nmp_bus_slots_get_idx(pos)];
}

static __inline__ void
nmp_bus_slots_set(NmpBusSlots *slots, NmpBusSlotPos pos, gpointer pt)
{
    slots->slots[nmp_bus_slots_get_idx(pos)] = pt;
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

#define NMP_TYPE_SYSMSG (nmp_sysmsg_get_type())
#define NMP_IS_SYSMSG(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_SYSMSG))
#define NMP_IS_SYSMSG_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_SYSMSG))
#define NMP_SYSMSG(o) (G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_SYSMSG, NmpSysMsg))
#define NMP_SYSMSG_CLASS(c) \
    (G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_SYSMSG, NmpSysMsgClass))
#define NMP_SYSMSG_GET_CLASS(o) \
    (G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_SYSMSG, NmpSysMsgClass))

typedef struct _NmpSysMsg NmpSysMsg;
typedef struct _NmpSysMsgClass NmpSysMsgClass;

typedef void (*NmpMsgPrivDes)(gpointer priv, gsize size);

typedef struct _NmpSysMsgPriv NmpSysMsgPriv;
struct _NmpSysMsgPriv
{
    gint                ref_count;
    gpointer            priv_data;
    gsize               priv_size;

    NmpMsgPrivDes       priv_destroy;
};

struct _NmpSysMsg
{
    NmpData             parent_object;

    struct {
        guint               seq;
    }packet_layer;      /* Low level layer */

    NmpMsgID            msg_id;
    guint               flags;

    NmpNetIO            *from_io;

    NmpBusSlotPos       dst;
    NmpBusSlotPos       src;
    NmpBusSlotPos       orig;

    NmpSysMsgPriv       *priv;			/* MSG data */

    gpointer			user_data;		/* User data*/
    gsize				user_size;
};


struct _NmpSysMsgClass
{
    NmpDataClass parent_class;
};

GType nmp_sysmsg_get_type( void );

NmpSysMsg *nmp_sysmsg_new(NmpMsgID msg_id, gpointer priv, gsize size,
    guint seq, NmpMsgPrivDes priv_destroy);

NmpSysMsg *nmp_sysmsg_new_2(NmpMsgID msg_id, gpointer priv, gsize size,
    guint seq);

void nmp_sysmsg_destroy(NmpSysMsg *msg);

void nmp_sysmsg_attach_io(NmpSysMsg *msg, NmpNetIO *io);
void nmp_sysmsg_detach_io(NmpSysMsg *msg);

void nmp_sysmsg_set_private(NmpSysMsg *msg, gpointer priv, gsize size,
    NmpMsgPrivDes priv_destroy);
gint nmp_sysmsg_set_private_2(NmpSysMsg *msg, gpointer priv, gsize size);

NmpSysMsg *nmp_sysmsg_copy_one(NmpSysMsg *msg);

gint nmp_sysmsg_set_userdata(NmpSysMsg *msg, gpointer data, gsize size);
gint nmp_sysmsg_get_userdata(NmpSysMsg *msg, gpointer *pdata, gsize *psize);

G_END_DECLS

#endif  //__NMP_SYS_MESSAGE__
