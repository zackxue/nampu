#ifndef __NMP_MOD_IVS_H__
#define __NMP_MOD_IVS_H__

#include "nmp_mods.h"
#include "nmp_internal_msg.h"

G_BEGIN_DECLS

#define DEFAULT_HEART_SLICE		3

#define JPF_TYPE_MODIVS	(jpf_mod_ivs_get_type())
#define JPF_IS_MODIVS(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_MODIVS))
#define JPF_IS_MODIVS_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_MODIVS))
#define JPF_MODIVS(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_MODIVS, JpfModIvs))
#define JPF_MODIVS_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_MODIVS, JpfModIvsClass))
#define JPF_MODIVS_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_MODIVS, JpfModIvsClass))

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


GType jpf_mod_ivs_get_type( void );


gint
jpf_mod_ivs_new_ivs(JpfModIvs *self, JpfNetIO *io, const gchar *id,
	JpfID *conflict);

gint
jpf_mod_ivs_sync_req(JpfModIvs *self, JpfMsgID msg_id,
       gpointer req, gint req_size,  gpointer res, gint res_size);

gpointer
jpf_mod_ivs_sync_req_2(JpfModIvs *self, JpfMsgID msg_id,
       gpointer req, gint req_size, gint *res_size);

void
jpf_mod_ivs_change_ivs_online_status(JpfAppObj *app_obj,
    JpfMsgIvsOnlineChange notify_info);

G_END_DECLS


#endif	//__NMP_MOD_IVS_H__
