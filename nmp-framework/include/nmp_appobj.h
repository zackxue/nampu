#ifndef __JPF_APP_OBJ_H__
#define __JPF_APP_OBJ_H__

#include "nmp_object.h"
#include "nmp_sysmsg.h"

G_BEGIN_DECLS

#define JPF_TYPE_APPOBJ	(jpf_app_obj_get_type())
#define JPF_IS_APPOBJ(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_APPOBJ))
#define JPF_IS_APPOBJ_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_APPOBJ))
#define JPF_APPOBJ(o) (G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_APPOBJ, JpfAppObj))
#define JPF_APPOBJ_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_APPOBJ, JpfAppObjClass))
#define JPF_APPOBJ_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_APPOBJ, JpfAppObjClass))

typedef struct _JpfAppObj JpfAppObj;
typedef struct _JpfAppObjClass JpfAppObjClass;

struct _JpfAppObj
{
	JpfObject parent_object;

};


struct _JpfAppObjClass
{
	JpfObjectClass parent_class;

	void (*deliver_in)(JpfAppObj *self, JpfSysMsg *msg);
	void (*deliver_out)(JpfAppObj *self, JpfSysMsg *msg);

	gint (*hook_from_bus)(JpfAppObj *self, JpfSysMsg *msg);
};


GType jpf_app_obj_get_type( void );

void jpf_app_obj_deliver_out(JpfAppObj *self, JpfSysMsg *msg);
void jpf_app_obj_deliver_in(JpfAppObj *self, JpfSysMsg *msg);
gint jpf_app_obj_hook_from_bus(JpfAppObj *self, JpfSysMsg *msg);

G_END_DECLS

#endif	//__JPF_APP_OBJ_H__
