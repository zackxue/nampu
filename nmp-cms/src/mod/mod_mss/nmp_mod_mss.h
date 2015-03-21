#ifndef __NMP_MOD_MSS_H__
#define __NMP_MOD_MSS_H__

#include "nmp_mods.h"
#include "nmp_internal_msg.h"

G_BEGIN_DECLS

#define DEFAULT_HEART_SLICE		3

#define NMP_TYPE_MODMSS	(nmp_mod_mss_get_type())
#define NMP_IS_MODMSS(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODMSS))
#define NMP_IS_MODMSS_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODMSS))
#define NMP_MODMSS(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODMSS, JpfModMss))
#define NMP_MODMSS_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODMSS, JpfModMssClass))
#define NMP_MODMSS_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODMSS, JpfModMssClass))

typedef struct _JpfModMss JpfModMss;
typedef struct _JpfModMssClass JpfModMssClass;
struct _JpfModMss
{
	JpfModAccess		parent_object;

	JpfGuestContainer	*container;
	JpfNetIO			*listen_io;

};


struct _JpfModMssClass
{
	JpfModAccessClass	parent_class;
};


GType nmp_mod_mss_get_type( void );


gint
nmp_mod_mss_new_mss(JpfModMss *self, JpfNetIO *io, const gchar *id,
	JpfID *conflict);

gint
nmp_mod_mss_sync_req(JpfModMss *self, NmpMsgID msg_id,
       gpointer req, gint req_size,  gpointer res, gint res_size);

gpointer
nmp_mod_mss_sync_req_2(JpfModMss *self, NmpMsgID msg_id,
       gpointer req, gint req_size, gint *res_size);

void
nmp_mod_mss_change_mss_online_status(NmpAppObj *app_obj,
    JpfMsgMssOnlineChange notify_info);

G_END_DECLS


#endif	//__NMP_MOD_MSS_H__
