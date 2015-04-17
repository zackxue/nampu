#include "nmp_share_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_ams.h"

void
nmp_cms_ams_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODAMS, NULL);
	if (G_UNLIKELY(!mod))
	{
		nmp_error("<NmpModAlms> alloc mod ams failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup((NmpAppMod*)mod);
	nmp_afx_mod_insert(BUSSLOT_POS_AMS, mod);
}


//:~ End
