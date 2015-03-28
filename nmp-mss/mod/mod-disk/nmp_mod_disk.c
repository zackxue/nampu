#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_mod_disk.h"
#include "nmp_storage_type.h"


G_DEFINE_TYPE(NmpModDisk, nmp_mod_disk, NMP_TYPE_APPMOD);

void nmp_mod_disk_register_msg_handler(NmpModDisk *self);

static void 
nmp_mod_disk_init(NmpModDisk *self)
{
//    NmpAppMod *a_self = ( NmpAppMod *)self;

//	nmp_register_fake_storage();
	nmp_register_jfs_storage();
}


gint
nmp_mod_disk_setup(NmpAppMod *am_self)
{
	NmpModDisk *self = (NmpModDisk*)am_self;
	G_ASSERT(self != NULL);

	nmp_app_mod_set_name(am_self, "MOD-DISK");

	nmp_mod_disk_register_msg_handler(self);
	return 0;
}


static void
nmp_mod_disk_class_init(NmpModDiskClass *k_class)
{
	NmpAppModClass *am_class = (NmpAppModClass*)k_class;

	am_class->setup_mod	= nmp_mod_disk_setup;
}


//:~ End
