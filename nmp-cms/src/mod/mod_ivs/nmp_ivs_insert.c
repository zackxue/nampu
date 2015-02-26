#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_ivs.h"

void
jpf_cms_ivs_insert( void )
{
	JpfAppMod *mod;

	mod = g_object_new(JPF_TYPE_MODIVS, NULL);
	if (G_UNLIKELY(!mod))
	{
		jpf_error("<JpfModIvs> alloc mod ivs failed!");
		FATAL_ERROR_EXIT;
	}

	jpf_app_mod_setup((JpfAppMod*)mod);
	jpf_afx_mod_insert(BUSSLOT_POS_IVS, mod);
}


//:~ End
