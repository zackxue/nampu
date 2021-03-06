#ifndef __NMP_MOD_MDS_H__
#define __NMP_MOD_MDS_H__

#include "nmp_mods.h"
#include "nmp_internal_msg.h"

G_BEGIN_DECLS

#define DEFAULT_HEART_SLICE		3

#define NMP_TYPE_MODMDS	(nmp_mod_mds_get_type())
#define NMP_IS_MODMDS(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODMDS))
#define NMP_IS_MODMDS_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODMDS))
#define NMP_MODMDS(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODMDS, NmpModMds))
#define NMP_MODMDS_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODMDS, NmpModMdsClass))
#define NMP_MODMDS_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODMDS, NmpModMdsClass))

typedef struct _NmpModMds NmpModMds;
typedef struct _NmpModMdsClass NmpModMdsClass;
struct _NmpModMds
{
	NmpModAccess		parent_object;

	NmpGuestContainer	*container;
	NmpNetIO			*listen_io;

};


struct _NmpModMdsClass
{
	NmpModAccessClass	parent_class;
};


GType nmp_mod_mds_get_type( void );


gint
nmp_mod_mds_new_mds(NmpModMds *self, NmpNetIO *io, const gchar *id,
	NmpID *conflict);

gint
nmp_mod_mds_sync_req(NmpModMds *self, NmpMsgID msg_id,
       gpointer req, gint req_size,  gpointer res, gint res_size);

gpointer
nmp_mod_mds_sync_req_2(NmpModMds *self, NmpMsgID msg_id,
       gpointer req, gint req_size, gint *res_size);

void
nmp_mod_mds_change_mds_online_status(NmpAppObj *app_obj,
    NmpMsgMdsOnlineChange notify_info);
G_END_DECLS


#endif	//__NMP_MOD_MDS_H__
