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

#define NMP_TYPE_DATA   (nmp_data_get_type())
#define NMP_IS_DATA(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_DATA))
#define NMP_IS_DATA_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_DATA))
#define NMP_DATA(o) (G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_DATA, NmpData))
#define NMP_DATA_CLASS(c) (G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_DATA, NmpDataClass))
#define NMP_DADA_GET_CLASS(o) \
    (G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_DATA, NmpDataClass))

typedef struct _NmpData NmpData;
typedef struct _NmpDataClass NmpDataClass;

struct _NmpData
{
    GObject g_object;

    gpointer data_addr;
    guint data_size;
    guint space_capacity;
};


struct _NmpDataClass
{
    GObjectClass parent_class;
};


GType nmp_data_get_type( void );


G_END_DECLS

#endif  //__NMP_DATA_H__
