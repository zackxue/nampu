#include "nmp_debug.h"
#include "nmp_mods.h"
#include "nmp_mod_vod.h"

void
nmp_mod_vod_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODVOD, NULL);
	if (G_UNLIKELY(!mod))
	{
		nmp_error("<NmpModVod> alloc mod vod failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup((NmpAppMod*)mod);
	nmp_afx_mod_insert(BUSSLOT_POS_VOD, mod);
}


//:~ End
