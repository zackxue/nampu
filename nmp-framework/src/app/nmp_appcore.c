#include "nmp_share_errno.h"
#include "nmp_share_debug.h"
#include "nmp_appcore.h"
#include "nmp_appmod.h"


G_DEFINE_TYPE(NmpAppCore, nmp_app_core, NMP_TYPE_APPOBJ);

static NmpAppCore *app_core = NULL;

static void
nmp_app_core_init(NmpAppCore *self)
{
    self->p_bus = g_object_new(NMP_TYPE_MSGBUS, NULL);
    if (G_UNLIKELY(!self->p_bus))
    {
        nmp_error("<NmpAppCore> alloc bus failed!");
        FATAL_ERROR_EXIT;
    }
}


static void
nmp_app_core_class_init(NmpAppCoreClass *c_self)
{
}


static __inline__ gint
nmp_app_core_insert_mod(NmpAppCore *core, NmpAppMod *mod, NmpBusSlotPos pos)
{
    g_return_val_if_fail(NMP_IS_APPCORE(core), -E_INVAL);
    g_return_val_if_fail(NMP_IS_APPMOD(mod), -E_INVAL);

    return nmp_msg_bus_slot_link(core->p_bus, pos, mod->io_mod);
}


void
nmp_afx_core_init( void )
{
	BUG_ON(app_core);

	app_core = g_object_new(NMP_TYPE_APPCORE, NULL);
	if (G_UNLIKELY(!app_core))
	{
		nmp_error("<main> alloc app core failed!");
		FATAL_ERROR_EXIT;
	}
}


NmpAfxInst
nmp_afx_inst_init( void )
{
	NmpAppCore *inst;

	inst = g_object_new(NMP_TYPE_APPCORE, NULL);
	if (G_UNLIKELY(!inst))
	{
		nmp_warning("<main> alloc app core inst failed!");
	}

	return inst;
}


void
nmp_afx_mod_insert(NmpBusSlotPos slot, NmpAppMod *mod)
{
	BUG_ON(!app_core);

	if (G_UNLIKELY(nmp_app_core_insert_mod(app_core, mod,
		slot)))
	{
		nmp_error("<main> insert mod failed!");
		FATAL_ERROR_EXIT;
	}
}


void
nmp_afx_bus_bypass( void )
{
	BUG_ON(!app_core);

	nmp_msg_bus_set_bypass(app_core->p_bus);
}


gint
nmp_afx_inst_insert_mod(NmpAfxInst inst, NmpBusSlotPos slot,
	NmpAppMod *mod)
{
	gint err;
	NmpAppCore *_inst = (NmpAppCore*)inst;

	if (!NMP_IS_APPCORE(_inst))
		return -EINVAL;

	err = nmp_app_core_insert_mod(_inst, mod, slot);
	return err;
}


void
nmp_afx_inst_bus_bypass(NmpAfxInst inst)
{
	NmpAppCore *_inst = (NmpAppCore*)inst;

	if (!NMP_IS_APPCORE(_inst))
		return;

	nmp_msg_bus_set_bypass(_inst->p_bus);
}


//:~ End
