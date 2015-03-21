#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_cu.h"

void
nmp_cms_cu_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODCU, NULL);
	if (G_UNLIKELY(!mod))
	{
		nmp_error("<NmpModCu> alloc mod cu failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup((NmpAppMod*)mod);
	nmp_afx_mod_insert(BUSSLOT_POS_CU, mod);
}


//:~ End
