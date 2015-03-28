#include "nmp_debug.h"
#include "nmp_mods.h"
#include "nmp_mod_disk.h"

void
nmp_mod_disk_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODDISK, NULL);
	if (G_UNLIKELY(!mod))
	{
		nmp_error("<NmpModDisk> alloc mod disk failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup((NmpAppMod*)mod);
	nmp_afx_mod_insert(BUSSLOT_POS_DISK, mod);
}


//:~ End
