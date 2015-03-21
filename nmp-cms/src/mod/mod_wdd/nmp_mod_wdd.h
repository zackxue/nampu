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


#define NMP_TYPE_MODWDD	(nmp_mod_wdd_get_type())
#define NMP_IS_MODWDD(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODWDD))
#define NMP_IS_MODWDD_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODWDD))
#define NMP_MODWDD(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODWDD, JpfModWdd))
#define NMP_MODWDD_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODWDD, JpfModWddClass))
#define NMP_MODWDD_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODWDD, JpfModWddClass))


typedef struct _JpfModWdd JpfModWdd;
typedef struct _JpfModWddClass JpfModWddClass;

struct _JpfModWdd
{
	NmpAppMod	parent_object;
};

struct _JpfModWddClass
{
	NmpAppModClass	parent_class;
};


GType nmp_mod_wdd_get_type(void);

gint
jpf_wdd_get_expired_time(JpfExpiredTime *expired_time);

gint
jpf_wdd_get_version();

G_END_DECLS


#endif	//__NMP_MOD_WDD_H__
