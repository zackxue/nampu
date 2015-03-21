/*
 * nmp_mods.c
 *
 * This file implements data structures shared by all mods.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include "nmp_mods.h"
#include "nmp_debug.h"
#include "nmp_errno.h"

guint msg_seq_generator = 0;

typedef struct _NmpVisitBlock NmpVisitBlock;
struct _NmpVisitBlock
{
	NmpGuestVisit func;
	gpointer data;
};


static gpointer
nmp_mods_cmd_executer(gpointer user_data);

static gint
nmp_mods_io_equal_fn(gconstpointer a, gconstpointer b)
{
	NmpNewIO *new_io;
	NmpNetIO *io;
	G_ASSERT(a != NULL && b != NULL);

	new_io = (NmpNewIO*)a;
	io = (NmpNetIO*)b;

	return new_io->io == io ? 0 : 1;
}


static gint
nmp_mods_io_list_timeout(gconstpointer a, gconstpointer b)
{
	NmpNewIOList *l;
	NmpNewIO *new_io;
	G_ASSERT(a != NULL && b != NULL);

	new_io = (NmpNewIO*)a;
	l = (NmpNewIOList*)b;

	return new_io->elapse > l->timeout ? 0 : 1;
}


static void
nmp_mods_io_1sec_elapsed(gpointer data, gpointer null)
{
	NmpNewIO *new_io;
	G_ASSERT(data != NULL);

	new_io = (NmpNewIO*)data;
	++new_io->elapse;
}


static __inline__ void
__nmp_mods_io_list_timer(NmpNewIOList *l)
{
	GList *list;
	NmpNewIO *new_io;
	NmpNetIO *io;

	g_list_foreach(l->list, nmp_mods_io_1sec_elapsed, NULL);

	for (;;)
	{
		list = g_list_find_custom(l->list, l,
			nmp_mods_io_list_timeout);

		if (!list)
			break;

		new_io = (NmpNewIO*)list->data;
		l->list = g_list_delete_link(l->list, list);

		g_mutex_unlock(l->lock);

		io = new_io->io;
		nmp_net_unref_io(io);	/* release list ownership */
		nmp_mod_acc_release_io((NmpModAccess*)l->owner, io);

		g_free(new_io);

		g_mutex_lock(l->lock);
	}
}


static gboolean
nmp_mods_io_list_timer(gpointer user_data)
{
	NmpNewIOList *l;
	G_ASSERT(user_data != NULL);

	l = (NmpNewIOList*)user_data;

	g_mutex_lock(l->lock);
	__nmp_mods_io_list_timer(l);
	g_mutex_unlock(l->lock);

	return TRUE;
}


static __inline__ NmpNewIOList *
nmp_mods_io_list_new(gpointer owner, guint timeout)
{
	NmpNewIOList *l;
	G_ASSERT(owner != NULL);

	l = g_new0(NmpNewIOList, 1);
	if (G_UNLIKELY(!l))
		return NULL;

	l->lock = g_mutex_new();
	if (G_UNLIKELY(!l->lock))
	{
		g_free(l);
		return NULL;
	}

	l->timer_id = nmp_set_timer(1000,
		nmp_mods_io_list_timer, l);
	if (G_UNLIKELY(!l->timer_id))
	{
		g_mutex_free(l->lock);
		g_free(l);
		return NULL;
	}

	l->timeout = timeout;
	l->owner = owner;

	return l;
}


static void
nmp_mods_io_list_release(NmpNewIOList *list)
{
	FATAL_ERROR_EXIT;
}


static __inline__ gint
__nmp_mods_io_list_insert(NmpNewIOList *l, NmpNetIO *io)
{
	NmpNewIO *new_io;
	GList *list;

	list = g_list_find_custom(l->list, io,
		nmp_mods_io_equal_fn);

	if (G_UNLIKELY(list))
		return -E_NEWIOEXIST;

	new_io = g_new0(NmpNewIO, 1);
	if (G_UNLIKELY(!new_io))
		return -E_NOMEM;

	new_io->list = l;
	new_io->io = io;
	new_io->elapse = 0;

	l->list = g_list_append(l->list, new_io);
	nmp_net_ref_io(io);

	return 0;
}


gint
__nmp_mods_io_list_delete(NmpNewIOList *l, NmpNetIO *io)
{
	GList *list;
	NmpNewIO *new_io;

	list = g_list_find_custom(l->list, io,
		nmp_mods_io_equal_fn);

	if (G_UNLIKELY(!list))
		return -E_NOSUCHIO;

	new_io = list->data;
	BUG_ON(!new_io);

	l->list = g_list_delete_link(l->list, list);
	g_free(new_io);
	nmp_net_unref_io(io);

	return 0;
}


gint
nmp_mods_io_list_insert(NmpNewIOList *l, NmpNetIO *io)
{
	gint ret;
	G_ASSERT(l != NULL);

	g_mutex_lock(l->lock);
	ret = __nmp_mods_io_list_insert(l, io);
	g_mutex_unlock(l->lock);

	return ret;
}


gint
nmp_mods_io_list_delete(NmpNewIOList *l, NmpNetIO *io)
{
	gint ret;
	G_ASSERT(l != NULL);

	g_mutex_lock(l->lock);
	ret = __nmp_mods_io_list_delete(l, io);
	g_mutex_unlock(l->lock);
printf("----------------nmp_mods_io_list_delete\n");
	return ret;
}


gint
nmp_mods_queue_cmd(GAsyncQueue *queue, NmpModCmd cmd,
	gpointer priv, NmpCmdBlockPrivDes priv_des)
{
	NmpCmdBlock *c_b;
	G_ASSERT(queue != NULL);

	c_b = g_new0(NmpCmdBlock, 1);
	if (G_UNLIKELY(!c_b))
		return -E_NOMEM;

	c_b->cmd = cmd;
	c_b->priv = priv;
	c_b->priv_des = priv_des;

	g_async_queue_push(queue, c_b);
	return 0;
}


NmpCmdBlock *
nmp_mods_pop_cmd(GAsyncQueue *queue)
{
	return (NmpCmdBlock*)g_async_queue_pop(queue);
}


void
nmp_mods_destroy_cmd(NmpCmdBlock *c_b)
{
	G_ASSERT(c_b != NULL);

	if (c_b->priv_des)
		c_b->priv_des(c_b->priv);

	g_free(c_b);
}


guint
nmp_mods_str_hash_pjw(const gchar *start, gsize len)
{
	guint g, h = 0;
	const gchar *end;

	end = start + len;
	while (start < end)
	{
		h = (h << 4) + *start;
		++start;

		if ((g = (h & 0xF0000000)))
		{
			h ^= (g >> 24);
			h ^= g;
		}
	}

	return h;
}


static __inline__ void
nmp_mods_base_obj_init(NmpGuestBase *base_obj, const gchar *id_str,
	NmpGuestFin finalize, gpointer priv_data)
{
	NmpID *id;

	id = (NmpID*)base_obj;
	strncpy(id->id_value, id_str, MAX_ID_LEN - 1);
	id->id_hash = nmp_mods_str_hash_pjw(
		id->id_value,
		strlen(id->id_value)
	);

	base_obj->ref_count = 1;
	base_obj->finalize = finalize;
	base_obj->priv_data = priv_data;
}


void
nmp_mods_guest_ref(NmpGuestBase *base_obj)
{
	G_ASSERT(base_obj != NULL &&
		g_atomic_int_get(&base_obj->ref_count) > 0);

	g_atomic_int_inc(&base_obj->ref_count);
}


void
nmp_mods_guest_unref(NmpGuestBase *base_obj)
{
	gboolean is_zero;
	G_ASSERT(base_obj != NULL &&
		g_atomic_int_get(&base_obj->ref_count) > 0);

	is_zero = g_atomic_int_dec_and_test(&base_obj->ref_count);
	if (is_zero)
	{
		if (base_obj->finalize)
			(*base_obj->finalize)(base_obj, base_obj->priv_data);

		if (base_obj->io)
			nmp_net_unref_io(base_obj->io);

		g_free(base_obj);
	}
}


NmpGuestBase *
nmp_mods_guest_new(gsize size, const gchar *id_str, NmpGuestFin finalize,
	gpointer priv_data)
{
	NmpGuestBase *base_obj;
	G_ASSERT(size >= sizeof(NmpGuestBase) && id_str);

	base_obj = g_malloc(size);
	if (G_UNLIKELY(!base_obj))
		return NULL;

	memset(base_obj, 0, size);
	nmp_mods_base_obj_init(base_obj, id_str, finalize, priv_data);

	return base_obj;
}


void
nmp_mods_guest_attach_io(NmpGuestBase *guest, NmpNetIO *io)
{
	G_ASSERT(guest != NULL && io != NULL);

	BUG_ON(guest->io != NULL);

	guest->io = io;
	nmp_net_ref_io(io);
}


static gboolean
nmp_mods_guest_io_equal(gpointer key, gpointer value, gpointer user_data)
{
	G_ASSERT(value != NULL);

	return ((NmpGuestBase*)value)->io == user_data;
}


static gboolean
nmp_mods_guest_base_equal(gpointer key, gpointer value, gpointer user_data)
{
	G_ASSERT(value != NULL);

	return value == user_data;
}


static __inline__ gint
__nmp_mods_container_del_guest_2(NmpGuestContainer *container,
	NmpNetIO *io, NmpID *out)
{
	gint n_del;
	NmpGuestBase *base_obj;

	base_obj = g_hash_table_find(
		container->guest_table,
		nmp_mods_guest_io_equal,
		io
	);
	if (!base_obj)
		return -E_NOSUCHGUEST;

	*out = base_obj->id;

	n_del = g_hash_table_foreach_remove(
		container->guest_table,
		nmp_mods_guest_io_equal,
		io
	);

	BUG_ON(n_del != 1);

	return 0;
}


gint
__nmp_mods_container_del_guest(NmpGuestContainer *container,
	NmpGuestBase *guest)
{
	gint n_del;

	n_del = g_hash_table_foreach_remove(
		container->guest_table,
		nmp_mods_guest_base_equal,
		guest
	);

	BUG_ON(n_del > 1);

	return !(n_del == 1);
}


gint
nmp_mods_container_del_guest_2(NmpGuestContainer *container,
	NmpNetIO *io, NmpID *out)
{
	gint ret;
	G_ASSERT(container != NULL && io != NULL && out != NULL);

	g_mutex_lock(container->table_lock);
	ret = __nmp_mods_container_del_guest_2(container, io, out);
	g_mutex_unlock(container->table_lock);

	return ret;
}


gint
nmp_mods_container_del_guest(NmpGuestContainer *container,
	NmpGuestBase *guest)
{
	gint ret;
	G_ASSERT(container != NULL && guest != NULL);

	g_mutex_lock(container->table_lock);
	ret = __nmp_mods_container_del_guest(container, guest);
	g_mutex_unlock(container->table_lock);

	return ret;
}


static __inline__ gint
__nmp_mods_container_add_guest(NmpGuestContainer *container,
	NmpGuestBase *base_obj, NmpID *conflict)
{
	gint ret;
	NmpGuestBase *old;

	old = g_hash_table_lookup(container->guest_table,
		base_obj);
	if (G_UNLIKELY(old))
	{
		if (old->io == base_obj->io)
			return -E_GUESTREOL;

		nmp_print(
			"<NmpGuestContainer> guest id:%s exists!",
			ID_OF_GUEST(base_obj)
		);
		return -E_GUESTIDEXIST;
	}

	if (!__nmp_mods_container_del_guest_2(container,
		base_obj->io, conflict))
	{
		nmp_print(
			"<NmpGuestContainer> new guest-%s and guest-%s use "
			"the same io!",
			ID_OF_GUEST(base_obj), ID_STR(conflict)
		);
		return -E_GUESTIOCFLT;
	}

	ret = nmp_mods_io_list_delete(container->unrecognized_list,
		base_obj->io);
	if (G_UNLIKELY(ret))
	{
		nmp_print(
			"<NmpGuestContainer> io not found in unrecognized "
			"io list, add guest:%s failed!",
			ID_OF_GUEST(base_obj)
		);
		return ret;
	}

	g_hash_table_insert(container->guest_table, base_obj,
		base_obj);

	base_obj->container = container;
	nmp_mods_guest_ref(base_obj);

	return 0;
}


gint
nmp_mods_container_add_guest(NmpGuestContainer *container,
	NmpGuestBase *guest, NmpID *conflict)
{
	gint ret;
	G_ASSERT(container != NULL && guest != NULL
		&& conflict != NULL);

	BUG_ON(!guest->io);
	BUG_ON(guest->container);

	g_mutex_lock(container->table_lock);
	ret = __nmp_mods_container_add_guest(container,
		guest, conflict);
	if (ret)
	{
		if (ret != -E_NOSUCHIO && ret != -E_GUESTREOL)
			nmp_mods_io_list_delete(
				container->unrecognized_list,
				guest->io
			);
	}
	g_mutex_unlock(container->table_lock);

	return ret;
}


static guint
nmp_mods_guest_hash_fn(gconstpointer key)
{
	NmpID *id;
	G_ASSERT(key != NULL);

	id = (NmpID*)key;

	return id->id_hash;
}


static gboolean
nmp_mods_guest_key_equal(gconstpointer a, gconstpointer b)
{
	NmpID *id_a, *id_b;

	id_a = (NmpID*)a;
	id_b = (NmpID*)b;

	return !strcmp(id_a->id_value, id_b->id_value);
}


static void
nmp_mods_guest_value_destroy(gpointer data)
{
	NmpGuestBase *base_obj;
	G_ASSERT(data != NULL);

	base_obj = (NmpGuestBase*)data;

	nmp_mod_acc_release_io(
		(NmpModAccess*)base_obj->container->owner,
		base_obj->io
	);

	nmp_mods_guest_unref(base_obj);


}


NmpGuestContainer *
nmp_mods_container_new(gpointer owner,  guint timeout)
{
	NmpGuestContainer *container;
	G_ASSERT(owner != NULL);

	container = g_new0(NmpGuestContainer, 1);
	if (G_UNLIKELY(!container))
	{
		nmp_warning("<NmpGuestContainer> new failed!");
		return NULL;
	}

	container->unrecognized_list = nmp_mods_io_list_new(
		owner, timeout);
	if (G_UNLIKELY(!container->unrecognized_list))
	{
		nmp_warning(
			"<NmpGuestContainer> alloc temp list failed!"
		);
		goto alloc_temp_list_err;
	}

	container->cmd_queue = g_async_queue_new();
	if (G_UNLIKELY(!container->cmd_queue))
	{
		nmp_warning(
			"<NmpGuestContainer> alloc cmd queue failed!"
		);
		goto alloc_cmd_queue_err;
	}

	container->guest_table = g_hash_table_new_full(
		nmp_mods_guest_hash_fn,
		nmp_mods_guest_key_equal,
		NULL,
		nmp_mods_guest_value_destroy
	);
	if (G_UNLIKELY(!container->guest_table))
	{
		nmp_warning(
			"<NmpGuestContainer> alloc guest hash table failed!"
		);
		goto alloc_hash_table_err;
	}

	container->table_lock = g_mutex_new();
	if (G_UNLIKELY(!container->table_lock))
	{
		nmp_warning(
			"<NmpGuestContainer> alloc table lock failed!"
		);
		goto alloc_table_lock_err;
	}

	container->container_manager = g_thread_create(
        nmp_mods_cmd_executer,
        container,
        FALSE,
        NULL
    );
    if (G_UNLIKELY(!container->container_manager))
    {
    	nmp_warning(
    		"<NmpGuestContainer> alloc manager failed!"
    	);
    	goto alloc_manager_err;
    }

	container->owner = owner;
    return container;

alloc_manager_err:
	g_mutex_free(container->table_lock);

alloc_table_lock_err:
	g_hash_table_destroy(container->guest_table);

alloc_hash_table_err:
	g_async_queue_unref(container->cmd_queue);

alloc_cmd_queue_err:
	nmp_mods_io_list_release(container->unrecognized_list);

alloc_temp_list_err:
	g_free(container);

	return NULL;
}


static __inline__ void
nmp_mods_container_notify_remove(NmpGuestContainer *container,
	NmpNetIO *io)
{
	gint ret;
	G_ASSERT(container != NULL && io != NULL);

	ret = nmp_mods_queue_cmd(container->cmd_queue,
		MOD_CMD_DESTROY_ENT, io, NULL);

	if (G_UNLIKELY(ret))
		nmp_warning("<NmpGuestContainer> queue cmd failed!");
}


gint
nmp_mod_container_add_io(NmpGuestContainer *container, NmpNetIO *io)
{
	gint err;
	G_ASSERT(container != NULL && io != NULL);

	err = nmp_mods_io_list_insert(container->unrecognized_list, io);
	if (err)
	{
		nmp_error(
			"<NmpGuestContainer> insert io to temp list err: %d!",
			err
		);
		return err;
	}

	return 0;
}


gint
nmp_mod_container_del_io(NmpGuestContainer *container, NmpNetIO *io)
{
	gint ret;
	G_ASSERT(container != NULL && io != NULL);

	ret = nmp_mods_io_list_delete(container->unrecognized_list, io);
	if (G_UNLIKELY(!ret))
		return ret;

	nmp_mods_container_notify_remove(container, io);

	return -E_INPROGRESS;
}



static __inline__ NmpGuestBase *
__nmp_mods_container_get_guest(NmpGuestContainer *container,
	NmpNetIO *io)
{
	NmpGuestBase *base_obj;

	base_obj = g_hash_table_find(
		container->guest_table,
		nmp_mods_guest_io_equal,
		io
	);

	if (base_obj)
		nmp_mods_guest_ref(base_obj);

	return base_obj;
}


static __inline__ NmpGuestBase *
__nmp_mods_container_get_guest_2(NmpGuestContainer *container,
	const gchar *id_str)
{
	NmpID id;
	NmpGuestBase *base_obj;

	memset(&id, 0, sizeof(id));
	strncpy(id.id_value, id_str, MAX_ID_LEN - 1);
	id.id_hash = nmp_mods_str_hash_pjw(
		id.id_value,
		strlen(id.id_value)
	);

	base_obj = g_hash_table_lookup(
		container->guest_table,
		&id
	);

	if (base_obj)
		nmp_mods_guest_ref(base_obj);

	return base_obj;
}


NmpGuestBase *
nmp_mods_container_get_guest(NmpGuestContainer *container,
	NmpNetIO *io)
{
	NmpGuestBase *base_obj;
	G_ASSERT(container != NULL && io != NULL);

	g_mutex_lock(container->table_lock);
	base_obj = __nmp_mods_container_get_guest(container, io);
	g_mutex_unlock(container->table_lock);

	return base_obj;
}


NmpGuestBase *
nmp_mods_container_get_guest_2(NmpGuestContainer *container,
	const gchar *id_str)
{
	NmpGuestBase *base_obj;
	G_ASSERT(container != NULL && id_str != NULL);

	g_mutex_lock(container->table_lock);
	base_obj = __nmp_mods_container_get_guest_2(container, id_str);
	g_mutex_unlock(container->table_lock);

	return base_obj;
}


void
nmp_mods_container_put_guest(NmpGuestContainer *container,
	NmpGuestBase *guest)
{
	G_ASSERT(container != NULL && guest != NULL);

	nmp_mods_guest_unref(guest);
}


gint
nmp_mods_container_guest_counts(NmpGuestContainer *container)
{
	G_ASSERT(container != NULL);

	return g_hash_table_size(container->guest_table);
}


static gpointer
nmp_mods_cmd_executer(gpointer user_data)
{
	NmpGuestContainer *container;
	NmpCmdBlock *c_b;
	NmpID id;
	gint ret;

	container = (NmpGuestContainer*)user_data;

	for (;;)
	{
		c_b = nmp_mods_pop_cmd(container->cmd_queue);
		switch (c_b->cmd)
		{
		case MOD_CMD_DESTROY_ENT:
			ret = nmp_mods_container_del_guest_2(container,
				(NmpNetIO*)c_b->priv, &id);
			if (G_UNLIKELY(ret))
				nmp_warning(
					"<NmpGuestContainer> can't find guest with io=%p!",
					c_b->priv
				);
			else
			{
				nmp_print(
					"<NmpGuestContainer> guest-%s offline!",
					id.id_value
				);
			}

			break;

		case MOD_CMD_CHECK_ENT:
			break;

		default:
			nmp_warning("<NmpGuestContainer> unrecognized mod cmd!");
			break;
		}

		nmp_mods_destroy_cmd(c_b);
	}

	return NULL;
}


gint
nmp_cms_mod_deliver_msg(NmpAppObj *self, gint dst, gint msg_id,
	void *parm, gint size, void (*destroy)(void *, gsize size))
{
	NmpSysMsg *msg;

	msg = nmp_sysmsg_new(msg_id, parm, size, ++msg_seq_generator,
		(NmpMsgPrivDes)destroy);
	if (G_UNLIKELY(!msg))
		return -ENOMEM;

	MSG_SET_DSTPOS(msg, dst);
	nmp_app_obj_deliver_out(self, msg);
	return 0;
}


gint
nmp_cms_mod_deliver_msg_2(NmpAppObj *self, gint dst, gint msg_id,
	void *parm, gint size)
{
	NmpSysMsg *msg;

	msg = nmp_sysmsg_new_2(msg_id, parm, size, ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return -ENOMEM;

	MSG_SET_DSTPOS(msg, dst);
	nmp_app_obj_deliver_out(self, msg);
	return 0;
}

gint
nmp_cms_mod_cpy_msg(NmpAppObj *app_obj,NmpSysMsg *msg,  gint dst)
{
	NmpSysMsg *cpy_msg;

	cpy_msg = nmp_sysmsg_copy_one(msg);
	if (G_UNLIKELY(!cpy_msg))
		return -ENOMEM;

	MSG_SET_DSTPOS(cpy_msg, dst);
	nmp_app_obj_deliver_out(app_obj, cpy_msg);
	return 0;
}



static void
nmp_mods_visit_func(gpointer key, gpointer value, gpointer user_data)
{
	NmpVisitBlock *pvb = (NmpVisitBlock*)user_data;
	if (pvb->func)
	{
		(*pvb->func)((NmpGuestBase*)value, pvb->data);
	}
}


static __inline__ void
__nmp_mods_container_do_for_each(NmpGuestContainer *container,
	NmpGuestVisit func, gpointer user_data)
{
	NmpVisitBlock vb;
	vb.func = func;
	vb.data = user_data;

	g_hash_table_foreach(
		container->guest_table,
		nmp_mods_visit_func,
		&vb
	);
}

void
nmp_mods_container_do_for_each(NmpGuestContainer *container,
	NmpGuestVisit func, gpointer user_data)
{
	G_ASSERT(container != NULL && func != NULL);

	g_mutex_lock(container->table_lock);
	__nmp_mods_container_do_for_each(container, func, user_data);
	g_mutex_unlock(container->table_lock);
}

//:~ End
