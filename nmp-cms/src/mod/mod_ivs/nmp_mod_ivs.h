#ifndef __NMP_MOD_IVS_H__
#define __NMP_MOD_IVS_H__

#include "nmp_mods.h"
#include "nmp_internal_msg.h"

G_BEGIN_DECLS

#define DEFAULT_HEART_SLICE		3

#define NMP_TYPE_MODIVS	(nmp_mod_ivs_get_type())
#define NMP_IS_MODIVS(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODIVS))
#define NMP_IS_MODIVS_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODIVS))
#define NMP_MODIVS(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODIVS, JpfModIvs))
#define NMP_MODIVS_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODIVS, JpfModIvsClass))
#define NMP_MODIVS_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODIVS, JpfModIvsClass))

typedef struct _JpfModIvs JpfModIvs;
typedef struct _JpfModIvsClass JpfModIvsClass;
struct _JpfModIvs
{
	JpfModAccess		parent_object;

	JpfGuestContainer	*container;
	JpfNetIO			*listen_io;

};


struct _JpfModIvsClass
{
	JpfModAccessClass	parent_class;
};


GType nmp_mod_ivs_get_type( void );


gint
nmp_mod_ivs_new_ivs(JpfModIvs *self, JpfNetIO *io, const gchar *id,
	JpfID *conflict);

gint
nmp_mod_ivs_sync_req(JpfModIvs *self, NmpMsgID msg_id,
       gpointer req, gint req_size,  gpointer res, gint res_size);

gpointer
nmp_mod_ivs_sync_req_2(JpfModIvs *self, NmpMsgID msg_id,
       gpointer req, gint req_size, gint *res_size);

void
nmp_mod_ivs_change_ivs_online_status(NmpAppObj *app_obj,
    JpfMsgIvsOnlineChange notify_info);

G_END_DECLS


#endif	//__NMP_MOD_IVS_H__
