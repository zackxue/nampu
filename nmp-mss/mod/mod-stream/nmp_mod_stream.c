#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_mods.h"
#include "nmp_mod_stream.h"
#include "nmp_sch_rtsp.h"


G_DEFINE_TYPE(NmpModStream, nmp_mod_stream, NMP_TYPE_APPMOD);

void nmp_mod_stream_register_msg_handler(NmpModStream *self);
void nmp_mod_stream_on_pp_event(gpointer priv, NmpMsg *msg);


static void 
nmp_mod_stream_init(NmpModStream *self)
{
//    NmpAppMod *a_self = ( NmpAppMod *)self;
}


gint
nmp_mod_stream_setup(NmpAppMod *am_self)
{
	NmpModStream *self = (NmpModStream*)am_self;
	G_ASSERT(self != NULL);

	nmp_app_mod_set_name(am_self, "MOD-STREAM");

	self->spool = nmp_spool_new(1000/TICK_PER_SECOND, &sch_rtsp_ops);
	if (!self->spool)
	{
		nmp_warning(
			"Create sch pool failed!"
		);
		FATAL_ERROR_EXIT;		
	}

	nmp_pool_set_handler((NmpPool*)self->spool,
		nmp_mod_stream_on_pp_event, self);

	nmp_mod_stream_register_msg_handler(self);
	return 0;
}


static void
nmp_mod_stream_class_init(NmpModStreamClass *k_class)
{
	NmpAppModClass *am_class = (NmpAppModClass*)k_class;

	am_class->setup_mod	= nmp_mod_stream_setup;
}


void
nmp_mod_stream_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODSTREAM, NULL);
	if (G_UNLIKELY(!mod))
	{
		nmp_error("<NmpModStream> alloc mod disk failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup((NmpAppMod*)mod);
	nmp_afx_mod_insert(BUSSLOT_POS_STREAM, mod);
}


//:~ End
