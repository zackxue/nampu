#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_dbs.h"

void
nmp_cms_dbs_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODDBS, NULL);
	if (G_UNLIKELY(!mod))
	{
		nmp_error("<NmpModDbs> alloc mod dbs failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup((NmpAppMod*)mod);;
	nmp_afx_mod_insert(BUSSLOT_POS_DBS, mod);
}


//:~ End
