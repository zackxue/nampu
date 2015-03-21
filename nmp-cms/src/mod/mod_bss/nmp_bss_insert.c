#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_bss.h"

void
nmp_cms_bss_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODBSS, NULL);
	if (G_UNLIKELY(!mod))
	{
		nmp_error("<NmpModBss> alloc mod bss failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup((NmpAppMod*)mod);
	nmp_afx_mod_insert(BUSSLOT_POS_BSS, mod);
}


//:~ End
