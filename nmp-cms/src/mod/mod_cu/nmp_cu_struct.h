#ifndef __NMP_CU_STRUCT_H__
#define __NMP_CU_STRUCT_H__

#include "nmp_mods.h"
#include "nmp_wait.h"
#include "nmp_list_head.h"

#define MAX_NAME_LEN				32
#define MAX_PASSWD_LEN				16


typedef enum
{
	USR_GRP_NONE_PERMISSION	= 0,
	USR_GRP_RECORD_BACKUP	= 1 << 0,
	USR_GRP_WALL_CONTROL		= 1 << 1,
	USR_GRP_VIDEO_HISTORY	= 1 << 2,
	USR_GRP_PARAMETER_SET 	= 1 << 3,
	USR_GRP_PTZ_CONTROL		= 1 << 4,
	USR_GRP_VIDEO_LIVE       	= 1 << 5,
	USR_GRP_ELECTRONIC_MAP   	= 1 << 6,
	USR_GRP_DEAL_ALARM     	= 1 << 7,
	USR_GRP_BROWSE_DEV_STATUS     	= 1 << 8
}JpfUsrGrpPermissions;

#define NMP_USR_GRP_PERMISSION(permission, flag)		(permission & flag)


typedef enum
{
	USR_FLG_DELETED	= 1 << 0,

}JpfUsrFlags;

typedef enum
{
	STAT_GRP_NEW,
	STAT_GRP_REQUEST,
	STAT_GRP_COMPLETED,
	STAT_GRP_FAILED
}JpfUsrGrpState;


typedef enum
{
	SHARE_MODE_UNKNOWN,
	SHARE_MODE_SHARED,
	SHARE_MODE_EXCLUSIVE,
	SHARE_MODE_GRAB
}JpfUsrShareMode;


typedef enum
{
	STAT_USR_NEW,
	STAT_USR_REQUEST,
	STAT_USR_FILLED,
	STAT_USR_COMPLETED,
	STAT_USR_FAILED
}JpfUsrState;

typedef struct _JpfUsrGroup JpfUsrGroup;
struct _JpfUsrGroup
{
	LIST_HEAD		list;

	JpfWait			*wait;			/* wait for completion */
	JpfUsrGrpState	state;

	gint			ref_count;
	gint			id;				/* group id */
	gint			rank;			/* group rank */
	guint			permissions;	/* set of permissions */
	gchar			name[MAX_NAME_LEN];
	GStaticMutex	lock;			/* protected all above */
	gint			err_no;
	gpointer		owner;
};


typedef struct _JpfUsr JpfUsr;
struct _JpfUsr
{
	LIST_HEAD		list;

	JpfWait			*wait;		/* wait for completion */

	gint			ref_count;
	gchar			user_name[MAX_NAME_LEN];
	gchar			user_passwd[MAX_PASSWD_LEN];
	JpfUsrShareMode	share_mode;
	JpfUsrState		user_state;
	gint			err_no;

	gint			flags;		/* USR_FLG_DELETED */

	gint			group_id;
	JpfUsrGroup		*user_group;	/* point to the group we belong */
	GStaticMutex	lock;			/* protected all above */

	gint			n_sessions;
	LIST_HEAD		list_session;
	GStaticMutex	list_slock;
};


typedef struct _JpfCu JpfCu;
struct _JpfCu
{
	JpfGuestBase	guest_base;		/* include session id */

	gint			ttl;			/* time to live */
	gint			hb_freq;		/* keep alive frequency (sec) */

	LIST_HEAD		list;

	JpfUsr			*user;
};


static __inline__ JpfUsr *
jpf_get_session_usr(JpfCu *cu)
{
	BUG_ON(!cu);
	return cu->user;
}

static __inline__ char *
jpf_get_usr_name(JpfCu *cu)
{
	BUG_ON(!cu);
	JpfUsr *usr;

	usr = jpf_get_session_usr(cu);
	return usr->user_name;
}

static __inline__ JpfUsrGroup *
jpf_get_usr_group(JpfUsr *user)
{
	BUG_ON(!user);
	return user->user_group;
}

static __inline__ guint
jpf_get_usr_permissions(JpfCu *cu)
{
    BUG_ON(!cu);

    JpfUsr *usr;
    JpfUsrGroup *user_group;

    usr = jpf_get_session_usr(cu);
    user_group = jpf_get_usr_group(usr);
    BUG_ON(!user_group);

    return user_group->permissions;
}

static __inline__ guint
jpf_get_usr_rank(JpfCu *cu)
{
    BUG_ON(!cu);

    JpfUsr *usr;
    JpfUsrGroup *user_group;

    usr = jpf_get_session_usr(cu);
    user_group = jpf_get_usr_group(usr);
    BUG_ON(!user_group);

    return user_group->rank;
}

void nmp_mod_cu_add_user_info(JpfUsr *user, gchar *passwd, gint grp_id);
void nmp_mod_cu_add_group_info(JpfUsrGroup *grp, gint rank, guint perm);

#endif	//__NMP_CU_STRUCT_H__
