#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_dbs.h"

void
jpf_cms_dbs_insert( void )
{
	JpfAppMod *mod;

	mod = g_object_new(JPF_TYPE_MODDBS, NULL);
	if (G_UNLIKELY(!mod))
	{
		jpf_error("<JpfModDbs> alloc mod dbs failed!");
		FATAL_ERROR_EXIT;
	}

	jpf_app_mod_setup((JpfAppMod*)mod);;
	jpf_afx_mod_insert(BUSSLOT_POS_DBS, mod);
}


//:~ End
