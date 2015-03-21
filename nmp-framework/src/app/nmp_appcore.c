#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_appmod.h"


G_DEFINE_TYPE(JpfAppCore, nmp_app_core, NMP_TYPE_APPOBJ);

static JpfAppCore *app_core = NULL;

static void
nmp_app_core_init(JpfAppCore *self)
{
    self->p_bus = g_object_new(NMP_TYPE_MSGBUS, NULL);
    if (G_UNLIKELY(!self->p_bus))
    {
        jpf_error("<JpfAppCore> alloc bus failed!");
        FATAL_ERROR_EXIT;
    }
}


static void
nmp_app_core_class_init(JpfAppCoreClass *c_self)
{
}


static __inline__ gint
nmp_app_core_insert_mod(JpfAppCore *core, NmpAppMod *mod, JpfBusSlotPos pos)
{
    g_return_val_if_fail(NMP_IS_APPCORE(core), -E_INVAL);
    g_return_val_if_fail(NMP_IS_APPMOD(mod), -E_INVAL);

    return jpf_msg_bus_slot_link(core->p_bus, pos, mod->io_mod);
}


void
jpf_afx_core_init( void )
{
	BUG_ON(app_core);

	app_core = g_object_new(NMP_TYPE_APPCORE, NULL);
	if (G_UNLIKELY(!app_core))
	{
		jpf_error("<main> alloc app core failed!");
		FATAL_ERROR_EXIT;
	}
}


JpfAfxInst
jpf_afx_inst_init( void )
{
	JpfAppCore *inst;

	inst = g_object_new(NMP_TYPE_APPCORE, NULL);
	if (G_UNLIKELY(!inst))
	{
		jpf_warning("<main> alloc app core inst failed!");
	}

	return inst;
}


void
jpf_afx_mod_insert(JpfBusSlotPos slot, NmpAppMod *mod)
{
	BUG_ON(!app_core);

	if (G_UNLIKELY(nmp_app_core_insert_mod(app_core, mod,
		slot)))
	{
		jpf_error("<main> insert mod failed!");
		FATAL_ERROR_EXIT;
	}
}


void
jpf_afx_bus_bypass( void )
{
	BUG_ON(!app_core);

	jpf_msg_bus_set_bypass(app_core->p_bus);
}


gint
jpf_afx_inst_insert_mod(JpfAfxInst inst, JpfBusSlotPos slot,
	NmpAppMod *mod)
{
	gint err;
	JpfAppCore *_inst = (JpfAppCore*)inst;

	if (!NMP_IS_APPCORE(_inst))
		return -EINVAL;

	err = nmp_app_core_insert_mod(_inst, mod, slot);
	return err;
}


void
jpf_afx_inst_bus_bypass(JpfAfxInst inst)
{
	JpfAppCore *_inst = (JpfAppCore*)inst;

	if (!NMP_IS_APPCORE(_inst))
		return;

	jpf_msg_bus_set_bypass(_inst->p_bus);
}


//:~ End
