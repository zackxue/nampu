#include "nmp_mod_cms.h"
#include "nmp_mods.h"
#include "nmp_proto.h"


G_DEFINE_TYPE(NmpModCms, nmp_mod_cms, NMP_TYPE_MODACCESS);


void nmp_cms_conn_check_state(NmpModCms *mod, NmpCmsConn *conn);
void nmp_mod_cms_register_msg_handler(NmpModCms *self);
static NmpModCms *this_module = NULL;

static gboolean
nmp_mod_cms_on_check_timer(gpointer user_data)
{
	NmpModCms *mod = (NmpModCms*)user_data;

	nmp_cms_conn_check_state(mod, mod->cms_conn);

	return TRUE;
}


static gint
nmp_mod_cms_setup(NmpAppMod *am_self)
{
	NmpModAccess *ma_self = (NmpModAccess*)am_self;
	NmpModCms *self = (NmpModCms*)am_self;

	nmp_mod_acc_init_net(ma_self, &nmp_packet_proto, &nmp_xml_proto);

	self->cms_conn = nmp_create_cms_conn();
	self->check_timer = nmp_set_timer(1000, nmp_mod_cms_on_check_timer, self);
	self->guid_qb = nmp_id_qb_new();
	self->mds_qb = nmp_id_qb_new();

	nmp_mod_cms_register_msg_handler(self);
	nmp_app_mod_set_name(am_self, "MOD-CMS");
	this_module = self;

	return 0;
}


static void
nmp_mod_cms_io_close(NmpModAccess *s_self, NmpNetIO *io, gint err)
{
	NmpModCms *self;
	G_ASSERT(s_self != NULL);

	self = (NmpModCms*)s_self;

	nmp_id_qb_stop(self->guid_qb);
	nmp_id_qb_stop(self->mds_qb);
	nmp_cms_conn_io_fin(self->cms_conn, io);
}


static gint
nmp_mod_cms_io_init(NmpModAccess *s_self, NmpNetIO *io)
{
	BUG();
	return 0;
}


static void
nmp_mod_cms_init(NmpModCms *self)
{
	self->cms_conn = NULL;
	self->check_timer = 0;
}


static void
nmp_mod_cms_class_init(NmpModCmsClass *k_class)
{
	NmpModAccessClass *ma_class = (NmpModAccessClass*)k_class;
	NmpAppModClass *am_class = (NmpAppModClass*)k_class;

	ma_class->io_close = nmp_mod_cms_io_close;
	am_class->setup_mod	= nmp_mod_cms_setup;
}


void
nmp_mod_cms_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODCMS, NULL);
	if (G_UNLIKELY(!mod))
	{
		nmp_warning("<NmpModCms> alloc mod cms failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup(mod);
	nmp_afx_mod_insert(BUSSLOT_POS_CMS, mod);
}


gint
nmp_mod_cms_state( void )
{
	if (!this_module)
		return -1;

	return nmp_cms_conn_state(this_module->cms_conn);
}


//:~ End
