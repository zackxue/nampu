#ifndef __NMP_MOD_CU_H__
#define __NMP_MOD_CU_H__

#include "nmp_mods.h"
#include "nmp_cu_struct.h"

#define TIMEOUT_N_PERIODS			3
#define HB_FREQ_DEFAULT				3000000

#define NMP_TYPE_MODCU	(nmp_mod_cu_get_type())
#define NMP_IS_MODCU(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), NMP_TYPE_MODCU))
#define NMP_IS_MODCU_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), NMP_TYPE_MODCU))
#define NMP_MODCU(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), NMP_TYPE_MODCU, NmpModCu))
#define NMP_MODCU_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), NMP_TYPE_MODCU, NmpModCuClass))
#define NMP_MODCU_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), NMP_TYPE_MODCU, NmpModCuClass))


typedef struct _NmpModCu NmpModCu;
typedef struct _NmpModCuClass NmpModCuClass;
struct _NmpModCu
{
	NmpModAccess		parent_object;

	NmpGuestContainer	*container;		/* guests container */

	LIST_HEAD			list_group;		/* user group link list */
	GStaticMutex		list_glock;

	LIST_HEAD			list_user;		/* user link list */
	GStaticMutex		list_ulock;

	NmpNetIO			*listen_io;		/* listening io(socket) */

	gint				n_groups;		/* how many user groups we have */
	gint				n_users;		/* how many users we have */
	gint				n_sessions;		/* how many cu-clients are connected */
};


struct _NmpModCuClass
{
	NmpModAccessClass	parent_class;
};


GType nmp_mod_cu_get_type( void );

gint nmp_mod_cu_user_session_new(NmpModCu *self, NmpNetIO *io,
	const gchar *name, const gchar *passwd, gchar session[], gsize size);

NmpCu *nmp_mod_cu_get_cu(NmpModCu *self, NmpNetIO *io);
NmpCu *nmp_mod_cu_get_cu_2(NmpModCu *self, const gchar *sid);
void nmp_mod_cu_put_cu(NmpModCu *self, NmpCu *cu);

gint nmp_mod_cu_del_cu(NmpModCu *self, NmpCu *cu);
gint nmp_mod_cu_del_cu_2(NmpModCu *self, NmpNetIO *io, NmpID *out);

void nmp_mod_cu_deliver_msg(NmpModCu *self, const char *usr, NmpSysMsg *msg);
void nmp_mod_cu_deliver_msg_2(NmpModCu *self, NmpSysMsg *msg);
void nmp_mod_cu_force_usr_offline(NmpModCu *self,
    const char *usr_name, NmpSysMsg *msg);

void nmp_mod_cu_force_usr_offline_by_group(NmpModCu *self,
    gint group_id, NmpSysMsg *msg);

void
nmp_mod_cu_broadcast_generic_msg(NmpModCu *self, gint id, gchar *parm1,
	gchar *parm2, gchar *parm3, gchar *content);

void
nmp_mod_cu_broadcast_to_all_user(NmpModCu *self, NmpSysMsg *msg);

G_END_DECLS


#endif	//__NMP_MOD_CU_H__
