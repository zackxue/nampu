#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_msgengine.h"
#include "nmp_appmod.h"
#include "nmp_memory.h"

G_DEFINE_TYPE(JpfMsgEngine, jpf_msg_engine, JPF_TYPE_OBJECT);

static void
jpf_msg_engine_run_task(gpointer data, gpointer user_data);
static gboolean g_msg_engine_thread_exclusive = FALSE;
static guint g_msg_engine_fthreads = MAX_MSG_ENGINE_THREADS/2;
static guint g_msg_engine_bthreads = MAX_MSG_ENGINE_THREADS/2;


static __inline__ void
jpf_msg_table_init(JpfMsgTable *table)
{
    memset(table, 0, sizeof(*table));
}


void
jpf_msg_engine_set_thread_exclusive(gboolean exclusive)
{
	g_msg_engine_thread_exclusive = exclusive;
}


void
jpf_msg_engine_set_threads(guint fthreads, guint bthreads)
{
	g_msg_engine_fthreads = fthreads;
	g_msg_engine_bthreads = bthreads;
}


static void
jpf_msg_engine_init(JpfMsgEngine *self)
{
    memset(self->name, 0, sizeof(self->name));
    self->owner = NULL;

    self->msg_dict = jpf_new(JpfMsgTable, 1);
    if (G_UNLIKELY(!self->msg_dict))
    {
        jpf_error(
            "<JpfMsgEngine> alloc msg table failed!"
        );
        FATAL_ERROR_EXIT;
    }

    jpf_msg_table_init(self->msg_dict);

    self->th_pool_f = g_thread_pool_new(
        jpf_msg_engine_run_task,
        self,
        g_msg_engine_fthreads,
        g_msg_engine_thread_exclusive,
        NULL
    );

    if (G_UNLIKELY(!self->th_pool_f))
    {
        jpf_error(
            "<JpfMsgEngine> alloc forward thread pool failed!"
        );
        FATAL_ERROR_EXIT;
    }

    self->th_pool_b = g_thread_pool_new(
        jpf_msg_engine_run_task,
        self,
        g_msg_engine_bthreads,
        g_msg_engine_thread_exclusive,
        NULL
    );

    if (G_UNLIKELY(!self->th_pool_b))
    {
        jpf_error(
            "<JpfMsgEngine> alloc backward thread pool failed!"
        );
        FATAL_ERROR_EXIT;
    }
}


static void
jpf_msg_engine_dispose(GObject *object)
{
    JpfMsgEngine *self = (JpfMsgEngine*)object;

    if (self->th_pool_b)
    {
        g_thread_pool_free(self->th_pool_b, FALSE, TRUE);
        self->th_pool_b = NULL;

        g_thread_pool_free(self->th_pool_f, FALSE, TRUE);
        self->th_pool_f = NULL;

        jpf_free(self->msg_dict);
        self->msg_dict = NULL;

        self->owner = NULL;
    }

    G_OBJECT_CLASS(jpf_msg_engine_parent_class)->dispose(object);
}


static void
jpf_msg_engine_class_init(JpfMsgEngineClass *c_self)
{
    GObjectClass *gobject_class;

    gobject_class = (GObjectClass*)c_self;
    gobject_class->dispose = jpf_msg_engine_dispose;
}


void
jpf_msg_engine_set_name(JpfMsgEngine *self, const gchar *name)
{
     G_ASSERT(self != NULL);

    strncpy(self->name, name, ENGINE_NAME - 1);
    self->name[ENGINE_NAME - 1] = 0;
}


gint
jpf_msg_engine_set_owner(JpfMsgEngine *self, JpfAppObj *owner)
{
    G_ASSERT(self != NULL);

    if (G_UNLIKELY(!owner))
        return -E_INVAL;

    self->owner = owner;
    return 0;
}


static __inline__ void
jpf_msg_engine_destroy_msg(JpfSysMsg *msg)
{
    jpf_sysmsg_destroy(msg);
}


#ifdef MSGID_USE_INTEGER

static __inline__ gint
jpf_msg_table_register(JpfMsgTable *table, JpfMsgID msg_id,
    JpfMsgFun f_fun, JpfMsgFun b_fun, guint flags)
{
    guint msg_iid;
    G_ASSERT(table != NULL);

    msg_iid = (guint)msg_id;
    if (msg_iid >= MAX_MSG_ENTRIES)
        return -EINVAL;

    if (G_UNLIKELY(table->counts >= MAX_MSG_ENTRIES))
        return -E_MSGTBLFULL;

    if (table->msg_enties[msg_iid].entry_msg)
        return -E_MSGEXIST;

    SET_MSGID(table->msg_enties[msg_iid].entry_msg, 1);
    table->msg_enties[msg_iid].entry_flags = flags;
    table->msg_enties[msg_iid].fun_forward = f_fun;
    table->msg_enties[msg_iid].fun_backward = b_fun;
    ++table->counts;

    return 0;
}


static __inline__ void
__jpf_msg_engine_dispatch_msg(JpfMsgEngine *self, JpfSysMsg *msg)
{
    guint msg_iid;
    JpfMsgFun fun;
    JpfMsgFunRet ret;

    if (G_UNLIKELY(!self->msg_dict))
    {
        jpf_warning(
            "<JpfMsgEngine> [%s] dispatch msg but no dict!",
            self->name
        );
        goto dispatch_msg_end;
    }
    else
    {
        msg_iid = (guint)MSG_GETID(msg);
        BUG_ON(msg_iid >= MAX_MSG_ENTRIES);

        if (!(guint)ENTRY_MSG_BYINDEX(self->msg_dict, msg_iid))
        {
            jpf_warning(
                "<JpfMsgEngine> [%s] dispatch msg %d but no handler, sync request timeout?",
                self->name,
                msg_iid
                );
            goto dispatch_msg_end;
        }

        if (!MSG_FORWARD(msg))
        {
            fun = ENTRY_BFUN_BYINDEX(self->msg_dict, msg_iid);
            if (G_UNLIKELY(!fun))
            {
                jpf_warning(
                    "<JpfMsgEngine> [%s] msg %d has no handler "
                    "in backward direction!",
                    self->name,
                    msg_iid
                );

                goto dispatch_msg_end;
            }

            ret = fun(self->owner, msg);
            BUG_ON(ret != MFR_ACCEPTED && ret != MFR_DELIVER_AHEAD &&
                ret != MFR_DELIVER_BACK);

            if (ret == MFR_DELIVER_AHEAD)
                jpf_app_obj_deliver_in(self->owner, msg);
            else if (ret == MFR_DELIVER_BACK)
                jpf_app_obj_deliver_out(self->owner, msg);
            return;
        }
        else
        {
            fun = ENTRY_FFUN_BYINDEX(self->msg_dict, msg_iid);
            if (G_UNLIKELY(!fun))
            {
                jpf_warning(
                    "<JpfMsgEngine> [%s] msg %d has no handler "
                    "in forward direction!",
                    self->name,
                    msg_iid
                );

                goto dispatch_msg_end;
            }

            ret = fun(self->owner, msg);
            BUG_ON(ret != MFR_ACCEPTED && ret != MFR_DELIVER_AHEAD &&
            ret != MFR_DELIVER_BACK);
            if (ret == MFR_DELIVER_AHEAD)
                jpf_app_obj_deliver_out(self->owner, msg);
            else if (ret == MFR_DELIVER_BACK)
                jpf_app_obj_deliver_in(self->owner, msg);
            return;
        }
    }

    jpf_warning(
        "<JpfMsgEngine> [%s] dispatch msg %d but no handler, sync request timeout?",
        self->name,
        MSG_GETID(msg)
    );

dispatch_msg_end:
    jpf_msg_engine_destroy_msg(msg);
}

#else   /* MSGID_USE_INTEGER */

static __inline__ gint
jpf_msg_table_ent_exist(JpfMsgTable *table, JpfMsgID msg_id)
{
    gint index = 0;

    while (index < table->counts)
    {
        if (EQUAL_MSGID(table->msg_enties[index].entry_msg, msg_id))
            return 1;

        ++index;
    }

    return 0;
}


static __inline__ gint
jpf_msg_table_register(JpfMsgTable *table, JpfMsgID msg_id,
    JpfMsgFun f_fun, JpfMsgFun b_fun, guint flags)
{
    G_ASSERT(table != NULL);

    if (G_UNLIKELY(table->counts >= MAX_MSG_ENTRIES))
        return -E_MSGTBLFULL;

    if (jpf_msg_table_ent_exist(table, msg_id))
        return -E_MSGEXIST;

    SET_MSGID(table->msg_enties[table->counts].entry_msg, msg_id);
    table->msg_enties[table->counts].entry_flags = flags;
    table->msg_enties[table->counts].fun_forward = f_fun;
    table->msg_enties[table->counts].fun_backward = b_fun;
    ++table->counts;

    return 0;
}


static __inline__ void
__jpf_msg_engine_dispatch_msg(JpfMsgEngine *self, JpfSysMsg *msg)
{
    gint index;
    JpfMsgFun fun;
    JpfMsgFunRet ret;

    if (G_UNLIKELY(!self->msg_dict))
    {
        jpf_warning(
            "<JpfMsgEngine> [%s] dispatch msg but no dict!",
            self->name
        );
        goto dispatch_msg_end;
    }
    else
    {
        for (index = 0; index < ENTRY_MSG_COUNTS(self->msg_dict); ++index)
        {
            if (!MSG_EQUAL(msg, ENTRY_MSG_BYINDEX(self->msg_dict, index)))
                continue;

            if (!MSG_FORWARD(msg))
            {
                fun = ENTRY_BFUN_BYINDEX(self->msg_dict, index);
                if (G_UNLIKELY(!fun))
                {
                    jpf_warning(
                        "<JpfMsgEngine> [%s] msg %s has no handler "
                        "in backward direction!",
                        self->name,
                        MSG_STR(msg)
                    );

                    goto dispatch_msg_end;
                }

                ret = fun(self->owner, msg);
                BUG_ON(ret != MFR_ACCEPTED && ret != MFR_DELIVER_AHEAD &&
                    ret != MFR_DELIVER_BACK);

                if (ret == MFR_DELIVER_AHEAD)
                    jpf_app_obj_deliver_in(self->owner, msg);
                else if (ret == MFR_DELIVER_BACK)
                    jpf_app_obj_deliver_out(self->owner, msg);
                return;
            }
            else
            {
                fun = ENTRY_FFUN_BYINDEX(self->msg_dict, index);
                if (G_UNLIKELY(!fun))
                {
                    jpf_warning(
                        "<JpfMsgEngine> [%s] msg %s has no handler "
                        "in forward direction!",
                        self->name,
                        MSG_STR(msg)
                    );

                    goto dispatch_msg_end;
                }

                ret = fun(self->owner, msg);
                BUG_ON(ret != MFR_ACCEPTED && ret != MFR_DELIVER_AHEAD &&
                ret != MFR_DELIVER_BACK);
                if (ret == MFR_DELIVER_AHEAD)
                    jpf_app_obj_deliver_out(self->owner, msg);
                else if (ret == MFR_DELIVER_BACK)
                    jpf_app_obj_deliver_in(self->owner, msg);
                return;
            }
        }
    }

    jpf_warning(
        "<JpfMsgEngine> [%s] dispatch msg %s but no handler, sync request timeout?",
        self->name,
        MSG_STR(msg)
    );

dispatch_msg_end:
    jpf_msg_engine_destroy_msg(msg);
}

#endif  /* MSGID_USE_INTEGER */


gint
jpf_msg_engine_register_msg(JpfMsgEngine *self, JpfMsgID msg_id,
    JpfMsgFun f_fun, JpfMsgFun b_fun, guint flags)
{
    G_ASSERT(self != NULL);

    return jpf_msg_table_register(self->msg_dict, msg_id,
        f_fun, b_fun, flags);
}


static __inline__ void
jpf_msg_engine_dispatch_msg(JpfMsgEngine *self, JpfSysMsg *msg)
{
    G_ASSERT(self != NULL && msg != NULL);

    BUG_ON(!self->owner);

    if (MSG_FORWARD(msg) || jpf_app_obj_hook_from_bus(self->owner, msg))
        __jpf_msg_engine_dispatch_msg(self, msg);
}


gint
jpf_msg_engine_push_msg_f(JpfMsgEngine *self, JpfSysMsg *msg)
{
    GError *err = NULL;
    G_ASSERT(self != NULL && msg != NULL);

    if (G_UNLIKELY(!self->th_pool_f))
        return -E_NOTHPOOL;

    g_thread_pool_push(self->th_pool_f, msg, &err);
    g_assert_no_error(err);

    return 0;
}


gint
jpf_msg_engine_push_msg_b(JpfMsgEngine *self, JpfSysMsg *msg)
{
    GError *err = NULL;
    G_ASSERT(self != NULL && msg != NULL);

    if (G_UNLIKELY(!self->th_pool_b))
        return -E_NOTHPOOL;

    g_thread_pool_push(self->th_pool_b, msg, &err);
    g_assert_no_error(err);

    return 0;
}


static void
jpf_msg_engine_run_task(gpointer data, gpointer user_data)
{
    JpfMsgEngine *self = (JpfMsgEngine*)user_data;
    JpfSysMsg *msg = (JpfSysMsg*)data;

    jpf_msg_engine_dispatch_msg(self, msg);
}


//:~ End
