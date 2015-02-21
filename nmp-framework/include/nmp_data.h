/*
 * nmp_data.h
 *
 * Base type of platform data, such as 'sysmsg'.
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
*/

#ifndef __JPF_DATA_H__
#define __JPF_DATA_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define JPF_TYPE_DATA   (jpf_data_get_type())
#define JPF_IS_DATA(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_DATA))
#define JPF_IS_DATA_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_DATA))
#define JPF_DATA(o) (G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_DATA, JpfData))
#define JPF_DATA_CLASS(c) (G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_DATA, JpfDataClass))
#define JPF_DADA_GET_CLASS(o) \
    (G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_DATA, JpfDataClass))

typedef struct _JpfData JpfData;
typedef struct _JpfDataClass JpfDataClass;

struct _JpfData
{
    GObject g_object;

    gpointer data_addr;
    guint data_size;
    guint space_capacity;
};


struct _JpfDataClass
{
    GObjectClass parent_class;
};


GType jpf_data_get_type( void );


G_END_DECLS

#endif  //__JPF_DATA_H__
