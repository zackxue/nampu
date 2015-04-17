
#include "nmp_proxy_log.h"
#include "nmp_net_impl.h"
#include "nmp_msg_impl.h"


static void default_msg_engine_func(void *data, void *user);

msg_t *alloc_new_msg(int id, void *data, size_t size, int seq)
{
    void *body;
    msg_t *msg = NULL;
    NMP_ASSERT(!(data && !size));

    body = nmp_alloc(size);
    if (body)
    {
        memcpy(body, data, size);
        msg = alloc_new_msg_2(id, body, size, seq, dealloc_msg_data);
        if (!msg)
            nmp_dealloc(body, size);
    }

    return msg;
}

msg_t *alloc_new_msg_2(int id, void *data, size_t size, int seq, msg_fin_t fin)
{
    msg_t *msg;
    NMP_ASSERT(!(data && !size) && !(data && !fin));

    msg = (msg_t*)nmp_new0(msg_t, 1);
    if (msg)
    {
        msg->id = id;
        msg->seq = seq;
        if (data)
        {
            msg->data = data;
            msg->size = size;
            msg->fin = fin;
        }
        
        msg->io = NULL;
        msg->owner.object = NULL;
        msg->owner.ref    = NULL;
        msg->owner.unref  = NULL;
    }

    return msg;
}

void dealloc_msg_data(void *data, size_t size)
{
    if (data && size >= 0)
        nmp_dealloc(data, size);
}

msg_t *attach_msg_io(msg_t *msg, nmpio_t *io)
{
    NMP_ASSERT(msg && io);
    msg->io = nmp_net_ref_io(io);
    return msg;
}

msg_t *attach_msg_owner(msg_t *msg, msg_owner_t *owner)
{
    NMP_ASSERT(msg);
    NMP_ASSERT(owner && owner->object);
    NMP_ASSERT(!(owner->ref && !owner->unref) && !(!owner->ref && owner->unref));

    if (owner->ref)
        owner->object = (*owner->ref)(owner->object);

    msg->owner = *owner;
    return msg;
}

void set_msg_data(msg_t *msg, void *data, size_t size)
{
    NMP_ASSERT(msg && !(data && !size));

    if (msg->data)
    {
        BUG_ON(!msg->fin);
        (*msg->fin)(msg->data, msg->size);
        msg->data = NULL;
        msg->size = 0;
        msg->fin = NULL;
    }

    if (data)
    {
        msg->data = nmp_alloc(size);
        msg->size = size;
        memcpy(msg->data, data, size);
        msg->fin = dealloc_msg_data;
    }
}

void set_msg_data_2(msg_t *msg, void *data, size_t size, msg_fin_t fin)
{
    NMP_ASSERT(!(data && !size) && !(data && !fin));

    if (msg->data)
    {
        BUG_ON(!msg->fin);
        (*msg->fin)(msg->data, msg->size);
        msg->data = NULL;
        msg->size = 0;
        msg->fin = NULL;
    }

    if (data)
    {
        msg->data = data;
        msg->size = size;
        msg->fin = fin;
    }
}

static __inline__ void free_msg_skin(msg_t *msg)
{
    if (msg->io)
        nmp_net_unref_io(msg->io);

    if (msg->owner.object && msg->owner.unref)
        (*msg->owner.unref)(msg->owner.object);

    nmp_del(msg, msg_t, 1);
}

void free_msg(msg_t *msg)
{
    NMP_ASSERT(msg);

    if (msg->data)
    {
        BUG_ON(!msg->fin);
        (*msg->fin)(msg->data, msg->size);
    }

    free_msg_skin(msg);
}

void free_msg_2(msg_t *msg)
{
    NMP_ASSERT(msg);

    free_msg_skin(msg);
}

msg_engine_t *create_msg_engine(nmp_bool_t immediate, int max_thread_num)
{
    msg_engine_t *me;
    NMP_ASSERT((immediate ? immediate : max_thread_num));

    me = (msg_engine_t*)nmp_new0(msg_engine_t, 1);
    me->msg_map = (msg_map_t*)nmp_new0(msg_map_t, 1);
    me->immediate = immediate;

    if (!me->immediate)
    {
        me->th_pool = nmp_threadpool_new(default_msg_engine_func, 
                        me, max_thread_num, NULL);
    }

    return me;
}

void destory_msg_engine(msg_engine_t *me)
{
    NMP_ASSERT(me);

    if (me->msg_map)
        nmp_del(me->msg_map, msg_map_t, 1);

    if (!me->immediate)
        nmp_threadpool_free(me->th_pool, 0, 1);

    nmp_del(me, msg_engine_t, 1);
}

#ifdef _DEBUG_
static __inline__ void 
register_msg_check(msg_map_t *msg_map, 
    int msg_id, int parm_id, msg_handler_t handler)
{
    int i;

    for (i=0; i<msg_map->count; i++)
    {
        if (msg_map->entries[i].msg_id == msg_id)
        {
            BUG();
        }

        if (msg_map->entries[i].parm_id == parm_id)
        {
            BUG();
        }
    }
}
#endif

int register_msg_handler(msg_engine_t *me, int msg_id,
        int parm_id, msg_handler_t handler)
{
    msg_map_t *msg_map;
    NMP_ASSERT(me);

    msg_map = me->msg_map;
    if (msg_map->count >= MAX_MSG_SIZE)
        return -1;

#ifdef _DEBUG_
    register_msg_check(msg_map, msg_id, parm_id, handler);
#endif

    msg_map->entries[msg_map->count].msg_id  = msg_id;
    msg_map->entries[msg_map->count].parm_id = parm_id;
    msg_map->entries[msg_map->count].handler = handler;
    msg_map->count++;

    return 0;
}


static __inline__ void 
dispatch_msg(msg_engine_t *me, msg_map_t *msg_map, msg_t *msg)
{
    int i, ret;
    msg_handler_t handler;

    for (i=0; i<msg_map->count; ++i)
    {
        if (msg_map->entries[i].msg_id == msg->id)
        {
            handler = msg_map->entries[i].handler;
            if (!handler)
            {
                show_debug("Msg '%d' no handler.\n", msg->id);
                break;
            }

            ret = (*handler)(msg->owner.object, msg, msg_map->entries[i].parm_id);

            BUG_ON(ret != RET_ACCEPTED && ret != RET_BACK && ret != RET_DROP);

            if (ret == RET_ACCEPTED)
                return ;

            if (ret == RET_DROP)
                break;

            send_msg(msg);
        }
    }

    free_msg(msg);
}

static void
default_msg_engine_func(void *data, void *user)
{
    msg_engine_t *me;
    msg_t *msg;
    NMP_ASSERT(data && user);

    me = (msg_engine_t*)user;
    msg = (msg_t*)data;

    dispatch_msg(me, me->msg_map, msg);
}

void deliver_msg(msg_engine_t *me, msg_t *msg)
{
    NMP_ASSERT(me && msg);

    if (!me->immediate)
        nmp_threadpool_push(me->th_pool, msg);
    else
        dispatch_msg(me, me->msg_map, msg);
}


