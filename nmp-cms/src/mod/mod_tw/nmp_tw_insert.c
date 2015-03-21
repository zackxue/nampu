#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_tw.h"

void
nmp_cms_tw_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODTW, NULL);
	if (G_UNLIKELY(!mod))
	{
		jpf_error("<JpfModTw> alloc mod tw failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup((NmpAppMod*)mod);
	jpf_afx_mod_insert(BUSSLOT_POS_TW, mod);
}


//:~ End
