/*
 *	@author:	zyt
 *	@time:	2013/01/21
 */
#ifndef __NMP_MOD_WDD_H__
#define __NMP_MOD_WDD_H__

#include "nmp_mods.h"
#include "nmp_wdd.h"
#include "nmp_msg_share.h"

G_BEGIN_DECLS

#define DEFAULT_HEART_SLICE		(3)


#define JPF_TYPE_MODWDD	(jpf_mod_wdd_get_type())
#define JPF_IS_MODWDD(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_MODWDD))
#define JPF_IS_MODWDD_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_MODWDD))
#define JPF_MODWDD(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_MODWDD, JpfModWdd))
#define JPF_MODWDD_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_MODWDD, JpfModWddClass))
#define JPF_MODWDD_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_MODWDD, JpfModWddClass))


typedef struct _JpfModWdd JpfModWdd;
typedef struct _JpfModWddClass JpfModWddClass;

struct _JpfModWdd
{
	JpfAppMod	parent_object;
};

struct _JpfModWddClass
{
	JpfAppModClass	parent_class;
};


GType jpf_mod_wdd_get_type(void);

gint
jpf_wdd_get_expired_time(JpfExpiredTime *expired_time);

gint
jpf_wdd_get_version();

G_END_DECLS


#endif	//__NMP_MOD_WDD_H__
