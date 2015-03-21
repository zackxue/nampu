#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_appobj.h"

G_DEFINE_TYPE(NmpAppObj, nmp_app_obj, NMP_TYPE_OBJECT);

static void
nmp_app_obj_init(NmpAppObj *self)
{
}


static void
nmp_app_obj_deliver_in_default(NmpAppObj *self, NmpSysMsg *msg)
{
    jpf_warning("<NmpAppObj> default deliver_in() called!");
    jpf_sysmsg_destroy(msg);
}


static void
nmp_app_obj_deliver_out_default(NmpAppObj *self, NmpSysMsg *msg)
{
    jpf_warning("<NmpAppObj> default deliver_out() called!");
    jpf_sysmsg_destroy(msg);
}


static gint
nmp_app_obj_hook_from_bus_default(NmpAppObj *self, NmpSysMsg *msg)
{
	return -1;
}


static void
nmp_app_obj_class_init(NmpAppObjClass *c_self)
{
    c_self->deliver_in = nmp_app_obj_deliver_in_default;
    c_self->deliver_out = nmp_app_obj_deliver_out_default;
    c_self->hook_from_bus = nmp_app_obj_hook_from_bus_default;
}


void
nmp_app_obj_deliver_out(NmpAppObj *self, NmpSysMsg *msg)
{
    G_ASSERT(NMP_IS_APPOBJ(self) && NMP_IS_SYSMSG(msg));

    jpf_sysmsg_detach_io(msg);
    NMP_APPOBJ_GET_CLASS(self)->deliver_out(
        self, msg
    );
}


void
nmp_app_obj_deliver_in(NmpAppObj *self, NmpSysMsg *msg)
{
	G_ASSERT(NMP_IS_APPOBJ(self) && NMP_IS_SYSMSG(msg));
/*	jpf_sysmsg_detach_io(msg); */

    NMP_APPOBJ_GET_CLASS(self)->deliver_in(
        self, msg
    );
}


gint
nmp_app_obj_hook_from_bus(NmpAppObj *self, NmpSysMsg *msg)
{
	G_ASSERT(self != NULL && msg != NULL);
	
	BUG_ON(msg->from_io);
	return NMP_APPOBJ_GET_CLASS(self)->hook_from_bus(
		self, msg
	);
}


//:~ End
