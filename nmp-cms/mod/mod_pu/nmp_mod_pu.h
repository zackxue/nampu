#ifndef __NMP_MOD_PU_H__
#define __NMP_MOD_PU_H__

#include "nmp_mods.h"
#include "nmp_pu_struct.h"
#include "nmp_msg_pu.h"

G_BEGIN_DECLS


#define DEFAULT_HEART_SLICE		3

#define JPF_TYPE_MODPU	(jpf_mod_pu_get_type())
#define JPF_IS_MODPU(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_MODPU))
#define JPF_IS_MODPU_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_MODPU))
#define JPF_MODPU(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_MODPU, JpfModPu))
#define JPF_MODPU_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_MODPU, JpfModPuClass))
#define JPF_MODPU_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_MODPU, JpfModPuClass))

typedef struct _JpfModPu JpfModPu;
typedef struct _JpfModPuClass JpfModPuClass;
struct _JpfModPu
{
	JpfModAccess		parent_object;

	JpfGuestContainer	*container;
	JpfNetIO			*listen_io;
};


struct _JpfModPuClass
{
	JpfModAccessClass	parent_class;
};


GType jpf_mod_pu_get_type( void );

void
jpf_mod_pu_change_pu_online_status(JpfAppObj *app_obj,
    JpfPuOnlineStatusChange notify_info);

void
jpf_mod_pu_update_online_status(JpfAppObj *self, JpfSysMsg *msg);

gint
jpf_mod_pu_register(JpfModPu *self, JpfNetIO *io, const gchar *id,
	JpfPuType t, JpfID *conflict);

gint
jpf_mod_pu_sync_req(JpfModPu *self, JpfMsgID msg_id,
       gpointer req, gint req_size,  gpointer res, gint res_size);

void jpf_mod_pu_set_recheck_tag(JpfModPu *self);

G_END_DECLS


#endif	//__NMP_MOD_PU_H__
