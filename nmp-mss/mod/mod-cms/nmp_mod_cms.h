#ifndef __NMP_MOD_CMS_H__
#define __NMP_MOD_CMS_H__

#include "nmp_afx.h"
#include "nmp_cms_conn.h"
#include "nmp_id_query.h"

G_BEGIN_DECLS

#define DEFAULT_HEART_SLICE		3

#define NMP_TYPE_MODCMS	(nmp_mod_cms_get_type())
#define NMP_IS_MODCMS(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODCMS))
#define NMP_IS_MODCMS_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODCMS))
#define NMP_MODCMS(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODCMS, NmpModCms))
#define NMP_MODCMS_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODCMS, NmpModCmsClass))
#define NMP_MODCMS_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODCMS, NmpModCmsClass))

typedef struct _NmpModCms NmpModCms;
typedef struct _NmpModCmsClass NmpModCmsClass;
struct _NmpModCms
{
	NmpModAccess		parent_object;

	NmpCmsConn			*cms_conn;
	guint				check_timer;

	NmpIDQueryBlock		*guid_qb;
	NmpIDQueryBlock		*mds_qb;
};


struct _NmpModCmsClass
{
	NmpModAccessClass	parent_class;
};


GType nmp_mod_cms_get_type( void );

gint nmp_mod_cms_post_msg(NmpCmsConn *conn, NmpSysMsg *msg);

G_END_DECLS

#endif	/* __NMP_MOD_CMS_H__ */
