#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_ams.h"

void
jpf_cms_ams_insert( void )
{
	JpfAppMod *mod;

	mod = g_object_new(JPF_TYPE_MODAMS, NULL);
	if (G_UNLIKELY(!mod))
	{
		jpf_error("<JpfModAlms> alloc mod ams failed!");
		FATAL_ERROR_EXIT;
	}

	jpf_app_mod_setup((JpfAppMod*)mod);
	jpf_afx_mod_insert(BUSSLOT_POS_AMS, mod);
}


//:~ End
