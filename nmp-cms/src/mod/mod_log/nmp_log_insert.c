#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_log.h"

void
nmp_cms_log_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODLOG, NULL);
	if (G_UNLIKELY(!mod))
	{
		nmp_error("<NmpModLog> alloc mod log failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup((NmpAppMod*)mod);
	nmp_afx_mod_insert(BUSSLOT_POS_LOG, mod);
}


//:~ End
