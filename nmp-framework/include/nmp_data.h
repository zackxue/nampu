/*
 * nmp_data.h
 *
 * Base type of platform data, such as 'sysmsg'.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_DATA_H__
#define __NMP_DATA_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define NMP_TYPE_DATA   (jpf_data_get_type())
#define NMP_IS_DATA(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_DATA))
#define NMP_IS_DATA_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_DATA))
#define NMP_DATA(o) (G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_DATA, JpfData))
#define NMP_DATA_CLASS(c) (G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_DATA, JpfDataClass))
#define NMP_DADA_GET_CLASS(o) \
    (G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_DATA, JpfDataClass))

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

#endif  //__NMP_DATA_H__
