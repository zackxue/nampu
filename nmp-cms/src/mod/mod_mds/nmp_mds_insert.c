#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_mds.h"

void
jpf_cms_mds_insert( void )
{
	JpfAppMod *mod;

	mod = g_object_new(JPF_TYPE_MODMDS, NULL);
	if (G_UNLIKELY(!mod))
	{
		jpf_error("<JpfModMdu> alloc mod mdu failed!");
		FATAL_ERROR_EXIT;
	}

	jpf_app_mod_setup((JpfAppMod*)mod);
	jpf_afx_mod_insert(BUSSLOT_POS_MDS, mod);
}


//:~ End
