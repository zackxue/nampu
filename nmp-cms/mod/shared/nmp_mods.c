/*
 * jpf_mods.c
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

typedef struct _JpfVisitBlock JpfVisitBlock;
struct _JpfVisitBlock
{
	JpfGuestVisit func;
	gpointer data;
};


static gpointer
jpf_mods_cmd_executer(gpointer user_data);

static gint
jpf_mods_io_equal_fn(gconstpointer a, gconstpointer b)
{
	JpfNewIO *new_io;
	JpfNetIO *io;
	G_ASSERT(a != NULL && b != NULL);

	new_io = (JpfNewIO*)a;
	io = (JpfNetIO*)b;

	return new_io->io == io ? 0 : 1;
}


static gint
jpf_mods_io_list_timeout(gconstpointer a, gconstpointer b)
{
	JpfNewIOList *l;
	JpfNewIO *new_io;
	G_ASSERT(a != NULL && b != NULL);

	new_io = (JpfNewIO*)a;
	l = (JpfNewIOList*)b;

	return new_io->elapse > l->timeout ? 0 : 1;
}


static void
jpf_mods_io_1sec_elapsed(gpointer data, gpointer null)
{
	JpfNewIO *new_io;
	G_ASSERT(data != NULL);

	new_io = (JpfNewIO*)data;
	++new_io->elapse;
}


static __inline__ void
__jpf_mods_io_list_timer(JpfNewIOList *l)
{
	GList *list;
	JpfNewIO *new_io;
	JpfNetIO *io;

	g_list_foreach(l->list, jpf_mods_io_1sec_elapsed, NULL);

	for (;;)
	{
		list = g_list_find_custom(l->list, l,
			jpf_mods_io_list_timeout);

		if (!list)
			break;

		new_io = (JpfNewIO*)list->data;
		l->list = g_list_delete_link(l->list, list);

		g_mutex_unlock(l->lock);

		io = new_io->io;
		jpf_net_unref_io(io);	/* release list ownership */
		jpf_mod_acc_release_io((JpfModAccess*)l->owner, io);

		g_free(new_io);

		g_mutex_lock(l->lock);
	}
}


static gboolean
jpf_mods_io_list_timer(gpointer user_data)
{
	JpfNewIOList *l;
	G_ASSERT(user_data != NULL);

	l = (JpfNewIOList*)user_data;

	g_mutex_lock(l->lock);
	__jpf_mods_io_list_timer(l);
	g_mutex_unlock(l->lock);

	return TRUE;
}


static __inline__ JpfNewIOList *
jpf_mods_io_list_new(gpointer owner, guint timeout)
{
	JpfNewIOList *l;
	G_ASSERT(owner != NULL);

	l = g_new0(JpfNewIOList, 1);
	if (G_UNLIKELY(!l))
		return NULL;

	l->lock = g_mutex_new();
	if (G_UNLIKELY(!l->lock))
	{
		g_free(l);
		return NULL;
	}

	l->timer_id = jpf_set_timer(1000,
		jpf_mods_io_list_timer, l);
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
jpf_mods_io_list_release(JpfNewIOList *list)
{
	FATAL_ERROR_EXIT;
}


static __inline__ gint
__jpf_mods_io_list_insert(JpfNewIOList *l, JpfNetIO *io)
{
	JpfNewIO *new_io;
	GList *list;

	list = g_list_find_custom(l->list, io,
		jpf_mods_io_equal_fn);

	if (G_UNLIKELY(list))
		return -E_NEWIOEXIST;

	new_io = g_new0(JpfNewIO, 1);
	if (G_UNLIKELY(!new_io))
		return -E_NOMEM;

	new_io->list = l;
	new_io->io = io;
	new_io->elapse = 0;

	l->list = g_list_append(l->list, new_io);
	jpf_net_ref_io(io);

	return 0;
}


gint
__jpf_mods_io_list_delete(JpfNewIOList *l, JpfNetIO *io)
{
	GList *list;
	JpfNewIO *new_io;

	list = g_list_find_custom(l->list, io,
		jpf_mods_io_equal_fn);

	if (G_UNLIKELY(!list))
		return -E_NOSUCHIO;

	new_io = list->data;
	BUG_ON(!new_io);

	l->list = g_list_delete_link(l->list, list);
	g_free(new_io);
	jpf_net_unref_io(io);

	return 0;
}


gint
jpf_mods_io_list_insert(JpfNewIOList *l, JpfNetIO *io)
{
	gint ret;
	G_ASSERT(l != NULL);

	g_mutex_lock(l->lock);
	ret = __jpf_mods_io_list_insert(l, io);
	g_mutex_unlock(l->lock);

	return ret;
}


gint
jpf_mods_io_list_delete(JpfNewIOList *l, JpfNetIO *io)
{
	gint ret;
	G_ASSERT(l != NULL);

	g_mutex_lock(l->lock);
	ret = __jpf_mods_io_list_delete(l, io);
	g_mutex_unlock(l->lock);
printf("----------------jpf_mods_io_list_delete\n");
	return ret;
}


gint
jpf_mods_queue_cmd(GAsyncQueue *queue, JpfModCmd cmd,
	gpointer priv, JpfCmdBlockPrivDes priv_des)
{
	JpfCmdBlock *c_b;
	G_ASSERT(queue != NULL);

	c_b = g_new0(JpfCmdBlock, 1);
	if (G_UNLIKELY(!c_b))
		return -E_NOMEM;

	c_b->cmd = cmd;
	c_b->priv = priv;
	c_b->priv_des = priv_des;

	g_async_queue_push(queue, c_b);
	return 0;
}


JpfCmdBlock *
jpf_mods_pop_cmd(GAsyncQueue *queue)
{
	return (JpfCmdBlock*)g_async_queue_pop(queue);
}


void
jpf_mods_destroy_cmd(JpfCmdBlock *c_b)
{
	G_ASSERT(c_b != NULL);

	if (c_b->priv_des)
		c_b->priv_des(c_b->priv);

	g_free(c_b);
}


guint
jpf_mods_str_hash_pjw(const gchar *start, gsize len)
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
jpf_mods_base_obj_init(JpfGuestBase *base_obj, const gchar *id_str,
	JpfGuestFin finalize, gpointer priv_data)
{
	JpfID *id;

	id = (JpfID*)base_obj;
	strncpy(id->id_value, id_str, MAX_ID_LEN - 1);
	id->id_hash = jpf_mods_str_hash_pjw(
		id->id_value,
		strlen(id->id_value)
	);

	base_obj->ref_count = 1;
	base_obj->finalize = finalize;
	base_obj->priv_data = priv_data;
}


void
jpf_mods_guest_ref(JpfGuestBase *base_obj)
{
	G_ASSERT(base_obj != NULL &&
		g_atomic_int_get(&base_obj->ref_count) > 0);

	g_atomic_int_inc(&base_obj->ref_count);
}


void
jpf_mods_guest_unref(JpfGuestBase *base_obj)
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
			jpf_net_unref_io(base_obj->io);

		g_free(base_obj);
	}
}


JpfGuestBase *
jpf_mods_guest_new(gsize size, const gchar *id_str, JpfGuestFin finalize,
	gpointer priv_data)
{
	JpfGuestBase *base_obj;
	G_ASSERT(size >= sizeof(JpfGuestBase) && id_str);

	base_obj = g_malloc(size);
	if (G_UNLIKELY(!base_obj))
		return NULL;

	memset(base_obj, 0, size);
	jpf_mods_base_obj_init(base_obj, id_str, finalize, priv_data);

	return base_obj;
}


void
jpf_mods_guest_attach_io(JpfGuestBase *guest, JpfNetIO *io)
{
	G_ASSERT(guest != NULL && io != NULL);

	BUG_ON(guest->io != NULL);

	guest->io = io;
	jpf_net_ref_io(io);
}


static gboolean
jpf_mods_guest_io_equal(gpointer key, gpointer value, gpointer user_data)
{
	G_ASSERT(value != NULL);

	return ((JpfGuestBase*)value)->io == user_data;
}


static gboolean
jpf_mods_guest_base_equal(gpointer key, gpointer value, gpointer user_data)
{
	G_ASSERT(value != NULL);

	return value == user_data;
}


static __inline__ gint
__jpf_mods_container_del_guest_2(JpfGuestContainer *container,
	JpfNetIO *io, JpfID *out)
{
	gint n_del;
	JpfGuestBase *base_obj;

	base_obj = g_hash_table_find(
		container->guest_table,
		jpf_mods_guest_io_equal,
		io
	);
	if (!base_obj)
		return -E_NOSUCHGUEST;

	*out = base_obj->id;

	n_del = g_hash_table_foreach_remove(
		container->guest_table,
		jpf_mods_guest_io_equal,
		io
	);

	BUG_ON(n_del != 1);

	return 0;
}


gint
__jpf_mods_container_del_guest(JpfGuestContainer *container,
	JpfGuestBase *guest)
{
	gint n_del;

	n_del = g_hash_table_foreach_remove(
		container->guest_table,
		jpf_mods_guest_base_equal,
		guest
	);

	BUG_ON(n_del > 1);

	return !(n_del == 1);
}


gint
jpf_mods_container_del_guest_2(JpfGuestContainer *container,
	JpfNetIO *io, JpfID *out)
{
	gint ret;
	G_ASSERT(container != NULL && io != NULL && out != NULL);

	g_mutex_lock(container->table_lock);
	ret = __jpf_mods_container_del_guest_2(container, io, out);
	g_mutex_unlock(container->table_lock);

	return ret;
}


gint
jpf_mods_container_del_guest(JpfGuestContainer *container,
	JpfGuestBase *guest)
{
	gint ret;
	G_ASSERT(container != NULL && guest != NULL);

	g_mutex_lock(container->table_lock);
	ret = __jpf_mods_container_del_guest(container, guest);
	g_mutex_unlock(container->table_lock);

	return ret;
}


static __inline__ gint
__jpf_mods_container_add_guest(JpfGuestContainer *container,
	JpfGuestBase *base_obj, JpfID *conflict)
{
	gint ret;
	JpfGuestBase *old;

	old = g_hash_table_lookup(container->guest_table,
		base_obj);
	if (G_UNLIKELY(old))
	{
		if (old->io == base_obj->io)
			return -E_GUESTREOL;

		jpf_print(
			"<JpfGuestContainer> guest id:%s exists!",
			ID_OF_GUEST(base_obj)
		);
		return -E_GUESTIDEXIST;
	}

	if (!__jpf_mods_container_del_guest_2(container,
		base_obj->io, conflict))
	{
		jpf_print(
			"<JpfGuestContainer> new guest-%s and guest-%s use "
			"the same io!",
			ID_OF_GUEST(base_obj), ID_STR(conflict)
		);
		return -E_GUESTIOCFLT;
	}

	ret = jpf_mods_io_list_delete(container->unrecognized_list,
		base_obj->io);
	if (G_UNLIKELY(ret))
	{
		jpf_print(
			"<JpfGuestContainer> io not found in unrecognized "
			"io list, add guest:%s failed!",
			ID_OF_GUEST(base_obj)
		);
		return ret;
	}

	g_hash_table_insert(container->guest_table, base_obj,
		base_obj);

	base_obj->container = container;
	jpf_mods_guest_ref(base_obj);

	return 0;
}


gint
jpf_mods_container_add_guest(JpfGuestContainer *container,
	JpfGuestBase *guest, JpfID *conflict)
{
	gint ret;
	G_ASSERT(container != NULL && guest != NULL
		&& conflict != NULL);

	BUG_ON(!guest->io);
	BUG_ON(guest->container);

	g_mutex_lock(container->table_lock);
	ret = __jpf_mods_container_add_guest(container,
		guest, conflict);
	if (ret)
	{
		if (ret != -E_NOSUCHIO && ret != -E_GUESTREOL)
			jpf_mods_io_list_delete(
				container->unrecognized_list,
				guest->io
			);
	}
	g_mutex_unlock(container->table_lock);

	return ret;
}


static guint
jpf_mods_guest_hash_fn(gconstpointer key)
{
	JpfID *id;
	G_ASSERT(key != NULL);

	id = (JpfID*)key;

	return id->id_hash;
}


static gboolean
jpf_mods_guest_key_equal(gconstpointer a, gconstpointer b)
{
	JpfID *id_a, *id_b;

	id_a = (JpfID*)a;
	id_b = (JpfID*)b;

	return !strcmp(id_a->id_value, id_b->id_value);
}


static void
jpf_mods_guest_value_destroy(gpointer data)
{
	JpfGuestBase *base_obj;
	G_ASSERT(data != NULL);

	base_obj = (JpfGuestBase*)data;

	jpf_mod_acc_release_io(
		(JpfModAccess*)base_obj->container->owner,
		base_obj->io
	);

	jpf_mods_guest_unref(base_obj);


}


JpfGuestContainer *
jpf_mods_container_new(gpointer owner,  guint timeout)
{
	JpfGuestContainer *container;
	G_ASSERT(owner != NULL);

	container = g_new0(JpfGuestContainer, 1);
	if (G_UNLIKELY(!container))
	{
		jpf_warning("<JpfGuestContainer> new failed!");
		return NULL;
	}

	container->unrecognized_list = jpf_mods_io_list_new(
		owner, timeout);
	if (G_UNLIKELY(!container->unrecognized_list))
	{
		jpf_warning(
			"<JpfGuestContainer> alloc temp list failed!"
		);
		goto alloc_temp_list_err;
	}

	container->cmd_queue = g_async_queue_new();
	if (G_UNLIKELY(!container->cmd_queue))
	{
		jpf_warning(
			"<JpfGuestContainer> alloc cmd queue failed!"
		);
		goto alloc_cmd_queue_err;
	}

	container->guest_table = g_hash_table_new_full(
		jpf_mods_guest_hash_fn,
		jpf_mods_guest_key_equal,
		NULL,
		jpf_mods_guest_value_destroy
	);
	if (G_UNLIKELY(!container->guest_table))
	{
		jpf_warning(
			"<JpfGuestContainer> alloc guest hash table failed!"
		);
		goto alloc_hash_table_err;
	}

	container->table_lock = g_mutex_new();
	if (G_UNLIKELY(!container->table_lock))
	{
		jpf_warning(
			"<JpfGuestContainer> alloc table lock failed!"
		);
		goto alloc_table_lock_err;
	}

	container->container_manager = g_thread_create(
        jpf_mods_cmd_executer,
        container,
        FALSE,
        NULL
    );
    if (G_UNLIKELY(!container->container_manager))
    {
    	jpf_warning(
    		"<JpfGuestContainer> alloc manager failed!"
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
	jpf_mods_io_list_release(container->unrecognized_list);

alloc_temp_list_err:
	g_free(container);

	return NULL;
}


static __inline__ void
jpf_mods_container_notify_remove(JpfGuestContainer *container,
	JpfNetIO *io)
{
	gint ret;
	G_ASSERT(container != NULL && io != NULL);

	ret = jpf_mods_queue_cmd(container->cmd_queue,
		MOD_CMD_DESTROY_ENT, io, NULL);

	if (G_UNLIKELY(ret))
		jpf_warning("<JpfGuestContainer> queue cmd failed!");
}


gint
jpf_mod_container_add_io(JpfGuestContainer *container, JpfNetIO *io)
{
	gint err;
	G_ASSERT(container != NULL && io != NULL);

	err = jpf_mods_io_list_insert(container->unrecognized_list, io);
	if (err)
	{
		jpf_error(
			"<JpfGuestContainer> insert io to temp list err: %d!",
			err
		);
		return err;
	}

	return 0;
}


gint
jpf_mod_container_del_io(JpfGuestContainer *container, JpfNetIO *io)
{
	gint ret;
	G_ASSERT(container != NULL && io != NULL);

	ret = jpf_mods_io_list_delete(container->unrecognized_list, io);
	if (G_UNLIKELY(!ret))
		return ret;

	jpf_mods_container_notify_remove(container, io);

	return -E_INPROGRESS;
}



static __inline__ JpfGuestBase *
__jpf_mods_container_get_guest(JpfGuestContainer *container,
	JpfNetIO *io)
{
	JpfGuestBase *base_obj;

	base_obj = g_hash_table_find(
		container->guest_table,
		jpf_mods_guest_io_equal,
		io
	);

	if (base_obj)
		jpf_mods_guest_ref(base_obj);

	return base_obj;
}


static __inline__ JpfGuestBase *
__jpf_mods_container_get_guest_2(JpfGuestContainer *container,
	const gchar *id_str)
{
	JpfID id;
	JpfGuestBase *base_obj;

	memset(&id, 0, sizeof(id));
	strncpy(id.id_value, id_str, MAX_ID_LEN - 1);
	id.id_hash = jpf_mods_str_hash_pjw(
		id.id_value,
		strlen(id.id_value)
	);

	base_obj = g_hash_table_lookup(
		container->guest_table,
		&id
	);

	if (base_obj)
		jpf_mods_guest_ref(base_obj);

	return base_obj;
}


JpfGuestBase *
jpf_mods_container_get_guest(JpfGuestContainer *container,
	JpfNetIO *io)
{
	JpfGuestBase *base_obj;
	G_ASSERT(container != NULL && io != NULL);

	g_mutex_lock(container->table_lock);
	base_obj = __jpf_mods_container_get_guest(container, io);
	g_mutex_unlock(container->table_lock);

	return base_obj;
}


JpfGuestBase *
jpf_mods_container_get_guest_2(JpfGuestContainer *container,
	const gchar *id_str)
{
	JpfGuestBase *base_obj;
	G_ASSERT(container != NULL && id_str != NULL);

	g_mutex_lock(container->table_lock);
	base_obj = __jpf_mods_container_get_guest_2(container, id_str);
	g_mutex_unlock(container->table_lock);

	return base_obj;
}


void
jpf_mods_container_put_guest(JpfGuestContainer *container,
	JpfGuestBase *guest)
{
	G_ASSERT(container != NULL && guest != NULL);

	jpf_mods_guest_unref(guest);
}


gint
jpf_mods_container_guest_counts(JpfGuestContainer *container)
{
	G_ASSERT(container != NULL);

	return g_hash_table_size(container->guest_table);
}


static gpointer
jpf_mods_cmd_executer(gpointer user_data)
{
	JpfGuestContainer *container;
	JpfCmdBlock *c_b;
	JpfID id;
	gint ret;

	container = (JpfGuestContainer*)user_data;

	for (;;)
	{
		c_b = jpf_mods_pop_cmd(container->cmd_queue);
		switch (c_b->cmd)
		{
		case MOD_CMD_DESTROY_ENT:
			ret = jpf_mods_container_del_guest_2(container,
				(JpfNetIO*)c_b->priv, &id);
			if (G_UNLIKELY(ret))
				jpf_warning(
					"<JpfGuestContainer> can't find guest with io=%p!",
					c_b->priv
				);
			else
			{
				jpf_print(
					"<JpfGuestContainer> guest-%s offline!",
					id.id_value
				);
			}

			break;

		case MOD_CMD_CHECK_ENT:
			break;

		default:
			jpf_warning("<JpfGuestContainer> unrecognized mod cmd!");
			break;
		}

		jpf_mods_destroy_cmd(c_b);
	}

	return NULL;
}


gint
jpf_cms_mod_deliver_msg(JpfAppObj *self, gint dst, gint msg_id,
	void *parm, gint size, void (*destroy)(void *, gsize size))
{
	JpfSysMsg *msg;

	msg = jpf_sysmsg_new(msg_id, parm, size, ++msg_seq_generator,
		(JpfMsgPrivDes)destroy);
	if (G_UNLIKELY(!msg))
		return -ENOMEM;

	MSG_SET_DSTPOS(msg, dst);
	jpf_app_obj_deliver_out(self, msg);
	return 0;
}


gint
jpf_cms_mod_deliver_msg_2(JpfAppObj *self, gint dst, gint msg_id,
	void *parm, gint size)
{
	JpfSysMsg *msg;

	msg = jpf_sysmsg_new_2(msg_id, parm, size, ++msg_seq_generator);
	if (G_UNLIKELY(!msg))
		return -ENOMEM;

	MSG_SET_DSTPOS(msg, dst);
	jpf_app_obj_deliver_out(self, msg);
	return 0;
}

gint
jpf_cms_mod_cpy_msg(JpfAppObj *app_obj,JpfSysMsg *msg,  gint dst)
{
	JpfSysMsg *cpy_msg;

	cpy_msg = jpf_sysmsg_copy_one(msg);
	if (G_UNLIKELY(!cpy_msg))
		return -ENOMEM;

	MSG_SET_DSTPOS(cpy_msg, dst);
	jpf_app_obj_deliver_out(app_obj, cpy_msg);
	return 0;
}



static void
jpf_mods_visit_func(gpointer key, gpointer value, gpointer user_data)
{
	JpfVisitBlock *pvb = (JpfVisitBlock*)user_data;
	if (pvb->func)
	{
		(*pvb->func)((JpfGuestBase*)value, pvb->data);
	}
}


static __inline__ void
__jpf_mods_container_do_for_each(JpfGuestContainer *container,
	JpfGuestVisit func, gpointer user_data)
{
	JpfVisitBlock vb;
	vb.func = func;
	vb.data = user_data;

	g_hash_table_foreach(
		container->guest_table,
		jpf_mods_visit_func,
		&vb
	);
}

void
jpf_mods_container_do_for_each(JpfGuestContainer *container,
	JpfGuestVisit func, gpointer user_data)
{
	G_ASSERT(container != NULL && func != NULL);

	g_mutex_lock(container->table_lock);
	__jpf_mods_container_do_for_each(container, func, user_data);
	g_mutex_unlock(container->table_lock);
}

//:~ End
