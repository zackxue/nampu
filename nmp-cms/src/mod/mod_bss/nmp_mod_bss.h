#ifndef __NMP_MOD_BSS_H__
#define __NMP_MOD_BSS_H__

#include "nmp_mods.h"

G_BEGIN_DECLS

#define DEFAULT_HEART_SLICE		3

#define NMP_TYPE_MODBSS	(nmp_mod_bss_get_type())
#define NMP_IS_MODBSS(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODBSS))
#define NMP_IS_MODBSS_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODBSS))
#define NMP_MODBSS(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODBSS, NmpModBss))
#define NMP_MODBSS_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODBSS, NmpModBssClass))
#define NMP_MODBSS_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODBSS, NmpModBssClass))

typedef struct _NmpModBss NmpModBss;
typedef struct _NmpModBssClass NmpModBssClass;
struct _NmpModBss
{
	NmpModAccess		parent_object;

	NmpGuestContainer	*container;
	NmpNetIO			*listen_io;

	//LIST_HEAD		list_user;		/* user link list */
//	GStaticMutex		list_ulock;
};


struct _NmpModBssClass
{
	NmpModAccessClass	parent_class;
};


GType nmp_mod_bss_get_type( void );


gint
nmp_mod_bss_new_admin(NmpModBss *self, NmpNetIO *io, const gchar *id,
	NmpID *conflict);

gint
nmp_mod_bss_sync_req(NmpModBss *self, NmpMsgID msg_id,
       gpointer req, gint req_size,  gpointer res, gint res_size);

gpointer
nmp_mod_bss_sync_req_2(NmpModBss *self, NmpMsgID msg_id,
       gpointer req, gint req_size, gint *res_size);

void
nmp_mod_bss_deliver_out_msg(NmpAppObj *self, NmpSysMsg *msg);

void
nmp_mod_bss_force_usr_offline(NmpModBss *self, const char *admin_name, NmpSysMsg *msg);

void
nmp_mod_bss_notify_policy_change(NmpAppObj *self,
    gpointer policy_change, gint size);

void nmp_search_pu_lock();

void nmp_search_pu_unlock();

G_END_DECLS


#endif	//__NMP_MOD_BSS_H__
