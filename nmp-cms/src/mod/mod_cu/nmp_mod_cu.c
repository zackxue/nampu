#include "nmp_mod_cu.h"
#include "nmp_debug.h"
#include "nmp_cu_struct.h"
#include "nmp_errno.h"
#include "nmp_ports.h"
#include "nmp_cu_msg.h"
#include "message/nmp_msg_cu.h"
#include "nmp_memory.h"
#include "nmp_shared.h"
#include "nmp_proto.h"


G_DEFINE_TYPE(JpfModCu, nmp_mod_cu, NMP_TYPE_MODACCESS);

void
nmp_mod_cu_register_msg_handler(JpfModCu *self);

GStaticMutex gen_sid_lock = G_STATIC_MUTEX_INIT;
//:TODO
//static guint msg_seq_generator = 0;

static __inline__ void
nmp_mod_cu_cal_ttl(JpfCu *cu)
{
	G_ASSERT(cu != NULL);

	cu->ttl = cu->hb_freq * TIMEOUT_N_PERIODS;
}


static __inline__ void
nmp_mod_cu_gen_session_id(gchar buf[], gsize size)
{
	static gint sid_no = 0;

	g_static_mutex_lock(&gen_sid_lock);

	snprintf(
		buf, size, "%s-%05d",
		jpf_get_local_domain_id(),
		(++sid_no) % 1000000
	);

	g_static_mutex_unlock(&gen_sid_lock);
}


static __inline__ void
nmp_mod_cu_struct_init(JpfCu *cu)
{
	cu->hb_freq = HB_FREQ_DEFAULT;
	nmp_mod_cu_cal_ttl(cu);
	cu->user = NULL;
}


static __inline__ void
nmp_mod_cu_group_ref(JpfUsrGroup *grp)
{
	G_ASSERT(grp != NULL && g_atomic_int_get(&grp->ref_count) > 0);

	g_atomic_int_inc(&grp->ref_count);
}


static __inline__ void
nmp_mod_cu_group_release(JpfUsrGroup *grp)
{
	G_ASSERT(grp != NULL);

	jpf_wait_free(grp->wait);
	g_free(grp);
}


static __inline__ JpfUsr *
nmp_mod_cu_user_new(const gchar *name)
{
	JpfUsr *user;

	user = g_new0(JpfUsr, 1);
	if (G_UNLIKELY(!user))
		return NULL;

	user->wait = jpf_wait_new();
	if (G_UNLIKELY(!user->wait))
	{
		g_free(user);
		return NULL;
	}

	INIT_LIST_HEAD(&user->list);
	user->ref_count = 1;
	strncpy(user->user_name, name, MAX_NAME_LEN - 1);
	user->share_mode = SHARE_MODE_SHARED;
	user->user_state = STAT_USR_NEW;
	user->group_id = -1;
	user->user_group = NULL;
	g_static_mutex_init(&user->lock);
	user->n_sessions = 0;
	INIT_LIST_HEAD(&user->list_session);
	g_static_mutex_init(&user->list_slock);

	return user;
}


static __inline__ void
nmp_mod_cu_user_release(JpfUsr *user)
{
	G_ASSERT(user != NULL);

	jpf_wait_free(user->wait);
	g_free(user);
}


void
nmp_mod_cu_add_user_info(JpfUsr *user, gchar *passwd, gint grp_id)
{
	G_ASSERT(user != NULL && passwd != NULL);

	g_static_mutex_lock(&user->lock);
	strncpy(user->user_passwd, passwd, MAX_PASSWD_LEN - 1);
	user->group_id = grp_id;
	user->user_state = STAT_USR_FILLED;
	g_static_mutex_unlock(&user->lock);
}


static __inline__ JpfUsrState
nmp_mod_cu_user_state(JpfUsr *user)
{
	JpfUsrState state;

	g_static_mutex_lock(&user->lock);
	state = user->user_state;
	if (state == STAT_USR_NEW)
		user->user_state = STAT_USR_REQUEST;
	g_static_mutex_unlock(&user->lock);

	return state;
}


static __inline__ void
nmp_mod_cu_set_user_state(JpfUsr *user, JpfUsrState state)
{
	g_static_mutex_lock(&user->lock);
	user->user_state = state;
	g_static_mutex_unlock(&user->lock);
}


JpfUsrShareMode
nmp_mod_cu_user_mode(JpfUsr *user)
{
	JpfUsrShareMode mode;

	g_static_mutex_lock(&user->lock);
	mode = user->share_mode;
	g_static_mutex_unlock(&user->lock);

	return mode;
}


static __inline__ gint
nmp_mod_cu_set_user_errno(JpfUsr *user, gint err)
{
	G_ASSERT(user != NULL);

	user->err_no = err;
}


static __inline__ gint
nmp_mod_cu_get_user_errno(JpfUsr *user)
{
	G_ASSERT(user != NULL);

	return user->err_no;
}


static __inline__ gint
nmp_mod_cu_is_user_new(JpfUsr *user)
{
	return nmp_mod_cu_user_state(user) == STAT_USR_NEW;
}


static __inline__ gint
nmp_mod_cu_user_failed(JpfUsrState state)
{
	return state == STAT_USR_FAILED;
}


static __inline__ gint
nmp_mod_cu_need_wait_user(JpfUsr *user)
{
	JpfUsrState state;
	G_ASSERT(user != NULL);

	state = nmp_mod_cu_user_state(user);
	return state != STAT_USR_NEW &&
		state != STAT_USR_COMPLETED &&
		state != STAT_USR_FAILED;
}


static __inline__ gint
nmp_mod_cu_is_user_complete(JpfUsr *user)
{
	JpfUsrState state;
	G_ASSERT(user != NULL);

	state = nmp_mod_cu_user_state(user);
	BUG_ON(state != STAT_USR_COMPLETED && state != STAT_USR_FAILED);
	return state == STAT_USR_COMPLETED;
}


static __inline__ void
nmp_mod_cu_user_wakeup(JpfUsr *user, gint err)
{
	JpfUsrState state;
	G_ASSERT(user != NULL);

	state = err ? STAT_USR_FAILED : STAT_USR_COMPLETED;
	jpf_wait_begin(user->wait);
	nmp_mod_cu_set_user_state(user, state);
	user->err_no = err;
	jpf_wait_wakeup_all(user->wait);
	jpf_wait_end(user->wait);
}


static __inline__ void
nmp_mod_cu_wait_user(JpfUsr *user)
{
	jpf_wait_begin(user->wait);

	while (nmp_mod_cu_need_wait_user(user))
		jpf_wait_waiting(user->wait);

	jpf_wait_end(user->wait);
}


void
nmp_mod_cu_add_user_session(JpfUsr *user, JpfCu *cu)
{
	G_ASSERT(user != NULL && cu != NULL);

	g_static_mutex_lock(&user->list_slock);
	list_add2(&cu->list, &user->list_session);
	++user->n_sessions;
	g_atomic_int_inc(&user->ref_count);
	cu->user = user;
	g_static_mutex_unlock(&user->list_slock);
}


static __inline__ JpfUsrGroup *
nmp_mod_cu_group_new(JpfModCu *mod, gint grp_id)
{
	JpfUsrGroup *grp;

	grp = g_new0(JpfUsrGroup, 1);
	if (G_UNLIKELY(!grp))
		return NULL;

	grp->wait = jpf_wait_new();
	if (G_UNLIKELY(!grp->wait))
	{
		g_free(grp);
		return NULL;
	}

	INIT_LIST_HEAD(&grp->list);
	grp->id = grp_id;
	grp->state = STAT_GRP_NEW;
	grp->ref_count = 1;
	g_static_mutex_init(&grp->lock);
	grp->owner = mod;

	return grp;
}


void
nmp_mod_cu_add_group_info(JpfUsrGroup *grp, gint rank, guint perm)
{
	G_ASSERT(grp != NULL);

	grp->rank = rank;
	grp->permissions = perm;
}


static __inline__ JpfUsrGroup *
__nmp_mod_cu_find_group(JpfModCu *self, gint grp_id)
{
	LIST_HEAD *l;
	JpfUsrGroup *grp;

	list_for_each(l, &self->list_group)
	{
		grp = list_entry(l, JpfUsrGroup, list);
		if (grp->id == grp_id)
			return grp;
	}

	return  NULL;
}


static __inline__ JpfUsrGroup *
nmp_mod_cu_find_and_get_group(JpfModCu *self, gint grp_id)
{
	JpfUsrGroup *grp;
	G_ASSERT(self != NULL);

	g_static_mutex_lock(&self->list_glock);
	grp = __nmp_mod_cu_find_group(self, grp_id);
	if (G_LIKELY(grp))
		g_atomic_int_inc(&grp->ref_count);
	g_static_mutex_unlock(&self->list_glock);

	return grp;
}


static __inline__ void
nmp_mod_cu_put_group(JpfModCu *self, JpfUsrGroup *grp)
{
	G_ASSERT(self != NULL && grp != NULL);

	g_static_mutex_lock(&self->list_glock);
	if (g_atomic_int_dec_and_test(&grp->ref_count))
	{
		list_del(&grp->list);
		nmp_mod_cu_group_release(grp);
	}
	g_static_mutex_unlock(&self->list_glock);
}


static __inline__ gint
nmp_mod_cu_add_group(JpfModCu *self, JpfUsrGroup *grp)
{
	JpfUsrGroup *old;
	gint ret = -E_USRGRPEXIST;

	g_static_mutex_lock(&self->list_glock);
	old = __nmp_mod_cu_find_group(self, grp->id);
	if (G_LIKELY(!old))
	{
		list_add2(&grp->list, &self->list_group);
		ret = 0;
	}
	g_static_mutex_unlock(&self->list_glock);

	return ret;
}


static __inline__ JpfUsrGroup *
nmp_mod_cu_get_group(JpfModCu *self, gint grp_id)
{
	JpfUsrGroup *grp;
	G_ASSERT(self != NULL);

	for (;;)
	{
		grp = nmp_mod_cu_find_and_get_group(self, grp_id);
		if (!grp)
		{
			grp = nmp_mod_cu_group_new(self, grp_id);
			if (G_UNLIKELY(!grp))
				return NULL;

			if (!nmp_mod_cu_add_group(self, grp))
				break;

			nmp_mod_cu_group_release(grp);
			continue;
		}

		break;
	}

	return grp;
}


static __inline__ JpfUsrGrpState
nmp_mod_cu_group_state(JpfUsrGroup *grp)
{
	JpfUsrGrpState state;
	G_ASSERT(grp != NULL);

	g_static_mutex_lock(&grp->lock);
	state = grp->state;
	if (state == STAT_GRP_NEW)
		grp->state = STAT_GRP_REQUEST;
	g_static_mutex_unlock(&grp->lock);

	return state;
}


static __inline__ void
nmp_mod_cu_set_group_state(JpfUsrGroup *grp, JpfUsrGrpState state)
{
	G_ASSERT(grp != NULL);

	g_static_mutex_lock(&grp->lock);
	grp->state = state;
	g_static_mutex_unlock(&grp->lock);
}


static __inline__ gint
nmp_mod_cu_need_wait_group(JpfUsrGroup *grp)
{
	JpfUsrGrpState state;
	G_ASSERT(grp != NULL);

	state = nmp_mod_cu_group_state(grp);
	return state == STAT_GRP_REQUEST;
}


static __inline__ gint
nmp_mod_cu_is_group_new(JpfUsrGroup *grp)
{
	G_ASSERT(grp != NULL);

	return nmp_mod_cu_group_state(grp) == STAT_GRP_NEW;
}


static __inline__ gint
nmp_mod_cu_is_group_complete(JpfUsrGroup *grp)
{
	JpfUsrGrpState state;
	G_ASSERT(grp != NULL);

	state = nmp_mod_cu_group_state(grp);
	BUG_ON(state != STAT_GRP_COMPLETED && state != STAT_GRP_FAILED);
	return state == STAT_GRP_COMPLETED;
}


static __inline__ void
nmp_mod_cu_add_group_user(JpfUsrGroup *grp, JpfUsr *user)
{
	G_ASSERT(grp != NULL && user != NULL);

	g_atomic_int_inc(&grp->ref_count);
	user->user_group = grp;
}


static __inline__ gint
nmp_mod_cu_get_group_errno(JpfUsrGroup *grp)
{
	G_ASSERT(grp != NULL);

	return grp->err_no;
}


static __inline__ void
nmp_mod_cu_group_wakeup(JpfUsrGroup *grp, gint err)
{
	JpfUsrGrpState state;
	G_ASSERT(grp != NULL);

	state = err ? STAT_GRP_FAILED : STAT_GRP_COMPLETED;
	jpf_wait_begin(grp->wait);
	nmp_mod_cu_set_group_state(grp, state);
	grp->err_no = err;
	jpf_wait_wakeup_all(grp->wait);
	jpf_wait_end(grp->wait);
}


static __inline__ void
nmp_mod_cu_wait_group(JpfUsrGroup *grp)
{
	jpf_wait_begin(grp->wait);

	while (nmp_mod_cu_need_wait_group(grp))
		jpf_wait_waiting(grp->wait);

	jpf_wait_end(grp->wait);
}


static __inline__ JpfUsr *
__nmp_mod_cu_find_user(JpfModCu *self, const gchar *name)
{
	LIST_HEAD *l;
	JpfUsr *user;

	list_for_each(l, &self->list_user)
	{
		user = list_entry(l, JpfUsr, list);
		if (!strcmp(user->user_name, name))
			return user;
	}

	return NULL;
}


static __inline__ JpfUsr *
nmp_mod_cu_find_and_get_user(JpfModCu *self, const gchar *name)
{
	JpfUsr *user;
	G_ASSERT(self != NULL && name != NULL);

	g_static_mutex_lock(&self->list_ulock);
	user = __nmp_mod_cu_find_user(self, name);
	if (user)
		g_atomic_int_inc(&user->ref_count);
	g_static_mutex_unlock(&self->list_ulock);

	return user;
}


static __inline__ void
nmp_mod_cu_put_user(JpfModCu *self, JpfUsr *user)
{
	JpfUsrGroup *grp = NULL;
	G_ASSERT(user != NULL);

	g_static_mutex_lock(&self->list_ulock);
	if (g_atomic_int_dec_and_test(&user->ref_count))
	{
		list_del(&user->list);
		grp = user->user_group;
		nmp_mod_cu_user_release(user);
	}
	g_static_mutex_unlock(&self->list_ulock);

	if (grp)
		nmp_mod_cu_put_group(self, grp);
}


static __inline__ gint
nmp_mod_cu_add_user(JpfModCu *self, JpfUsr *user)
{
	JpfUsr *old;
	gint ret = -E_USREXIST;
	G_ASSERT(self != NULL && user != NULL);

	g_static_mutex_lock(&self->list_ulock);
	old = __nmp_mod_cu_find_user(self, user->user_name);
	if (G_LIKELY(!old))
	{
		list_add2(&user->list, &self->list_user);
		ret = 0;
	}
	g_static_mutex_unlock(&self->list_ulock);

	return ret;
}


void
nmp_mod_cu_del_user_session(JpfCu *cu)
{
	JpfUsr *user;
	G_ASSERT(cu != NULL);

	user = cu->user;
	BUG_ON(!user);

	g_static_mutex_lock(&user->list_slock);
	list_del(&cu->list);
	--user->n_sessions;
	cu->user = NULL;
	g_static_mutex_unlock(&user->list_slock);

	BUG_ON(!user->user_group);
	BUG_ON(!user->user_group->owner);

	nmp_mod_cu_put_user(
		(JpfModCu*)user->user_group->owner,
		user
	);
}


static void
nmp_mod_cu_destroy(JpfGuestBase *obj, gpointer priv_data)
{
	JpfCu *cu;
	G_ASSERT(obj != NULL);

	cu = (JpfCu*)obj;
	nmp_mod_cu_del_user_session(cu);
}


void
nmp_mod_cu_del_user(JpfModCu *self, const gchar *name)
{
	JpfUsr *user = NULL;
	G_ASSERT(self != NULL && name != NULL);

    user = nmp_mod_cu_find_and_get_user(self, name);
    if(user)
	    nmp_mod_cu_put_user(self, user);
}

static __inline__ JpfUsr *
nmp_mod_cu_get_user(JpfModCu *self, const gchar *name)
{
	JpfUsr *user;
	G_ASSERT(self != NULL && name != NULL);

	for (;;)
	{
		user = nmp_mod_cu_find_and_get_user(self, name);
		if (!user)
		{
			user = nmp_mod_cu_user_new(name);
			if (G_UNLIKELY(!user))
				return NULL;

			if (!nmp_mod_cu_add_user(self, user))
				break;

			nmp_mod_cu_user_release(user);
			continue;
		}

		break;
	}

	return user;
}


gint
nmp_mod_cu_user_session_new(JpfModCu *self, JpfNetIO *io,
	const gchar *name, const gchar *passwd, gchar session[], gsize size)
{
	JpfUsr *user;
	JpfCu *cu = NULL;
	gint err = 0;
	JpfUsrGroup *grp = NULL;
	JpfUsrShareMode	share_mode;
	JpfID conflict;
	gchar session_id[MAX_NAME_LEN];

	G_ASSERT(self != NULL && io != NULL);

	user = nmp_mod_cu_get_user(self, name);
	if (G_UNLIKELY(!user))
	{
		err = -E_NOMEM;
		goto alloc_user_failed;
	}

	if (nmp_mod_cu_is_user_new(user))
	{
		err = nmp_mod_cu_get_user_info(self, user);
		if (G_UNLIKELY(err))
		{
			err = -E_USRINFO;
			goto query_user_info_failed;
		}

		nmp_mod_cu_set_user_state(user, STAT_USR_FILLED);
		grp = nmp_mod_cu_get_group(self, user->group_id);
		if (G_UNLIKELY(!grp))
		{
			err = -E_NOMEM;
			goto alloc_group_failed;
		}

		if (nmp_mod_cu_is_group_new(grp))
		{
			err = nmp_mod_cu_get_group_info(self, grp);
			if (G_UNLIKELY(err))
			{
				err = -E_UGRPINFO;
				goto query_grp_info_failed;
			}

			nmp_mod_cu_add_group_user(grp, user);
			nmp_mod_cu_group_wakeup(grp, err);
		}
		else
		{
			nmp_mod_cu_add_group_user(grp, user);
		}

		if (nmp_mod_cu_need_wait_group(grp))
			nmp_mod_cu_wait_group(grp);

		nmp_mod_cu_user_wakeup(user, nmp_mod_cu_get_group_errno(grp));
	}

	if (nmp_mod_cu_need_wait_user(user))
		nmp_mod_cu_wait_user(user);

	if (!nmp_mod_cu_is_user_complete(user))
	{
		err = nmp_mod_cu_get_user_errno(user);
		goto new_session_out;
	}

	share_mode = nmp_mod_cu_user_mode(user);
	switch (share_mode)
	{
	case SHARE_MODE_SHARED:
		if (strcmp(passwd, user->user_passwd) )
		{
			err = -E_PASSWD;
			goto new_session_out;
		}
		nmp_mod_cu_gen_session_id(session_id, MAX_NAME_LEN);
		cu = (JpfCu*)jpf_mods_guest_new(sizeof(JpfCu),
			session_id, nmp_mod_cu_destroy, NULL);
		if (G_UNLIKELY(!cu))
		{
			err = -E_NOMEM;
			goto new_session_out;
		}

		nmp_mod_cu_struct_init(cu);
		jpf_mods_guest_attach_io((JpfGuestBase*)cu, io);
		nmp_mod_cu_add_user_session(user, cu);

		err = jpf_mods_container_add_guest(self->container,
			(JpfGuestBase*)cu, &conflict);
		if (G_UNLIKELY(err))
			goto new_session_out;

		memset(session, 0, size);
		strncpy(session, session_id, size - 1);

		break;

	case SHARE_MODE_EXCLUSIVE:
		err = -E_NOTSUPPORT;
		break;

	case SHARE_MODE_GRAB:
		err = -E_NOTSUPPORT;
		break;

	default:
		FATAL_ERROR_EXIT;
	}

new_session_out:
	if (cu)
		jpf_mods_guest_unref((JpfGuestBase*)cu);

	if (grp)
		nmp_mod_cu_put_group(self, grp);

	if (user)
		nmp_mod_cu_put_user(self, user);

	return err;

query_grp_info_failed:
	nmp_mod_cu_group_wakeup(grp, err);

alloc_group_failed:
query_user_info_failed:
	nmp_mod_cu_user_wakeup(user, err);

alloc_user_failed:
	goto new_session_out;
}


static void
nmp_mod_cu_io_close(JpfModAccess *s_self, JpfNetIO *io, gint err)
{
	gint ret;
	JpfModCu *self;
	G_ASSERT(s_self != NULL);

	self = (JpfModCu*)s_self;

	ret = nmp_mod_container_del_io(self->container, io);
	if (G_UNLIKELY(!ret))
	{
		jpf_print(
			"<JpfModCu> unrecognized connection closed, err:%d.",
			err
		);
	}
}


static gint
nmp_mod_cu_io_init(JpfModAccess *s_self, JpfNetIO *io)
{
	gint err;
	JpfModCu *self;
	G_ASSERT(s_self != NULL);

	self = (JpfModCu*)s_self;

	err = nmp_mod_container_add_io(self->container, io);
	if (err)
	{
		jpf_error(
			"<JpfModCu> insert io to temp list err: %d!",
			err
		);
		return err;
	}

	jpf_net_unref_io(io);
	return 0;
}


gint
nmp_mod_cu_setup(NmpAppMod *am_self)
{
	gint err;
	JpfModAccess *ma_self;
	JpfModCu *self;
	struct sockaddr_in sin;
	G_ASSERT(am_self != NULL);

	self = (JpfModCu*)am_self;
	ma_self  = (JpfModAccess*)am_self;

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(JPFCMS_CU_PORT);
	sin.sin_addr.s_addr = INADDR_ANY;

	nmp_mod_acc_init_net(ma_self, &jxj_packet_proto, &jxj_xml_proto);

	self->listen_io = nmp_mod_acc_create_listen_io(
		ma_self, (struct sockaddr*)&sin, &err
	);
	if (!self->listen_io)
	{
		jpf_error("<JpfModPu> create listen io failed!");
		return err;
	}

	jpf_net_set_heavy_io_load(self->listen_io);
	nmp_app_mod_set_name(am_self, "MOD-CU");
 	nmp_mod_cu_register_msg_handler(self);

	return 0;
}


static void
nmp_mod_cu_init(JpfModCu *self)
{
	self->container = jpf_mods_container_new(
		self,
		jpf_get_sys_parm_int(SYS_PARM_WAIT_AFTER_CONNECTED)
	);
	if (G_UNLIKELY(!self->container))
	{
		jpf_error("<JpfModCu> alloc guest container failed!");
		FATAL_ERROR_EXIT;
	}

	INIT_LIST_HEAD(&self->list_group);
	g_static_mutex_init(&self->list_glock);
	INIT_LIST_HEAD(&self->list_user);
	g_static_mutex_init(&self->list_ulock);

    self->listen_io = NULL;

    self->n_groups = 0;
    self->n_users = 0;
    self->n_sessions = 0;
}


static void
nmp_mod_cu_class_init(JpfModCuClass *k_class)
{
	JpfModAccessClass *ma_class = (JpfModAccessClass*)k_class;
	NmpAppModClass *am_class = (NmpAppModClass*)k_class;

	ma_class->io_close	= nmp_mod_cu_io_close;
	ma_class->io_init	= nmp_mod_cu_io_init;
	am_class->setup_mod	= nmp_mod_cu_setup;
}


JpfCu *
nmp_mod_cu_get_cu(JpfModCu *self, JpfNetIO *io)
{
	G_ASSERT(self != NULL && io != NULL);

	return (JpfCu*)jpf_mods_container_get_guest(
		self->container, io);
}


JpfCu *
nmp_mod_cu_get_cu_2(JpfModCu *self, const gchar *sid)
{
	G_ASSERT(self != NULL && sid != NULL);

	return (JpfCu*)jpf_mods_container_get_guest_2(
		self->container, sid);
}


void
nmp_mod_cu_put_cu(JpfModCu *self, JpfCu *cu)
{
	G_ASSERT(self != NULL && cu != NULL);

	jpf_mods_container_put_guest(
		self->container, (JpfGuestBase*)cu);
}


gint
nmp_mod_cu_del_cu(JpfModCu *self, JpfCu *cu)
{
	G_ASSERT(self != NULL && cu != NULL);

	return jpf_mods_container_del_guest(
		self->container, (JpfGuestBase*)cu);
}


gint
nmp_mod_cu_del_cu_2(JpfModCu *self, JpfNetIO *io, JpfID *out)
{
	G_ASSERT(self != NULL && io != NULL && out != NULL);

	return jpf_mods_container_del_guest_2(self->container,
		io, out);
}


gint
nmp_mod_cu_sync_req(JpfModCu *self, NmpMsgID msg_id,
       gpointer req, gint req_size,  gpointer res, gint res_size)
{
	gint err = 0;
	JpfMsgErrCode *res_info;
	NmpSysMsg *msg;
	G_ASSERT(self != NULL);

	msg = jpf_sysmsg_new_2(msg_id, req, req_size, ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return -E_NOMEM;

       MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
	err = nmp_app_mod_sync_request((NmpAppMod*)self, &msg);
 	if (G_UNLIKELY(err))	/* send failed */
	{
		jpf_warning(
			"<JpfModCu> request cmd %d failed!", msg_id
		);
		jpf_sysmsg_destroy(msg);
		return err;
	}

	if (G_UNLIKELY(!msg))	/* sent, but no response */
	{
		jpf_warning(
			"<JpfModCu> request cmd %d timeout!", msg_id
		);
		return -E_TIMEOUT;
	}

	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (G_UNLIKELY(res))
		memcpy(res, MSG_GET_DATA(msg), res_size);

	err = RES_CODE(res_info);
	jpf_sysmsg_destroy(msg);

	return err;
}


gpointer
nmp_mod_cu_sync_req_2(JpfModCu *self, NmpMsgID msg_id,
       gpointer req, gint req_size, gint *res_size)
{
    gint err = 0;
    JpfMsgErrCode *res_info;
    gpointer res;
    NmpSysMsg *msg;
    guint len;

    G_ASSERT(self != NULL);

    msg = jpf_sysmsg_new_2(msg_id, req, req_size, ++msg_seq_generator);
    if (G_UNLIKELY(!msg))
    	return NULL;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    len = sizeof(JpfMsgErrCode);

    err = nmp_app_mod_sync_request((NmpAppMod*)self, &msg);
    if (G_UNLIKELY(err))	/* send failed */
    {
    	jpf_warning(
    		"<JpfModCu> request cmd %d failed!", msg_id
    	);

    	jpf_sysmsg_destroy(msg);

    	res_info = jpf_mem_kalloc(len);
    	if (res_info)
    	{
    		SET_CODE(res_info, err);
    		*res_size = len;
    	}

    	return res_info;
    }

    if (G_UNLIKELY(!msg))	/* sent, but no response */
    {
        jpf_warning(
    		"<JpfModCu> request cmd %d timeout!", msg_id
    	 );
    	 res_info = jpf_mem_kalloc(len);
    	 err = -E_TIMEOUT;
        if (res_info)
    	{
    		SET_CODE(res_info, err);
    		*res_size = len;
    	}
    	return res_info;
    }

    res = MSG_GET_DATA(msg);
    if (!res)
    {
    	jpf_sysmsg_destroy(msg);
    	return NULL;
    }

    res_info = jpf_mem_kalloc(MSG_DATA_SIZE(msg));
    if (G_UNLIKELY(!res_info))
    {
    	jpf_sysmsg_destroy(msg);
    	return NULL;
    }

    *res_size =  MSG_DATA_SIZE(msg);
    memcpy(res_info, res, *res_size);
    jpf_sysmsg_destroy(msg);

    return res_info;
}

//#define TEST_CODES
#ifdef TEST_CODES
static __inline__ void
__nmp_mod_cu_deliver_to_usr(JpfModCu *self, JpfUsr *usr, NmpSysMsg *msg)
{
	JpfCu *cu;
	LIST_HEAD *list;
	NmpSysMsg *msg_copy;
 	gint i;

	list_for_each(list, &usr->list_session)
	{
		cu = list_entry(list, JpfCu, list);
		for (i = 0; i <5000; i++)
		{
	    		msg_copy = jpf_sysmsg_copy_one(msg);
	    		if (!msg_copy)
	    		{
	    			jpf_error(
	    				"<JpfModCu> copy sys-msg failed while delivering msg to cu."
	    			);
	    			FATAL_ERROR_EXIT;
	    		}

	    		jpf_sysmsg_attach_io(msg_copy, IO_OF_GUEST(cu));
	    		nmp_app_obj_deliver_in((NmpAppObj*)self, msg_copy);
	    		if (i%100 == 0)
	    		 usleep(200*1000);
		}
	}
}

#else

static __inline__ void
__nmp_mod_cu_deliver_to_usr(JpfModCu *self, JpfUsr *usr, NmpSysMsg *msg)
{
	JpfCu *cu;
	LIST_HEAD *list;
	NmpSysMsg *msg_copy;

	list_for_each(list, &usr->list_session)
	{
		cu = list_entry(list, JpfCu, list);
		msg_copy = jpf_sysmsg_copy_one(msg);
		if (!msg_copy)
		{
			jpf_error(
				"<JpfModCu> copy sys-msg failed while delivering msg to cu."
			);
			FATAL_ERROR_EXIT;
		}

		jpf_sysmsg_attach_io(msg_copy, IO_OF_GUEST(cu));
		nmp_app_obj_deliver_in((NmpAppObj*)self, msg_copy);
	}
}

#endif

static __inline__ void
nmp_mod_cu_deliver_to_usr(JpfModCu *self, JpfUsr *usr, NmpSysMsg *msg)
{
	g_static_mutex_lock(&usr->list_slock);
	__nmp_mod_cu_deliver_to_usr(self, usr, msg);
	g_static_mutex_unlock(&usr->list_slock);
}


static __inline__ void
__nmp_mod_cu_deliver_msg(JpfModCu *self, const char *usr_name, NmpSysMsg *msg)
{
	JpfUsr *usr;
	LIST_HEAD *list;

	list_for_each(list, &self->list_user)
	{
		usr = list_entry(list, JpfUsr, list);
		if (!usr_name || !strcmp(usr_name, usr->user_name))
			nmp_mod_cu_deliver_to_usr(self, usr, msg);
	}
}


void
nmp_mod_cu_deliver_msg(JpfModCu *self, const char *usr_name, NmpSysMsg *msg)
{
	G_ASSERT(self != NULL && msg != NULL);

	g_static_mutex_lock(&self->list_ulock);
	__nmp_mod_cu_deliver_msg(self, usr_name, msg);
	g_static_mutex_unlock(&self->list_ulock);
}


static __inline__ void
__nmp_mod_cu_deliver_msg_2(JpfModCu *self, NmpSysMsg *msg)
{
	JpfUsr *usr;
	LIST_HEAD *list;

	list_for_each(list, &self->list_user)
	{
		usr = list_entry(list, JpfUsr, list);
		nmp_mod_cu_deliver_to_usr(self, usr, msg);
	}
}


void
nmp_mod_cu_deliver_msg_2(JpfModCu *self, NmpSysMsg *msg)
{
	G_ASSERT(self != NULL && msg != NULL);

	g_static_mutex_lock(&self->list_ulock);
	__nmp_mod_cu_deliver_msg_2(self, msg);
	g_static_mutex_unlock(&self->list_ulock);
}

static __inline__ void
__nmp_mod_cu_deliver_to_usr_offline(JpfModCu *self, JpfUsr *usr, NmpSysMsg *msg)
{
    JpfCu *cu;
    LIST_HEAD *list;
    NmpSysMsg *msg_copy;

    list_for_each(list, &usr->list_session)
    {
        cu = list_entry(list, JpfCu, list);
        msg_copy = jpf_sysmsg_copy_one(msg);
        if (!msg_copy)
        {
            jpf_error(
            	"<JpfModCu> copy sys-msg failed while delivering msg to cu."
            );
            FATAL_ERROR_EXIT;
        }

        jpf_sysmsg_attach_io(msg_copy, IO_OF_GUEST(cu));
        nmp_app_obj_deliver_in((NmpAppObj*)self, msg_copy);
        nmp_mod_acc_release_io((JpfModAccess*)self,  IO_OF_GUEST(cu));
        nmp_mod_container_del_io(self->container,  IO_OF_GUEST(cu));
    }
}


static __inline__ void
nmp_mod_cu_deliver_to_usr_offline(JpfModCu *self, JpfUsr *usr, NmpSysMsg *msg)
{
	g_static_mutex_lock(&usr->list_slock);
	__nmp_mod_cu_deliver_to_usr_offline(self, usr, msg);
	g_static_mutex_unlock(&usr->list_slock);
}

static __inline__ void
__nmp_mod_cu_force_usr_offline(JpfModCu *self, const char *usr_name, NmpSysMsg *msg)
{
	JpfUsr *usr;
	LIST_HEAD *list;

	list_for_each(list, &self->list_user)
	{
		usr = list_entry(list, JpfUsr, list);
		if (!usr_name || !strcmp(usr_name, usr->user_name))
		{
			nmp_mod_cu_deliver_to_usr_offline(self, usr, msg);
		}
	}
}


void
nmp_mod_cu_force_usr_offline(JpfModCu *self, const char *usr_name, NmpSysMsg *msg)
{
	G_ASSERT(self != NULL && msg != NULL);

	g_static_mutex_lock(&self->list_ulock);
	__nmp_mod_cu_force_usr_offline(self, usr_name, msg);
	g_static_mutex_unlock(&self->list_ulock);
}


static __inline__ void
__nmp_mod_cu_force_usr_offline_by_group(JpfModCu *self,
    gint group_id, NmpSysMsg *msg)
{
	JpfUsr *usr;
	LIST_HEAD *list;

	list_for_each(list, &self->list_user)
	{
		usr = list_entry(list, JpfUsr, list);
		if (group_id == usr->group_id)
		{
			nmp_mod_cu_deliver_to_usr_offline(self, usr, msg);
		}
	}
}


void
nmp_mod_cu_force_usr_offline_by_group(JpfModCu *self,
    gint group_id, NmpSysMsg *msg)
{
	G_ASSERT(self != NULL && msg != NULL);

	g_static_mutex_lock(&self->list_ulock);
	__nmp_mod_cu_force_usr_offline_by_group(self, group_id, msg);
	g_static_mutex_unlock(&self->list_ulock);
}


static __inline__ void
__nmp_mod_cu_all_user(JpfModCu *self, NmpSysMsg *msg)
{
	LIST_HEAD *l;
	JpfUsr *user;

	list_for_each(l, &self->list_user)
	{
		user = list_entry(l, JpfUsr, list);
		{
            nmp_mod_cu_deliver_msg(self, user->user_name, msg);
		}

	}
}


void
nmp_mod_cu_broadcast_to_all_user(JpfModCu *self, NmpSysMsg *msg)
{
	 __nmp_mod_cu_all_user(self, msg);
}


void
nmp_mod_cu_broadcast_generic_msg(JpfModCu *self, gint id, gchar *parm1,
	gchar *parm2, gchar *parm3, gchar *content)
{
    NmpSysMsg *broadcast_msg;
    int seq;
    JpfNotifyMessage general_msg;

    memset(&general_msg, 0, sizeof(general_msg));
    general_msg.msg_id = id;
    if (parm1)
	strncpy(general_msg.param1, parm1, GENERAL_MSG_PARM_LEN - 1);
    if (parm2)
	strncpy(general_msg.param2, parm2, GENERAL_MSG_PARM_LEN - 1);
    if (parm3)
	strncpy(general_msg.param3, parm3, GENERAL_MSG_PARM_LEN - 1);
    if (content)
	strncpy(general_msg.content, content, DESCRIPTION_INFO_LEN - 1);

    seq = time(NULL);
    broadcast_msg = jpf_sysmsg_new_2(MESSAGE_BROADCAST_GENERAL_MSG, &general_msg, sizeof(general_msg), seq);
    if (broadcast_msg)
    {
        nmp_mod_cu_broadcast_to_all_user(self, broadcast_msg);
	 jpf_sysmsg_destroy(broadcast_msg);
    }
}


static __inline__ void
__nmp_mod_cu_update_usr_group_permissions(JpfModCu *self,
    gint group_id, gint permission, gint rank)
{
	JpfUsrGroup *usr_group;
	LIST_HEAD *list;

	list_for_each(list, &self->list_group)
	{
		usr_group = list_entry(list, JpfUsrGroup, list);
		if (group_id == usr_group->id)
		{
			usr_group->rank = rank;
			usr_group->permissions = permission;
		}
	}
}


void
nmp_mod_cu_update_usr_group_permissions(JpfModCu *self,
    gint group_id, gint permission, gint rank)
{
	G_ASSERT(self != NULL);

	g_static_mutex_lock(&self->list_glock);
	__nmp_mod_cu_update_usr_group_permissions(self, group_id, permission, rank);
	g_static_mutex_unlock(&self->list_glock);
}

//:~ End
