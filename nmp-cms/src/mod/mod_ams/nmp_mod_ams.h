#ifndef __NMP_MOD_AMS_H__
#define __NMP_MOD_AMS_H__

#include "nmp_mods.h"
#include "nmp_internal_msg.h"

G_BEGIN_DECLS

#define DEFAULT_HEART_SLICE		3

#define NMP_TYPE_MODAMS	(nmp_mod_ams_get_type())
#define NMP_IS_MODAMS(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODAMS))
#define NMP_IS_MODAMS_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODAMS))
#define NMP_MODAMS(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODAMS, JpfModAms))
#define NMP_MODAMS_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODAMS, JpfModAmsClass))
#define NMP_MODAMS_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODAMS, JpfModAmsClass))

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


GType nmp_mod_ams_get_type( void );


gint
nmp_mod_ams_new_ams(JpfModAms *self, JpfNetIO *io, const gchar *id,
	JpfID *conflict);

gint
nmp_mod_ams_sync_req(JpfModAms *self, NmpMsgID msg_id,
	gpointer req, gint req_size,  gpointer res, gint res_size);

gpointer
nmp_mod_ams_sync_req_2(JpfModAms *self, NmpMsgID msg_id,
	gpointer req, gint req_size, gint *res_size);

void
nmp_mod_ams_change_ams_online_status(NmpAppObj *app_obj,
	JpfMsgAmsOnlineChange notify_info);

JpfModAms *jpf_get_mod_ams();

G_END_DECLS


#endif	//__NMP_MOD_AMS_H__
