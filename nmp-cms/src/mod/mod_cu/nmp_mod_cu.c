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


G_DEFINE_TYPE(NmpModCu, nmp_mod_cu, NMP_TYPE_MODACCESS);

void
nmp_mod_cu_register_msg_handler(NmpModCu *self);

GStaticMutex gen_sid_lock = G_STATIC_MUTEX_INIT;
//:TODO
//static guint msg_seq_generator = 0;

static __inline__ void
nmp_mod_cu_cal_ttl(NmpCu *cu)
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
		nmp_get_local_domain_id(),
		(++sid_no) % 1000000
	);

	g_static_mutex_unlock(&gen_sid_lock);
}


static __inline__ void
nmp_mod_cu_struct_init(NmpCu *cu)
{
	cu->hb_freq = HB_FREQ_DEFAULT;
	nmp_mod_cu_cal_ttl(cu);
	cu->user = NULL;
}


static __inline__ void
nmp_mod_cu_group_ref(NmpUsrGroup *grp)
{
	G_ASSERT(grp != NULL && g_atomic_int_get(&grp->ref_count) > 0);

	g_atomic_int_inc(&grp->ref_count);
}


static __inline__ void
nmp_mod_cu_group_release(NmpUsrGroup *grp)
{
	G_ASSERT(grp != NULL);

	nmp_wait_free(grp->wait);
	g_free(grp);
}


static __inline__ NmpUsr *
nmp_mod_cu_user_new(const gchar *name)
{
	NmpUsr *user;

	user = g_new0(NmpUsr, 1);
	if (G_UNLIKELY(!user))
		return NULL;

	user->wait = nmp_wait_new();
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
nmp_mod_cu_user_release(NmpUsr *user)
{
	G_ASSERT(user != NULL);

	nmp_wait_free(user->wait);
	g_free(user);
}


void
nmp_mod_cu_add_user_info(NmpUsr *user, gchar *passwd, gint grp_id)
{
	G_ASSERT(user != NULL && passwd != NULL);

	g_static_mutex_lock(&user->lock);
	strncpy(user->user_passwd, passwd, MAX_PASSWD_LEN - 1);
	user->group_id = grp_id;
	user->user_state = STAT_USR_FILLED;
	g_static_mutex_unlock(&user->lock);
}


static __inline__ NmpUsrState
nmp_mod_cu_user_state(NmpUsr *user)
{
	NmpUsrState state;

	g_static_mutex_lock(&user->lock);
	state = user->user_state;
	if (state == STAT_USR_NEW)
		user->user_state = STAT_USR_REQUEST;
	g_static_mutex_unlock(&user->lock);

	return state;
}


static __inline__ void
nmp_mod_cu_set_user_state(NmpUsr *user, NmpUsrState state)
{
	g_static_mutex_lock(&user->lock);
	user->user_state = state;
	g_static_mutex_unlock(&user->lock);
}


NmpUsrShareMode
nmp_mod_cu_user_mode(NmpUsr *user)
{
	NmpUsrShareMode mode;

	g_static_mutex_lock(&user->lock);
	mode = user->share_mode;
	g_static_mutex_unlock(&user->lock);

	return mode;
}


static __inline__ gint
nmp_mod_cu_set_user_errno(NmpUsr *user, gint err)
{
	G_ASSERT(user != NULL);

	user->err_no = err;
}


static __inline__ gint
nmp_mod_cu_get_user_errno(NmpUsr *user)
{
	G_ASSERT(user != NULL);

	return user->err_no;
}


static __inline__ gint
nmp_mod_cu_is_user_new(NmpUsr *user)
{
	return nmp_mod_cu_user_state(user) == STAT_USR_NEW;
}


static __inline__ gint
nmp_mod_cu_user_failed(NmpUsrState state)
{
	return state == STAT_USR_FAILED;
}


static __inline__ gint
nmp_mod_cu_need_wait_user(NmpUsr *user)
{
	NmpUsrState state;
	G_ASSERT(user != NULL);

	state = nmp_mod_cu_user_state(user);
	return state != STAT_USR_NEW &&
		state != STAT_USR_COMPLETED &&
		state != STAT_USR_FAILED;
}


static __inline__ gint
nmp_mod_cu_is_user_complete(NmpUsr *user)
{
	NmpUsrState state;
	G_ASSERT(user != NULL);

	state = nmp_mod_cu_user_state(user);
	BUG_ON(state != STAT_USR_COMPLETED && state != STAT_USR_FAILED);
	return state == STAT_USR_COMPLETED;
}


static __inline__ void
nmp_mod_cu_user_wakeup(NmpUsr *user, gint err)
{
	NmpUsrState state;
	G_ASSERT(user != NULL);

	state = err ? STAT_USR_FAILED : STAT_USR_COMPLETED;
	nmp_wait_begin(user->wait);
	nmp_mod_cu_set_user_state(user, state);
	user->err_no = err;
	nmp_wait_wakeup_all(user->wait);
	nmp_wait_end(user->wait);
}


static __inline__ void
nmp_mod_cu_wait_user(NmpUsr *user)
{
	nmp_wait_begin(user->wait);

	while (nmp_mod_cu_need_wait_user(user))
		nmp_wait_waiting(user->wait);

	nmp_wait_end(user->wait);
}


void
nmp_mod_cu_add_user_session(NmpUsr *user, NmpCu *cu)
{
	G_ASSERT(user != NULL && cu != NULL);

	g_static_mutex_lock(&user->list_slock);
	list_add2(&cu->list, &user->list_session);
	++user->n_sessions;
	g_atomic_int_inc(&user->ref_count);
	cu->user = user;
	g_static_mutex_unlock(&user->list_slock);
}


static __inline__ NmpUsrGroup *
nmp_mod_cu_group_new(NmpModCu *mod, gint grp_id)
{
	NmpUsrGroup *grp;

	grp = g_new0(NmpUsrGroup, 1);
	if (G_UNLIKELY(!grp))
		return NULL;

	grp->wait = nmp_wait_new();
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
nmp_mod_cu_add_group_info(NmpUsrGroup *grp, gint rank, guint perm)
{
	G_ASSERT(grp != NULL);

	grp->rank = rank;
	grp->permissions = perm;
}


static __inline__ NmpUsrGroup *
__nmp_mod_cu_find_group(NmpModCu *self, gint grp_id)
{
	LIST_HEAD *l;
	NmpUsrGroup *grp;

	list_for_each(l, &self->list_group)
	{
		grp = list_entry(l, NmpUsrGroup, list);
		if (grp->id == grp_id)
			return grp;
	}

	return  NULL;
}


static __inline__ NmpUsrGroup *
nmp_mod_cu_find_and_get_group(NmpModCu *self, gint grp_id)
{
	NmpUsrGroup *grp;
	G_ASSERT(self != NULL);

	g_static_mutex_lock(&self->list_glock);
	grp = __nmp_mod_cu_find_group(self, grp_id);
	if (G_LIKELY(grp))
		g_atomic_int_inc(&grp->ref_count);
	g_static_mutex_unlock(&self->list_glock);

	return grp;
}


static __inline__ void
nmp_mod_cu_put_group(NmpModCu *self, NmpUsrGroup *grp)
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
nmp_mod_cu_add_group(NmpModCu *self, NmpUsrGroup *grp)
{
	NmpUsrGroup *old;
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


static __inline__ NmpUsrGroup *
nmp_mod_cu_get_group(NmpModCu *self, gint grp_id)
{
	NmpUsrGroup *grp;
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


static __inline__ NmpUsrGrpState
nmp_mod_cu_group_state(NmpUsrGroup *grp)
{
	NmpUsrGrpState state;
	G_ASSERT(grp != NULL);

	g_static_mutex_lock(&grp->lock);
	state = grp->state;
	if (state == STAT_GRP_NEW)
		grp->state = STAT_GRP_REQUEST;
	g_static_mutex_unlock(&grp->lock);

	return state;
}


static __inline__ void
nmp_mod_cu_set_group_state(NmpUsrGroup *grp, NmpUsrGrpState state)
{
	G_ASSERT(grp != NULL);

	g_static_mutex_lock(&grp->lock);
	grp->state = state;
	g_static_mutex_unlock(&grp->lock);
}


static __inline__ gint
nmp_mod_cu_need_wait_group(NmpUsrGroup *grp)
{
	NmpUsrGrpState state;
	G_ASSERT(grp != NULL);

	state = nmp_mod_cu_group_state(grp);
	return state == STAT_GRP_REQUEST;
}


static __inline__ gint
nmp_mod_cu_is_group_new(NmpUsrGroup *grp)
{
	G_ASSERT(grp != NULL);

	return nmp_mod_cu_group_state(grp) == STAT_GRP_NEW;
}


static __inline__ gint
nmp_mod_cu_is_group_complete(NmpUsrGroup *grp)
{
	NmpUsrGrpState state;
	G_ASSERT(grp != NULL);

	state = nmp_mod_cu_group_state(grp);
	BUG_ON(state != STAT_GRP_COMPLETED && state != STAT_GRP_FAILED);
	return state == STAT_GRP_COMPLETED;
}


static __inline__ void
nmp_mod_cu_add_group_user(NmpUsrGroup *grp, NmpUsr *user)
{
	G_ASSERT(grp != NULL && user != NULL);

	g_atomic_int_inc(&grp->ref_count);
	user->user_group = grp;
}


static __inline__ gint
nmp_mod_cu_get_group_errno(NmpUsrGroup *grp)
{
	G_ASSERT(grp != NULL);

	return grp->err_no;
}


static __inline__ void
nmp_mod_cu_group_wakeup(NmpUsrGroup *grp, gint err)
{
	NmpUsrGrpState state;
	G_ASSERT(grp != NULL);

	state = err ? STAT_GRP_FAILED : STAT_GRP_COMPLETED;
	nmp_wait_begin(grp->wait);
	nmp_mod_cu_set_group_state(grp, state);
	grp->err_no = err;
	nmp_wait_wakeup_all(grp->wait);
	nmp_wait_end(grp->wait);
}


static __inline__ void
nmp_mod_cu_wait_group(NmpUsrGroup *grp)
{
	nmp_wait_begin(grp->wait);

	while (nmp_mod_cu_need_wait_group(grp))
		nmp_wait_waiting(grp->wait);

	nmp_wait_end(grp->wait);
}


static __inline__ NmpUsr *
__nmp_mod_cu_find_user(NmpModCu *self, const gchar *name)
{
	LIST_HEAD *l;
	NmpUsr *user;

	list_for_each(l, &self->list_user)
	{
		user = list_entry(l, NmpUsr, list);
		if (!strcmp(user->user_name, name))
			return user;
	}

	return NULL;
}


static __inline__ NmpUsr *
nmp_mod_cu_find_and_get_user(NmpModCu *self, const gchar *name)
{
	NmpUsr *user;
	G_ASSERT(self != NULL && name != NULL);

	g_static_mutex_lock(&self->list_ulock);
	user = __nmp_mod_cu_find_user(self, name);
	if (user)
		g_atomic_int_inc(&user->ref_count);
	g_static_mutex_unlock(&self->list_ulock);

	return user;
}


static __inline__ void
nmp_mod_cu_put_user(NmpModCu *self, NmpUsr *user)
{
	NmpUsrGroup *grp = NULL;
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
nmp_mod_cu_add_user(NmpModCu *self, NmpUsr *user)
{
	NmpUsr *old;
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
nmp_mod_cu_del_user_session(NmpCu *cu)
{
	NmpUsr *user;
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
		(NmpModCu*)user->user_group->owner,
		user
	);
}


static void
nmp_mod_cu_destroy(NmpGuestBase *obj, gpointer priv_data)
{
	NmpCu *cu;
	G_ASSERT(obj != NULL);

	cu = (NmpCu*)obj;
	nmp_mod_cu_del_user_session(cu);
}


void
nmp_mod_cu_del_user(NmpModCu *self, const gchar *name)
{
	NmpUsr *user = NULL;
	G_ASSERT(self != NULL && name != NULL);

    user = nmp_mod_cu_find_and_get_user(self, name);
    if(user)
	    nmp_mod_cu_put_user(self, user);
}

static __inline__ NmpUsr *
nmp_mod_cu_get_user(NmpModCu *self, const gchar *name)
{
	NmpUsr *user;
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
nmp_mod_cu_user_session_new(NmpModCu *self, NmpNetIO *io,
	const gchar *name, const gchar *passwd, gchar session[], gsize size)
{
	NmpUsr *user;
	NmpCu *cu = NULL;
	gint err = 0;
	NmpUsrGroup *grp = NULL;
	NmpUsrShareMode	share_mode;
	NmpID conflict;
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
		cu = (NmpCu*)nmp_mods_guest_new(sizeof(NmpCu),
			session_id, nmp_mod_cu_destroy, NULL);
		if (G_UNLIKELY(!cu))
		{
			err = -E_NOMEM;
			goto new_session_out;
		}

		nmp_mod_cu_struct_init(cu);
		nmp_mods_guest_attach_io((NmpGuestBase*)cu, io);
		nmp_mod_cu_add_user_session(user, cu);

		err = nmp_mods_container_add_guest(self->container,
			(NmpGuestBase*)cu, &conflict);
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
		nmp_mods_guest_unref((NmpGuestBase*)cu);

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
nmp_mod_cu_io_close(NmpModAccess *s_self, NmpNetIO *io, gint err)
{
	gint ret;
	NmpModCu *self;
	G_ASSERT(s_self != NULL);

	self = (NmpModCu*)s_self;

	ret = nmp_mod_container_del_io(self->container, io);
	if (G_UNLIKELY(!ret))
	{
		nmp_print(
			"<NmpModCu> unrecognized connection closed, err:%d.",
			err
		);
	}
}


static gint
nmp_mod_cu_io_init(NmpModAccess *s_self, NmpNetIO *io)
{
	gint err;
	NmpModCu *self;
	G_ASSERT(s_self != NULL);

	self = (NmpModCu*)s_self;

	err = nmp_mod_container_add_io(self->container, io);
	if (err)
	{
		nmp_error(
			"<NmpModCu> insert io to temp list err: %d!",
			err
		);
		return err;
	}

	nmp_net_unref_io(io);
	return 0;
}


gint
nmp_mod_cu_setup(NmpAppMod *am_self)
{
	gint err;
	NmpModAccess *ma_self;
	NmpModCu *self;
	struct sockaddr_in sin;
	G_ASSERT(am_self != NULL);

	self = (NmpModCu*)am_self;
	ma_self  = (NmpModAccess*)am_self;

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(JPFCMS_CU_PORT);
	sin.sin_addr.s_addr = INADDR_ANY;

	nmp_mod_acc_init_net(ma_self, &nmp_packet_proto, &nmp_xml_proto);

	self->listen_io = nmp_mod_acc_create_listen_io(
		ma_self, (struct sockaddr*)&sin, &err
	);
	if (!self->listen_io)
	{
		nmp_error("<NmpModPu> create listen io failed!");
		return err;
	}

	nmp_net_set_heavy_io_load(self->listen_io);
	nmp_app_mod_set_name(am_self, "MOD-CU");
 	nmp_mod_cu_register_msg_handler(self);

	return 0;
}


static void
nmp_mod_cu_init(NmpModCu *self)
{
	self->container = nmp_mods_container_new(
		self,
		nmp_get_sys_parm_int(SYS_PARM_WAIT_AFTER_CONNECTED)
	);
	if (G_UNLIKELY(!self->container))
	{
		nmp_error("<NmpModCu> alloc guest container failed!");
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
nmp_mod_cu_class_init(NmpModCuClass *k_class)
{
	NmpModAccessClass *ma_class = (NmpModAccessClass*)k_class;
	NmpAppModClass *am_class = (NmpAppModClass*)k_class;

	ma_class->io_close	= nmp_mod_cu_io_close;
	ma_class->io_init	= nmp_mod_cu_io_init;
	am_class->setup_mod	= nmp_mod_cu_setup;
}


NmpCu *
nmp_mod_cu_get_cu(NmpModCu *self, NmpNetIO *io)
{
	G_ASSERT(self != NULL && io != NULL);

	return (NmpCu*)nmp_mods_container_get_guest(
		self->container, io);
}


NmpCu *
nmp_mod_cu_get_cu_2(NmpModCu *self, const gchar *sid)
{
	G_ASSERT(self != NULL && sid != NULL);

	return (NmpCu*)nmp_mods_container_get_guest_2(
		self->container, sid);
}


void
nmp_mod_cu_put_cu(NmpModCu *self, NmpCu *cu)
{
	G_ASSERT(self != NULL && cu != NULL);

	nmp_mods_container_put_guest(
		self->container, (NmpGuestBase*)cu);
}


gint
nmp_mod_cu_del_cu(NmpModCu *self, NmpCu *cu)
{
	G_ASSERT(self != NULL && cu != NULL);

	return nmp_mods_container_del_guest(
		self->container, (NmpGuestBase*)cu);
}


gint
nmp_mod_cu_del_cu_2(NmpModCu *self, NmpNetIO *io, NmpID *out)
{
	G_ASSERT(self != NULL && io != NULL && out != NULL);

	return nmp_mods_container_del_guest_2(self->container,
		io, out);
}


gint
nmp_mod_cu_sync_req(NmpModCu *self, NmpMsgID msg_id,
       gpointer req, gint req_size,  gpointer res, gint res_size)
{
	gint err = 0;
	NmpMsgErrCode *res_info;
	NmpSysMsg *msg;
	G_ASSERT(self != NULL);

	msg = nmp_sysmsg_new_2(msg_id, req, req_size, ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return -E_NOMEM;

       MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
	err = nmp_app_mod_sync_request((NmpAppMod*)self, &msg);
 	if (G_UNLIKELY(err))	/* send failed */
	{
		nmp_warning(
			"<NmpModCu> request cmd %d failed!", msg_id
		);
		nmp_sysmsg_destroy(msg);
		return err;
	}

	if (G_UNLIKELY(!msg))	/* sent, but no response */
	{
		nmp_warning(
			"<NmpModCu> request cmd %d timeout!", msg_id
		);
		return -E_TIMEOUT;
	}

	res_info = MSG_GET_DATA(msg);
	BUG_ON(!res_info);

	if (G_UNLIKELY(res))
		memcpy(res, MSG_GET_DATA(msg), res_size);

	err = RES_CODE(res_info);
	nmp_sysmsg_destroy(msg);

	return err;
}


gpointer
nmp_mod_cu_sync_req_2(NmpModCu *self, NmpMsgID msg_id,
       gpointer req, gint req_size, gint *res_size)
{
    gint err = 0;
    NmpMsgErrCode *res_info;
    gpointer res;
    NmpSysMsg *msg;
    guint len;

    G_ASSERT(self != NULL);

    msg = nmp_sysmsg_new_2(msg_id, req, req_size, ++msg_seq_generator);
    if (G_UNLIKELY(!msg))
    	return NULL;

    MSG_SET_DSTPOS(msg, BUSSLOT_POS_DBS);
    len = sizeof(NmpMsgErrCode);

    err = nmp_app_mod_sync_request((NmpAppMod*)self, &msg);
    if (G_UNLIKELY(err))	/* send failed */
    {
    	nmp_warning(
    		"<NmpModCu> request cmd %d failed!", msg_id
    	);

    	nmp_sysmsg_destroy(msg);

    	res_info = nmp_mem_kalloc(len);
    	if (res_info)
    	{
    		SET_CODE(res_info, err);
    		*res_size = len;
    	}

    	return res_info;
    }

    if (G_UNLIKELY(!msg))	/* sent, but no response */
    {
        nmp_warning(
    		"<NmpModCu> request cmd %d timeout!", msg_id
    	 );
    	 res_info = nmp_mem_kalloc(len);
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
    	nmp_sysmsg_destroy(msg);
    	return NULL;
    }

    res_info = nmp_mem_kalloc(MSG_DATA_SIZE(msg));
    if (G_UNLIKELY(!res_info))
    {
    	nmp_sysmsg_destroy(msg);
    	return NULL;
    }

    *res_size =  MSG_DATA_SIZE(msg);
    memcpy(res_info, res, *res_size);
    nmp_sysmsg_destroy(msg);

    return res_info;
}

//#define TEST_CODES
#ifdef TEST_CODES
static __inline__ void
__nmp_mod_cu_deliver_to_usr(NmpModCu *self, NmpUsr *usr, NmpSysMsg *msg)
{
	NmpCu *cu;
	LIST_HEAD *list;
	NmpSysMsg *msg_copy;
 	gint i;

	list_for_each(list, &usr->list_session)
	{
		cu = list_entry(list, NmpCu, list);
		for (i = 0; i <5000; i++)
		{
	    		msg_copy = nmp_sysmsg_copy_one(msg);
	    		if (!msg_copy)
	    		{
	    			nmp_error(
	    				"<NmpModCu> copy sys-msg failed while delivering msg to cu."
	    			);
	    			FATAL_ERROR_EXIT;
	    		}

	    		nmp_sysmsg_attach_io(msg_copy, IO_OF_GUEST(cu));
	    		nmp_app_obj_deliver_in((NmpAppObj*)self, msg_copy);
	    		if (i%100 == 0)
	    		 usleep(200*1000);
		}
	}
}

#else

static __inline__ void
__nmp_mod_cu_deliver_to_usr(NmpModCu *self, NmpUsr *usr, NmpSysMsg *msg)
{
	NmpCu *cu;
	LIST_HEAD *list;
	NmpSysMsg *msg_copy;

	list_for_each(list, &usr->list_session)
	{
		cu = list_entry(list, NmpCu, list);
		msg_copy = nmp_sysmsg_copy_one(msg);
		if (!msg_copy)
		{
			nmp_error(
				"<NmpModCu> copy sys-msg failed while delivering msg to cu."
			);
			FATAL_ERROR_EXIT;
		}

		nmp_sysmsg_attach_io(msg_copy, IO_OF_GUEST(cu));
		nmp_app_obj_deliver_in((NmpAppObj*)self, msg_copy);
	}
}

#endif

static __inline__ void
nmp_mod_cu_deliver_to_usr(NmpModCu *self, NmpUsr *usr, NmpSysMsg *msg)
{
	g_static_mutex_lock(&usr->list_slock);
	__nmp_mod_cu_deliver_to_usr(self, usr, msg);
	g_static_mutex_unlock(&usr->list_slock);
}


static __inline__ void
__nmp_mod_cu_deliver_msg(NmpModCu *self, const char *usr_name, NmpSysMsg *msg)
{
	NmpUsr *usr;
	LIST_HEAD *list;

	list_for_each(list, &self->list_user)
	{
		usr = list_entry(list, NmpUsr, list);
		if (!usr_name || !strcmp(usr_name, usr->user_name))
			nmp_mod_cu_deliver_to_usr(self, usr, msg);
	}
}


void
nmp_mod_cu_deliver_msg(NmpModCu *self, const char *usr_name, NmpSysMsg *msg)
{
	G_ASSERT(self != NULL && msg != NULL);

	g_static_mutex_lock(&self->list_ulock);
	__nmp_mod_cu_deliver_msg(self, usr_name, msg);
	g_static_mutex_unlock(&self->list_ulock);
}


static __inline__ void
__nmp_mod_cu_deliver_msg_2(NmpModCu *self, NmpSysMsg *msg)
{
	NmpUsr *usr;
	LIST_HEAD *list;

	list_for_each(list, &self->list_user)
	{
		usr = list_entry(list, NmpUsr, list);
		nmp_mod_cu_deliver_to_usr(self, usr, msg);
	}
}


void
nmp_mod_cu_deliver_msg_2(NmpModCu *self, NmpSysMsg *msg)
{
	G_ASSERT(self != NULL && msg != NULL);

	g_static_mutex_lock(&self->list_ulock);
	__nmp_mod_cu_deliver_msg_2(self, msg);
	g_static_mutex_unlock(&self->list_ulock);
}

static __inline__ void
__nmp_mod_cu_deliver_to_usr_offline(NmpModCu *self, NmpUsr *usr, NmpSysMsg *msg)
{
    NmpCu *cu;
    LIST_HEAD *list;
    NmpSysMsg *msg_copy;

    list_for_each(list, &usr->list_session)
    {
        cu = list_entry(list, NmpCu, list);
        msg_copy = nmp_sysmsg_copy_one(msg);
        if (!msg_copy)
        {
            nmp_error(
            	"<NmpModCu> copy sys-msg failed while delivering msg to cu."
            );
            FATAL_ERROR_EXIT;
        }

        nmp_sysmsg_attach_io(msg_copy, IO_OF_GUEST(cu));
        nmp_app_obj_deliver_in((NmpAppObj*)self, msg_copy);
        nmp_mod_acc_release_io((NmpModAccess*)self,  IO_OF_GUEST(cu));
        nmp_mod_container_del_io(self->container,  IO_OF_GUEST(cu));
    }
}


static __inline__ void
nmp_mod_cu_deliver_to_usr_offline(NmpModCu *self, NmpUsr *usr, NmpSysMsg *msg)
{
	g_static_mutex_lock(&usr->list_slock);
	__nmp_mod_cu_deliver_to_usr_offline(self, usr, msg);
	g_static_mutex_unlock(&usr->list_slock);
}

static __inline__ void
__nmp_mod_cu_force_usr_offline(NmpModCu *self, const char *usr_name, NmpSysMsg *msg)
{
	NmpUsr *usr;
	LIST_HEAD *list;

	list_for_each(list, &self->list_user)
	{
		usr = list_entry(list, NmpUsr, list);
		if (!usr_name || !strcmp(usr_name, usr->user_name))
		{
			nmp_mod_cu_deliver_to_usr_offline(self, usr, msg);
		}
	}
}


void
nmp_mod_cu_force_usr_offline(NmpModCu *self, const char *usr_name, NmpSysMsg *msg)
{
	G_ASSERT(self != NULL && msg != NULL);

	g_static_mutex_lock(&self->list_ulock);
	__nmp_mod_cu_force_usr_offline(self, usr_name, msg);
	g_static_mutex_unlock(&self->list_ulock);
}


static __inline__ void
__nmp_mod_cu_force_usr_offline_by_group(NmpModCu *self,
    gint group_id, NmpSysMsg *msg)
{
	NmpUsr *usr;
	LIST_HEAD *list;

	list_for_each(list, &self->list_user)
	{
		usr = list_entry(list, NmpUsr, list);
		if (group_id == usr->group_id)
		{
			nmp_mod_cu_deliver_to_usr_offline(self, usr, msg);
		}
	}
}


void
nmp_mod_cu_force_usr_offline_by_group(NmpModCu *self,
    gint group_id, NmpSysMsg *msg)
{
	G_ASSERT(self != NULL && msg != NULL);

	g_static_mutex_lock(&self->list_ulock);
	__nmp_mod_cu_force_usr_offline_by_group(self, group_id, msg);
	g_static_mutex_unlock(&self->list_ulock);
}


static __inline__ void
__nmp_mod_cu_all_user(NmpModCu *self, NmpSysMsg *msg)
{
	LIST_HEAD *l;
	NmpUsr *user;

	list_for_each(l, &self->list_user)
	{
		user = list_entry(l, NmpUsr, list);
		{
            nmp_mod_cu_deliver_msg(self, user->user_name, msg);
		}

	}
}


void
nmp_mod_cu_broadcast_to_all_user(NmpModCu *self, NmpSysMsg *msg)
{
	 __nmp_mod_cu_all_user(self, msg);
}


void
nmp_mod_cu_broadcast_generic_msg(NmpModCu *self, gint id, gchar *parm1,
	gchar *parm2, gchar *parm3, gchar *content)
{
    NmpSysMsg *broadcast_msg;
    int seq;
    NmpNotifyMessage general_msg;

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
    broadcast_msg = nmp_sysmsg_new_2(MESSAGE_BROADCAST_GENERAL_MSG, &general_msg, sizeof(general_msg), seq);
    if (broadcast_msg)
    {
        nmp_mod_cu_broadcast_to_all_user(self, broadcast_msg);
	 nmp_sysmsg_destroy(broadcast_msg);
    }
}


static __inline__ void
__nmp_mod_cu_update_usr_group_permissions(NmpModCu *self,
    gint group_id, gint permission, gint rank)
{
	NmpUsrGroup *usr_group;
	LIST_HEAD *list;

	list_for_each(list, &self->list_group)
	{
		usr_group = list_entry(list, NmpUsrGroup, list);
		if (group_id == usr_group->id)
		{
			usr_group->rank = rank;
			usr_group->permissions = permission;
		}
	}
}


void
nmp_mod_cu_update_usr_group_permissions(NmpModCu *self,
    gint group_id, gint permission, gint rank)
{
	G_ASSERT(self != NULL);

	g_static_mutex_lock(&self->list_glock);
	__nmp_mod_cu_update_usr_group_permissions(self, group_id, permission, rank);
	g_static_mutex_unlock(&self->list_glock);
}

//:~ End
