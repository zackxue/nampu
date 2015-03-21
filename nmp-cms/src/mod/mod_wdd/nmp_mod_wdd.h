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
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODWDD, NmpModWdd))
#define NMP_MODWDD_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODWDD, NmpModWddClass))
#define NMP_MODWDD_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODWDD, NmpModWddClass))


typedef struct _NmpModWdd NmpModWdd;
typedef struct _NmpModWddClass NmpModWddClass;

struct _NmpModWdd
{
	NmpAppMod	parent_object;
};

struct _NmpModWddClass
{
	NmpAppModClass	parent_class;
};


GType nmp_mod_wdd_get_type(void);

gint
nmp_wdd_get_expired_time(NmpExpiredTime *expired_time);

gint
nmp_wdd_get_version();

G_END_DECLS


#endif	//__NMP_MOD_WDD_H__
