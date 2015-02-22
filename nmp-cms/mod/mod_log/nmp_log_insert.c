#include "nmp_debug.h"
#include "nmp_appcore.h"
#include "nmp_mod_log.h"

void
jpf_cms_log_insert( void )
{
	JpfAppMod *mod;

	mod = g_object_new(JPF_TYPE_MODLOG, NULL);
	if (G_UNLIKELY(!mod))
	{
		jpf_error("<JpfModLog> alloc mod log failed!");
		FATAL_ERROR_EXIT;
	}

	jpf_app_mod_setup((JpfAppMod*)mod);
	jpf_afx_mod_insert(BUSSLOT_POS_LOG, mod);
}


//:~ End
