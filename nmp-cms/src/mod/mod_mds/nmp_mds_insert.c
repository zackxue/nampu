#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_mds.h"

void
nmp_cms_mds_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODMDS, NULL);
	if (G_UNLIKELY(!mod))
	{
		nmp_error("<NmpModMdu> alloc mod mdu failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup((NmpAppMod*)mod);
	nmp_afx_mod_insert(BUSSLOT_POS_MDS, mod);
}


//:~ End
