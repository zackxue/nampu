#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_wdd.h"

void
nmp_cms_wdd_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODWDD, NULL);
	if (G_UNLIKELY(!mod))
	{
		jpf_error("<JpfModWdd> alloc mod wdd failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup((NmpAppMod*)mod);
	jpf_afx_mod_insert(BUSSLOT_POS_WDD, mod);
}


//:~ End
