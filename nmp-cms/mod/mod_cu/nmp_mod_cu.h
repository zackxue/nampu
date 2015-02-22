#ifndef __NMP_MOD_CU_H__
#define __NMP_MOD_CU_H__

#include "nmp_mods.h"
#include "nmp_cu_struct.h"

#define TIMEOUT_N_PERIODS			3
#define HB_FREQ_DEFAULT				3000000

#define JPF_TYPE_MODCU	(jpf_mod_cu_get_type())
#define JPF_IS_MODCU(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), JPF_TYPE_MODCU))
#define JPF_IS_MODCU_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE((c), JPF_TYPE_MODCU))
#define JPF_MODCU(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), JPF_TYPE_MODCU, JpfModCu))
#define JPF_MODCU_CLASS(c) \
	(G_TYPE_CHECK_CLASS_CAST((c), JPF_TYPE_MODCU, JpfModCuClass))
#define JPF_MODCU_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), JPF_TYPE_MODCU, JpfModCuClass))


typedef struct _JpfModCu JpfModCu;
typedef struct _JpfModCuClass JpfModCuClass;
struct _JpfModCu
{
	JpfModAccess		parent_object;

	JpfGuestContainer	*container;		/* guests container */

	LIST_HEAD			list_group;		/* user group link list */
	GStaticMutex		list_glock;

	LIST_HEAD			list_user;		/* user link list */
	GStaticMutex		list_ulock;

	JpfNetIO			*listen_io;		/* listening io(socket) */

	gint				n_groups;		/* how many user groups we have */
	gint				n_users;		/* how many users we have */
	gint				n_sessions;		/* how many cu-clients are connected */
};


struct _JpfModCuClass
{
	JpfModAccessClass	parent_class;
};


GType jpf_mod_cu_get_type( void );

gint jpf_mod_cu_user_session_new(JpfModCu *self, JpfNetIO *io,
	const gchar *name, const gchar *passwd, gchar session[], gsize size);

JpfCu *jpf_mod_cu_get_cu(JpfModCu *self, JpfNetIO *io);
JpfCu *jpf_mod_cu_get_cu_2(JpfModCu *self, const gchar *sid);
void jpf_mod_cu_put_cu(JpfModCu *self, JpfCu *cu);

gint jpf_mod_cu_del_cu(JpfModCu *self, JpfCu *cu);
gint jpf_mod_cu_del_cu_2(JpfModCu *self, JpfNetIO *io, JpfID *out);

void jpf_mod_cu_deliver_msg(JpfModCu *self, const char *usr, JpfSysMsg *msg);
void jpf_mod_cu_deliver_msg_2(JpfModCu *self, JpfSysMsg *msg);
void jpf_mod_cu_force_usr_offline(JpfModCu *self,
    const char *usr_name, JpfSysMsg *msg);

void jpf_mod_cu_force_usr_offline_by_group(JpfModCu *self,
    gint group_id, JpfSysMsg *msg);

void
Jpf_mod_cu_broadcast_generic_msg(JpfModCu *self, gint id, gchar *parm1,
	gchar *parm2, gchar *parm3, gchar *content);

void
jpf_mod_cu_broadcast_to_all_user(JpfModCu *self, JpfSysMsg *msg);

G_END_DECLS


#endif	//__NMP_MOD_CU_H__
