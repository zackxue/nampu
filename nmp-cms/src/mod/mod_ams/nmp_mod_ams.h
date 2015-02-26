#ifndef __NMP_MOD_AMS_H__
#define __NMP_MOD_AMS_H__

#include "nmp_mods.h"
#include "nmp_internal_msg.h"

G_BEGIN_DECLS

#define DEFAULT_HEART_SLICE		3

#define JPF_TYPE_MODAMS	(jpf_mod_ams_get_type())
#define JPF_IS_MODAMS(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_MODAMS))
#define JPF_IS_MODAMS_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_MODAMS))
#define JPF_MODAMS(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_MODAMS, JpfModAms))
#define JPF_MODAMS_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_MODAMS, JpfModAmsClass))
#define JPF_MODAMS_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_MODAMS, JpfModAmsClass))

typedef struct _JpfModAms JpfModAms;
typedef struct _JpfModAmsClass JpfModAmsClass;
struct _JpfModAms
{
	JpfModAccess		parent_object;

	JpfGuestContainer	*container;
	JpfNetIO			*listen_io;

};


struct _JpfModAmsClass
{
	JpfModAccessClass	parent_class;
};


GType jpf_mod_ams_get_type( void );


gint
jpf_mod_ams_new_ams(JpfModAms *self, JpfNetIO *io, const gchar *id,
	JpfID *conflict);

gint
jpf_mod_ams_sync_req(JpfModAms *self, JpfMsgID msg_id,
	gpointer req, gint req_size,  gpointer res, gint res_size);

gpointer
jpf_mod_ams_sync_req_2(JpfModAms *self, JpfMsgID msg_id,
	gpointer req, gint req_size, gint *res_size);

void
jpf_mod_ams_change_ams_online_status(JpfAppObj *app_obj,
	JpfMsgAmsOnlineChange notify_info);

JpfModAms *jpf_get_mod_ams();

G_END_DECLS


#endif	//__NMP_MOD_AMS_H__
