#ifndef __NMP_MOD_PU_H__
#define __NMP_MOD_PU_H__

#include "nmp_mods.h"
#include "nmp_pu_struct.h"
#include "nmp_msg_pu.h"

G_BEGIN_DECLS


#define DEFAULT_HEART_SLICE		3

#define NMP_TYPE_MODPU	(nmp_mod_pu_get_type())
#define NMP_IS_MODPU(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODPU))
#define NMP_IS_MODPU_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODPU))
#define NMP_MODPU(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODPU, NmpModPu))
#define NMP_MODPU_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODPU, NmpModPuClass))
#define NMP_MODPU_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODPU, NmpModPuClass))

typedef struct _NmpModPu NmpModPu;
typedef struct _NmpModPuClass NmpModPuClass;
struct _NmpModPu
{
	NmpModAccess		parent_object;

	NmpGuestContainer	*container;
	NmpNetIO			*listen_io;
};


struct _NmpModPuClass
{
	NmpModAccessClass	parent_class;
};


GType nmp_mod_pu_get_type( void );

void
nmp_mod_pu_change_pu_online_status(NmpAppObj *app_obj,
    NmpPuOnlineStatusChange notify_info);

void
nmp_mod_pu_update_online_status(NmpAppObj *self, NmpSysMsg *msg);

gint
nmp_mod_pu_register(NmpModPu *self, NmpNetIO *io, const gchar *id,
	NmpPuType t, NmpID *conflict);

gint
nmp_mod_pu_sync_req(NmpModPu *self, NmpMsgID msg_id,
       gpointer req, gint req_size,  gpointer res, gint res_size);

void nmp_mod_pu_set_recheck_tag(NmpModPu *self);

G_END_DECLS


#endif	//__NMP_MOD_PU_H__
