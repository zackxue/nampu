#ifndef __NMP_MOD_BSS_H__
#define __NMP_MOD_BSS_H__

#include "nmp_mods.h"

G_BEGIN_DECLS

#define DEFAULT_HEART_SLICE		3

#define JPF_TYPE_MODBSS	(jpf_mod_bss_get_type())
#define JPF_IS_MODBSS(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_MODBSS))
#define JPF_IS_MODBSS_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_MODBSS))
#define JPF_MODBSS(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_MODBSS, JpfModBss))
#define JPF_MODBSS_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_MODBSS, JpfModBssClass))
#define JPF_MODBSS_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_MODBSS, JpfModBssClass))

typedef struct _JpfModBss JpfModBss;
typedef struct _JpfModBssClass JpfModBssClass;
struct _JpfModBss
{
	JpfModAccess		parent_object;

	JpfGuestContainer	*container;
	JpfNetIO			*listen_io;

	//LIST_HEAD		list_user;		/* user link list */
//	GStaticMutex		list_ulock;
};


struct _JpfModBssClass
{
	JpfModAccessClass	parent_class;
};


GType jpf_mod_bss_get_type( void );


gint
jpf_mod_bss_new_admin(JpfModBss *self, JpfNetIO *io, const gchar *id,
	JpfID *conflict);

gint
jpf_mod_bss_sync_req(JpfModBss *self, JpfMsgID msg_id,
       gpointer req, gint req_size,  gpointer res, gint res_size);

gpointer
jpf_mod_bss_sync_req_2(JpfModBss *self, JpfMsgID msg_id,
       gpointer req, gint req_size, gint *res_size);

void
jpf_mod_bss_deliver_out_msg(JpfAppObj *self, JpfSysMsg *msg);

void
jpf_mod_bss_force_usr_offline(JpfModBss *self, const char *admin_name, JpfSysMsg *msg);

void
jpf_mod_bss_notify_policy_change(JpfAppObj *self,
    gpointer policy_change, gint size);

void jpf_search_pu_lock();

void jpf_search_pu_unlock();

G_END_DECLS


#endif	//__NMP_MOD_BSS_H__
