/*
 * nmp_modio.h
 *
 * mod I/O object description.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_MOD_IO_H__
#define __NMP_MOD_IO_H__

#include "nmp_object.h"
#include "nmp_islot.h"
#include "nmp_sysmsg.h"

G_BEGIN_DECLS

#define NMP_TYPE_MODIO  (nmp_mod_io_get_type())
#define NMP_IS_MODIO(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODIO))
#define NMP_IS_MODIO_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODIO))
#define NMP_MODIO(o) (G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODIO, NmpModIO))
#define NMP_MODIO_CLASS(c) \
    (G_TYPE_CHECK_CLASS_CAST((c),NMP_TYPE_MODIO, NmpModIOClass))
#define NMP_MODIO_GET_CLASS(o) \
    (G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODIO, NmpModIOClass))

typedef struct _NmpModIO NmpModIO;
typedef struct _NmpModIOClass NmpModIOClass;

struct _NmpModIO
{
    JpfObject parent_object;

    gpointer owner;     /* parent mod */
    JpfISlot *i_peer;	/* always bus slot */
};


struct _NmpModIOClass	/* help to implement iSlot interface */
{
    JpfObjectClass parent_class;

    gint (*slot_init)(NmpModIO *self, GValue *parm);
    gint (*mod_snd)(NmpModIO *self, NmpSysMsg *msg);
    gint (*mod_rcv)(NmpModIO *self, NmpSysMsg *msg);
    gint (*connect)(NmpModIO *self, JpfISlot *i_peer);
    gint (*disconnect)(NmpModIO *self);
    gint (*slot_ok)(NmpModIO *self);    
};


GType nmp_mod_io_get_type( void );

G_BEGIN_DECLS

#endif  //__NMP_MOD_IO_H__
