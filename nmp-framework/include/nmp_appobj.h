#ifndef __NMP_APP_OBJ_H__
#define __NMP_APP_OBJ_H__

#include "nmp_object.h"
#include "nmp_sysmsg.h"

G_BEGIN_DECLS

#define NMP_TYPE_APPOBJ	(nmp_app_obj_get_type())
#define NMP_IS_APPOBJ(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_APPOBJ))
#define NMP_IS_APPOBJ_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_APPOBJ))
#define NMP_APPOBJ(o) (G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_APPOBJ, NmpAppObj))
#define NMP_APPOBJ_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_APPOBJ, NmpAppObjClass))
#define NMP_APPOBJ_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_APPOBJ, NmpAppObjClass))

typedef struct _NmpAppObj NmpAppObj;
typedef struct _NmpAppObjClass NmpAppObjClass;

struct _NmpAppObj
{
	NmpObject parent_object;

};


struct _NmpAppObjClass
{
	NmpObjectClass parent_class;

	void (*deliver_in)(NmpAppObj *self, NmpSysMsg *msg);
	void (*deliver_out)(NmpAppObj *self, NmpSysMsg *msg);

	gint (*hook_from_bus)(NmpAppObj *self, NmpSysMsg *msg);
};


GType nmp_app_obj_get_type( void );

void nmp_app_obj_deliver_out(NmpAppObj *self, NmpSysMsg *msg);
void nmp_app_obj_deliver_in(NmpAppObj *self, NmpSysMsg *msg);
gint nmp_app_obj_hook_from_bus(NmpAppObj *self, NmpSysMsg *msg);

G_END_DECLS

#endif	//__NMP_APP_OBJ_H__
