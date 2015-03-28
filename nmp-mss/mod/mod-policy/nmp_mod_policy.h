#ifndef __NMP_MOD_POLICY_H__
#define __NMP_MOD_POLICY_H__

#include "nmp_gu.h"
#include "nmp_afx.h"

G_BEGIN_DECLS

#define DEFAULT_HEART_SLICE		3

#define NMP_TYPE_MODPOLICY	(nmp_mod_policy_get_type())
#define NMP_IS_MODPOLICY(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODPOLICY))
#define NMP_IS_MODPOLICY_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODPOLICY))
#define NMP_MODPOLICY(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODPOLICY, NmpModPolicy))
#define NMP_MODPOLICY_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODPOLICY, NmpModPolicyClass))
#define NMP_MODPOLICY_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODPOLICY, NmpModPolicyClass))

typedef struct _NmpModPolicy NmpModPolicy;
typedef struct _NmpModPolicyClass NmpModPolicyClass;
struct _NmpModPolicy
{
	NmpAppMod		parent_object;

	NmpGuPool	*gu_pool;
};


struct _NmpModPolicyClass
{
	NmpAppModClass	parent_class;
};


GType nmp_mod_policy_get_type( void );

G_END_DECLS

#endif	/* __NMP_MOD_POLICY_H__ */
