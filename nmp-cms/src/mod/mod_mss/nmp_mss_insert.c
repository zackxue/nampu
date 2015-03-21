#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_mss.h"

void
nmp_cms_mss_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODMSS, NULL);
	if (G_UNLIKELY(!mod))
	{
		nmp_error("<NmpModMss> alloc mod mss failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup((NmpAppMod*)mod);
	nmp_afx_mod_insert(BUSSLOT_POS_MSS, mod);
}


//:~ End
