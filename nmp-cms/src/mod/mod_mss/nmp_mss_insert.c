#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_mss.h"

void
jpf_cms_mss_insert( void )
{
	JpfAppMod *mod;

	mod = g_object_new(JPF_TYPE_MODMSS, NULL);
	if (G_UNLIKELY(!mod))
	{
		jpf_error("<JpfModMss> alloc mod mss failed!");
		FATAL_ERROR_EXIT;
	}

	jpf_app_mod_setup((JpfAppMod*)mod);
	jpf_afx_mod_insert(BUSSLOT_POS_MSS, mod);
}


//:~ End
