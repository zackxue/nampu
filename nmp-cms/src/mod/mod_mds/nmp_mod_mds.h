#ifndef __NMP_MOD_MDS_H__
#define __NMP_MOD_MDS_H__

#include "nmp_mods.h"
#include "nmp_internal_msg.h"

G_BEGIN_DECLS

#define DEFAULT_HEART_SLICE		3

#define JPF_TYPE_MODMDS	(jpf_mod_mds_get_type())
#define JPF_IS_MODMDS(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_MODMDS))
#define JPF_IS_MODMDS_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_MODMDS))
#define JPF_MODMDS(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_MODMDS, JpfModMds))
#define JPF_MODMDS_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_MODMDS, JpfModMdsClass))
#define JPF_MODMDS_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_MODMDS, JpfModMdsClass))

typedef struct _JpfModMds JpfModMds;
typedef struct _JpfModMdsClass JpfModMdsClass;
struct _JpfModMds
{
	JpfModAccess		parent_object;

	JpfGuestContainer	*container;
	JpfNetIO			*listen_io;

};


struct _JpfModMdsClass
{
	JpfModAccessClass	parent_class;
};


GType jpf_mod_mds_get_type( void );


gint
jpf_mod_mds_new_mds(JpfModMds *self, JpfNetIO *io, const gchar *id,
	JpfID *conflict);

gint
jpf_mod_mds_sync_req(JpfModMds *self, JpfMsgID msg_id,
       gpointer req, gint req_size,  gpointer res, gint res_size);

gpointer
jpf_mod_mds_sync_req_2(JpfModMds *self, JpfMsgID msg_id,
       gpointer req, gint req_size, gint *res_size);

void
jpf_mod_mds_change_mds_online_status(JpfAppObj *app_obj,
    JpfMsgMdsOnlineChange notify_info);
G_END_DECLS


#endif	//__NMP_MOD_MDS_H__
