#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_pu.h"

void
jpf_cms_pu_insert( void )
{
	JpfAppMod *mod;

	mod = g_object_new(JPF_TYPE_MODPU, NULL);
	if (G_UNLIKELY(!mod))
	{
		jpf_error("<JpfModPu> alloc mod pu failed!");
		FATAL_ERROR_EXIT;
	}

	jpf_app_mod_setup((JpfAppMod*)mod);
	jpf_afx_mod_insert(BUSSLOT_POS_PU, mod);
}


//:~ End
