#include "nmp_mod_policy.h"
#include "nmp_mods.h"


G_DEFINE_TYPE(NmpModPolicy, nmp_mod_policy, NMP_TYPE_APPMOD);

void nmp_mod_policy_register_msg_handler(NmpModPolicy *self);
void nmp_mod_policy_on_pp_event(gpointer priv, NmpMsg *msg);


static void
nmp_mod_policy_init(NmpModPolicy *self)
{
	self->gu_pool = NULL;
}


static gint
nmp_mod_policy_setup(NmpAppMod *super_self)
{
	NmpModPolicy *self = (NmpModPolicy*)super_self;

	nmp_app_mod_set_name(super_self, "MOD-POLICY");

	self->gu_pool = nmp_new_gu_pool();
	if (!self->gu_pool)
	{
		nmp_warning(
			"Create gu policy pool failed!"
		);
		FATAL_ERROR_EXIT;
	}

	nmp_gu_pool_set_handler(self->gu_pool, nmp_mod_policy_on_pp_event, self);
	nmp_mod_policy_register_msg_handler(self);

	return 0;
}


static void
nmp_mod_policy_class_init(NmpModPolicyClass *self_class)
{
	NmpAppModClass *super_class = (NmpAppModClass*)self_class;

	super_class->setup_mod	= nmp_mod_policy_setup;
}


void
nmp_mod_policy_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODPOLICY, NULL);
	if (G_UNLIKELY(!mod))
	{
		nmp_error("<NmpModBss> alloc mod bss failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup(mod);
	nmp_afx_mod_insert(BUSSLOT_POS_POLICY, mod);
}


//:~ End
