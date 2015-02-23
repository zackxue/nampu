#ifndef __NMP_OBJECT_H__
#define __NMP_OBJECT_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define JPF_TYPE_OBJECT	(jpf_object_get_type())
#define JPF_IS_OBJECT(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_OBJECT))
#define JPF_IS_OBJECT_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_OBJECT))
#define JPF_OBJECT(o)	\
	(G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_OBJECT, JpfObject)
#define JPF_OBJECT_CLASS(c)	\
	(G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_OBJECT,  JpfObjectClass))
#define JPF_OBJECT_GET_CLASS(o)	\
	(G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_OBJECT, JpfObjectClass))

/*
 * JpfObject, base type of all structral Jpf-types.
*/

typedef struct _JpfObject JpfObject;
typedef struct _JpfObjectClass JpfObjectClass;


struct _JpfObject
{
	GObject g_object;

};


struct _JpfObjectClass
{
	GObjectClass parent_class;

};


GType jpf_object_get_type( void );

G_END_DECLS

#endif	//__NMP_OBJECT_H__
