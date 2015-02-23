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

#define JPF_TYPE_MODIO  (jpf_mod_io_get_type())
#define JPF_IS_MODIO(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_MODIO))
#define JPF_IS_MODIO_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_MODIO))
#define JPF_MODIO(o) (G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_MODIO, JpfModIO))
#define JPF_MODIO_CLASS(c) \
    (G_TYPE_CHECK_CLASS_CAST((c),JPF_TYPE_MODIO, JpfModIOClass))
#define JPF_MODIO_GET_CLASS(o) \
    (G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_MODIO, JpfModIOClass))

typedef struct _JpfModIO JpfModIO;
typedef struct _JpfModIOClass JpfModIOClass;

struct _JpfModIO
{
    JpfObject parent_object;

    gpointer owner;     /* parent mod */
    JpfISlot *i_peer;	/* always bus slot */
};


struct _JpfModIOClass	/* help to implement iSlot interface */
{
    JpfObjectClass parent_class;

    gint (*slot_init)(JpfModIO *self, GValue *parm);
    gint (*mod_snd)(JpfModIO *self, JpfSysMsg *msg);
    gint (*mod_rcv)(JpfModIO *self, JpfSysMsg *msg);
    gint (*connect)(JpfModIO *self, JpfISlot *i_peer);
    gint (*disconnect)(JpfModIO *self);
    gint (*slot_ok)(JpfModIO *self);    
};


GType jpf_mod_io_get_type( void );

G_BEGIN_DECLS

#endif  //__NMP_MOD_IO_H__
