#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_appobj.h"

G_DEFINE_TYPE(JpfAppObj, jpf_app_obj, JPF_TYPE_OBJECT);

static void
jpf_app_obj_init(JpfAppObj *self)
{
}


static void
jpf_app_obj_deliver_in_default(JpfAppObj *self, JpfSysMsg *msg)
{
    jpf_warning("<JpfAppObj> default deliver_in() called!");
    jpf_sysmsg_destroy(msg);
}


static void
jpf_app_obj_deliver_out_default(JpfAppObj *self, JpfSysMsg *msg)
{
    jpf_warning("<JpfAppObj> default deliver_out() called!");
    jpf_sysmsg_destroy(msg);
}


static gint
jpf_app_obj_hook_from_bus_default(JpfAppObj *self, JpfSysMsg *msg)
{
	return -1;
}


static void
jpf_app_obj_class_init(JpfAppObjClass *c_self)
{
    c_self->deliver_in = jpf_app_obj_deliver_in_default;
    c_self->deliver_out = jpf_app_obj_deliver_out_default;
    c_self->hook_from_bus = jpf_app_obj_hook_from_bus_default;
}


void
jpf_app_obj_deliver_out(JpfAppObj *self, JpfSysMsg *msg)
{
    G_ASSERT(JPF_IS_APPOBJ(self) && JPF_IS_SYSMSG(msg));

    jpf_sysmsg_detach_io(msg);
    JPF_APPOBJ_GET_CLASS(self)->deliver_out(
        self, msg
    );
}


void
jpf_app_obj_deliver_in(JpfAppObj *self, JpfSysMsg *msg)
{
	G_ASSERT(JPF_IS_APPOBJ(self) && JPF_IS_SYSMSG(msg));
/*	jpf_sysmsg_detach_io(msg); */

    JPF_APPOBJ_GET_CLASS(self)->deliver_in(
        self, msg
    );
}


gint
jpf_app_obj_hook_from_bus(JpfAppObj *self, JpfSysMsg *msg)
{
	G_ASSERT(self != NULL && msg != NULL);
	
	BUG_ON(msg->from_io);
	return JPF_APPOBJ_GET_CLASS(self)->hook_from_bus(
		self, msg
	);
}


//:~ End
