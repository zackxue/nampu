#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_mod_vod.h"
#include "nmp_rtsp_mds.h"
#include "nmp_jfs_records.h"


G_DEFINE_TYPE(NmpModVod, nmp_mod_vod, NMP_TYPE_APPMOD);

void nmp_mod_vod_register_msg_handler(NmpModVod *self);
void nmp_mod_vod_on_pp_event(gpointer priv, NmpMsg *msg);


static void 
nmp_mod_vod_init(NmpModVod *self)
{
//    NmpAppMod *a_self = ( NmpAppMod *)self;
	nmp_record_stor_init();
}


gint
nmp_mod_vod_setup(NmpAppMod *am_self)
{
	NmpModVod *self = (NmpModVod*)am_self;
	G_ASSERT(self != NULL);

	nmp_app_mod_set_name(am_self, "MOD-VOD");
	self->mds_pool = nmp_spool_mds_new(1000/TICK_PER_SECOND, &nmp_rtsp_mds_ops);
	if (!self->mds_pool)
	{
		nmp_warning(
			"Create mds pool failed!"
		);
		FATAL_ERROR_EXIT;		
	}

	self->records = nmp_records_new(&nmp_advanced_records);
	if (!self->records)
	{
		nmp_warning(
			"Create mds records failed!"
		);
		FATAL_ERROR_EXIT;	
	}
register_file_ops(&jfs_file_ops);
	nmp_pool_set_handler((NmpPool*)self->mds_pool,
		nmp_mod_vod_on_pp_event, self);

	nmp_mod_vod_register_msg_handler(self);
	return 0;
}


static void
nmp_mod_vod_class_init(NmpModVodClass *k_class)
{
	NmpAppModClass *am_class = (NmpAppModClass*)k_class;

	am_class->setup_mod	= nmp_mod_vod_setup;
}


//:~ End
