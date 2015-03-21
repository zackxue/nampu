#ifndef __NMP_OBJECT_H__
#define __NMP_OBJECT_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define NMP_TYPE_OBJECT	(jpf_object_get_type())
#define NMP_IS_OBJECT(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_OBJECT))
#define NMP_IS_OBJECT_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_OBJECT))
#define NMP_OBJECT(o)	\
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_OBJECT, JpfObject)
#define NMP_OBJECT_CLASS(c)	\
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_OBJECT,  JpfObjectClass))
#define NMP_OBJECT_GET_CLASS(o)	\
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_OBJECT, JpfObjectClass))

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
