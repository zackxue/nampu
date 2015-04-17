#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "nmp_net_impl.h"
#include "nmp_resolve_host.h"
#include "nmp_proxy_device.h"


extern int config_broadcast_device_state(proxy_device_t *dev, proxy_state_t *dev_state);

enum
{
    CREATE_PLT_SRV = 1,
    CREATE_SDK_SRV = 2,
    CREATE_ALL_SRV = 3, /* =1+2 */
};


typedef struct proxy_device_list proxy_dlist_t;

struct proxy_device_list
{
    nmp_mutex_t *lock;
    nmp_list_t  *list;
};

static proxy_dlist_t *global_dlist_manager = NULL;

static __inline__ int32_t 
__proxy_device_killed(proxy_device_t *dev)
{
    return dev->killed == STATE_KILLED;
}


static void 
proxy_check_device(void *orig, void *custom)
{
    proxy_device_t *dev = (proxy_device_t*)orig;
    proxy_dlist_t *dev_list = (proxy_dlist_t*)custom;
    NMP_ASSERT(orig && custom);

    if (__proxy_device_killed(dev))
        return ;

    proxy_device_ref(dev);
    check_service(dev->sdk_srv);
    check_service(dev->plt_srv);

    nmp_mutex_unlock(dev_list->lock);
    proxy_device_unref(dev);
    nmp_mutex_lock(dev_list->lock);
}
static void 
proxy_destroy_device(void *orig, void *custom)
{
    proxy_device_t *dev = (proxy_device_t*)orig;
    proxy_dlist_t *dev_list = (proxy_dlist_t*)custom;

    if (!__proxy_device_killed(dev))
        dev->killed = STATE_KILLED;

    nmp_mutex_unlock(dev_list->lock);
    proxy_device_unref(dev);
    nmp_mutex_lock(dev_list->lock);
}

int proxy_manage_device()
{
    proxy_dlist_t *dev_list;
    if (!global_dlist_manager)
    {
        global_dlist_manager = (proxy_dlist_t*)nmp_new(proxy_dlist_t, 1);
        global_dlist_manager->lock = nmp_mutex_new();
        global_dlist_manager->list = NULL;
        dev_list = global_dlist_manager;
    }
    else
        dev_list = global_dlist_manager;

    nmp_mutex_lock(dev_list->lock);
    nmp_list_foreach(dev_list->list, proxy_check_device, (void*)dev_list);
    nmp_mutex_unlock(dev_list->lock);

    return 0;
}

void proxy_destroy_all_device()
{
    proxy_dlist_t *dev_list;
    if (!global_dlist_manager)
    {
        show_warn("Global device list uninitialized!!!\n");
        return ;
    }
    else
        dev_list = global_dlist_manager;

    nmp_mutex_lock(dev_list->lock);
    nmp_list_foreach(dev_list->list, proxy_destroy_device, (void*)dev_list);
    nmp_mutex_unlock(dev_list->lock);
}

static void 
proxy_add_new_device(proxy_device_t *dev)
{
    proxy_dlist_t *dev_list;
    if (!global_dlist_manager)
    {
        global_dlist_manager = (proxy_dlist_t*)nmp_new(proxy_dlist_t, 1);
        global_dlist_manager->lock = nmp_mutex_new();
        global_dlist_manager->list = NULL;
        dev_list = global_dlist_manager;
    }
    else
        dev_list = global_dlist_manager;

    nmp_mutex_lock(dev_list->lock);
    dev_list->list = nmp_list_add_tail(dev_list->list, dev);
    nmp_mutex_unlock(dev_list->lock);

    return ;
}

static void 
proxy_rm_one_device(proxy_device_t *dev)
{
    proxy_dlist_t *dev_list;
    if (!global_dlist_manager)
    {
        show_warn("Global device list uninitialized!!!\n");
        return ;
    }
    else
        dev_list = global_dlist_manager;

    nmp_mutex_lock(dev_list->lock);
    dev_list->list = nmp_list_remove(dev_list->list, dev);
    nmp_mutex_unlock(dev_list->lock);

    return ;
}

static __inline__ void 
release_device_private(proxy_device_t *dev)
{
    int pri_size;
    proxy_private_t *pri_obj;

    pri_obj = (proxy_private_t*)dev->pri_data;
    if (pri_obj)
    {
        dev->pri_data = NULL;
        pri_size = sizeof(proxy_private_t) + pri_obj->total_size;
        nmp_mutex_free(pri_obj->lock);
        nmp_dealloc(pri_obj, pri_size);
    }
}

static __inline__ int 
proxy_init_device(proxy_device_t *dev)
{
    int flag = 0;
    struct service *plt_srv = NULL;
    struct service *sdk_srv = NULL;

    proxy_plt_t plt_info;
    proxy_sdk_t sdk_info;

    memset(&plt_info, 0, sizeof(proxy_plt_t));
    memset(&sdk_info, 0, sizeof(proxy_sdk_t));

    if (proxy_get_device_private(dev, DEV_PRI_PLT_SRV, &plt_info))
    {
        show_warn("No plt info, while init device!!");
        return E_INIT_DEVICE;
    }
    if (proxy_get_device_private(dev, DEV_PRI_PRX_SDK, &sdk_info))
    {
        show_warn("No sdk info, while init device!!");
        return E_INIT_DEVICE;
    }

    plt_srv = create_new_service(dev, PLT_SRV, DEF_PLATFORM_SERVICE_NAME);
    if (plt_srv)
    {
        dev->plt_srv = plt_srv;
        flag += CREATE_PLT_SRV;
    }

    sdk_srv = create_new_service(dev, SDK_SRV, sdk_info.sdk_name);
    if (sdk_srv)
    {
        dev->sdk_srv = sdk_srv;
        flag += CREATE_SDK_SRV;
    }

    switch (flag)
    {
        case CREATE_PLT_SRV:
            destory_service(plt_srv);
            flag = E_INIT_DEVICE;
            break;

        case CREATE_SDK_SRV:
            destory_service(sdk_srv);
            flag = E_INIT_DEVICE;
            break;

        case CREATE_ALL_SRV:
            flag = 0;
            break;
    }

    return -flag;
}

proxy_device_t *
proxy_create_new_device(int index, proxy_plt_t *plt_info, proxy_sdk_t *sdk_info)
{
    proxy_device_t *dev;

    NMP_ASSERT(plt_info && sdk_info);

    dev = (proxy_device_t*)nmp_new0(proxy_device_t, 1);

    proxy_set_device_private(dev, DEV_PRI_PLT_SRV, sizeof(proxy_plt_t), plt_info);
    proxy_set_device_private(dev, DEV_PRI_PRX_SDK, sizeof(proxy_sdk_t), sdk_info);

    if (!proxy_init_device(dev))
    {
        atomic_set(&dev->ref_count, 1);
        dev->lock = nmp_mutex_new();
        dev->killed = STATE_OK;
        dev->state.state = STATE_LOGOUT;
        dev->state.error = 0;
        dev->fastenings.dev_id = index;
        dev->fastenings.dev_type = plt_info->pu_type;
    }
    else
    {
        release_device_private(dev);
        nmp_del(dev, proxy_device_t, 1);
        dev = NULL;
    }

    if (dev)
        proxy_add_new_device(dev);

    return dev;
}

proxy_device_t *proxy_device_ref(proxy_device_t *dev)
{
    NMP_ASSERT(dev && atomic_get(&dev->ref_count) > 0);

    atomic_inc(&dev->ref_count);
    return dev;
}

void proxy_device_unref(proxy_device_t *dev)
{
    NMP_ASSERT(dev && atomic_get(&dev->ref_count) > 0);

    if (atomic_dec_and_test_zero(&dev->ref_count))
    {
        proxy_rm_one_device(dev);

        destory_service(dev->sdk_srv);
        destory_service(dev->plt_srv);
        release_device_private(dev);

        nmp_mutex_free(dev->lock);
        nmp_del(dev, proxy_device_t, 1);
        dev = NULL;
    }
}

static int compare_dev_id(void *orig, void *custom)
{
    proxy_device_t *prx_dev = (proxy_device_t*)orig;
    if (prx_dev->fastenings.dev_id == (int)custom)
        return 0;
    else
        return -1;
}
static int compare_dev_ip(void *orig, void *custom)
{
    char addr[MAX_IP_LEN];
    proxy_sdk_t sdk_info;
    proxy_device_t *prx_dev = (proxy_device_t*)orig;

    memset(addr, 0, sizeof(addr));
    memset(&sdk_info, 0, sizeof(proxy_sdk_t));

    if (!proxy_get_device_private(prx_dev, DEV_PRI_PRX_SDK, &sdk_info))
    {
        if (proxy_resolve_host_immediate(sdk_info.dev_host, addr, sizeof(addr)))
            return strcmp(custom, (const char*)addr);
    }

    return -1;
}
static int compare_pu_id(void *orig, void *custom)
{
    proxy_plt_t plt_info;
    memset(&plt_info, 0, sizeof(proxy_plt_t));

    proxy_device_t *prx_dev = (proxy_device_t*)orig;

    if (!proxy_get_device_private(prx_dev, DEV_PRI_PLT_SRV, &plt_info))
    {
        return strcmp(custom, plt_info.pu_id);
    }

    return -1;
}

static __inline__ proxy_device_t *
__find_device_by_dev_id(nmp_list_t *list, int dev_id)
{
    nmp_list_t *node;
    proxy_device_t *prx_dev = NULL;

    node = nmp_list_find_custom(nmp_list_first(list), 
            (void*)dev_id, compare_dev_id);
    if (node)
    {
        prx_dev = (proxy_device_t*)nmp_list_data(node);
        nmp_mutex_lock(prx_dev->lock);
        if (!__proxy_device_killed(prx_dev))
            proxy_device_ref(prx_dev);
        else
            prx_dev = NULL;
        nmp_mutex_unlock(prx_dev->lock);
    }

    return prx_dev;
}
static __inline__ proxy_device_t *
__find_device_by_user_id(nmp_list_t *list, int user_id, const char *sdk_name)
{
    nmp_list_t *node;
    proxy_device_t *tmp_dev = NULL;
    proxy_device_t *prx_dev = NULL;

    node = nmp_list_first(list);
    while (node)
    {
        tmp_dev = (proxy_device_t*)nmp_list_data(node);
        if (!strcmp(tmp_dev->sdk_srv->tm->service_name, sdk_name))
        {
            if (!control_service(tmp_dev->sdk_srv, 
                CTRL_CMD_COMPARE, (void*)user_id))
            {
                nmp_mutex_lock(tmp_dev->lock);
                if (!__proxy_device_killed(tmp_dev))
                    prx_dev = proxy_device_ref(tmp_dev);
                else
                    prx_dev = NULL;
                nmp_mutex_unlock(tmp_dev->lock);
                break;
            }
        }
        node = nmp_list_next(node);
    }

    return prx_dev;
}
static __inline__ proxy_device_t *
__find_device_by_dev_ip(nmp_list_t *list, char *dev_ip)
{
    nmp_list_t *node;
    proxy_device_t *prx_dev = NULL;

    node = nmp_list_find_custom(nmp_list_first(list), 
            (void*)dev_ip, compare_dev_ip);
    if (node)
    {
        prx_dev = (proxy_device_t*)nmp_list_data(node);
        nmp_mutex_lock(prx_dev->lock);
        if (!__proxy_device_killed(prx_dev))
            proxy_device_ref(prx_dev);
        else
            prx_dev = NULL;
        nmp_mutex_unlock(prx_dev->lock);
    }

    return prx_dev;
}
static __inline__ proxy_device_t *
__find_device_by_pu_id(nmp_list_t *list, char *pu_id)
{
    nmp_list_t *node;
    proxy_device_t *prx_dev = NULL;

    node = nmp_list_find_custom(nmp_list_first(list), 
            (void*)pu_id, compare_pu_id);
    if (node)
    {
        prx_dev = (proxy_device_t*)nmp_list_data(node);
        nmp_mutex_lock(prx_dev->lock);
        if (!__proxy_device_killed(prx_dev))
            proxy_device_ref(prx_dev);
        else
            prx_dev = NULL;
        nmp_mutex_unlock(prx_dev->lock);
    }

    return prx_dev;
}

proxy_device_t *
proxy_find_device_by_dev_id(int dev_id)
{
    proxy_dlist_t *dev_list;
    proxy_device_t *prx_dev;

    NMP_ASSERT(0 <= dev_id);

    if (!global_dlist_manager)
    {
        show_warn("Global device list uninitialized!!!\n");
        return NULL;
    }
    else
        dev_list = global_dlist_manager;

    nmp_mutex_lock(dev_list->lock);
    prx_dev = __find_device_by_dev_id(dev_list->list, dev_id);
    nmp_mutex_unlock(dev_list->lock);

    return prx_dev;
}

proxy_device_t *
proxy_find_device_by_user_id(int user_id, const char *sdk_name)
{
    proxy_dlist_t *dev_list;
    proxy_device_t *prx_dev;

    NMP_ASSERT(sdk_name);

    if (!global_dlist_manager)
    {
        show_warn("Global device list uninitialized!!!\n");
        return NULL;
    }
    else
        dev_list = global_dlist_manager;

    nmp_mutex_lock(dev_list->lock);
    prx_dev = __find_device_by_user_id(dev_list->list, user_id, sdk_name);
    nmp_mutex_unlock(dev_list->lock);

    return prx_dev;
}

proxy_device_t *
proxy_find_device_by_dev_ip(char *dev_ip)
{
    proxy_dlist_t *dev_list;
    proxy_device_t *prx_dev;

    NMP_ASSERT(dev_ip);

    if (!global_dlist_manager)
    {
        show_warn("Global device list uninitialized!!!\n");
        return NULL;
    }
    else
        dev_list = global_dlist_manager;

    nmp_mutex_lock(dev_list->lock);
    prx_dev = __find_device_by_dev_ip(dev_list->list, dev_ip);
    nmp_mutex_unlock(dev_list->lock);

    return prx_dev;
}

proxy_device_t *
proxy_find_device_by_pu_id(char *pu_id)
{
    proxy_dlist_t *dev_list;
    proxy_device_t *prx_dev;

    NMP_ASSERT(pu_id);

    if (!global_dlist_manager)
    {
        show_warn("Global device list uninitialized!!!\n");
        return NULL;
    }
    else
        dev_list = global_dlist_manager;

    nmp_mutex_lock(dev_list->lock);
    prx_dev = __find_device_by_pu_id(dev_list->list, pu_id);
    nmp_mutex_unlock(dev_list->lock);

    return prx_dev;
}


int proxy_get_device_private(proxy_device_t *dev, int type, void *pvalue)
{
    int ret;
    proxy_private_t *pri_data;

    NMP_ASSERT(dev && pvalue);

    if (type >= DEV_PRI_MAX || type <= DEV_PRI_MIN)
        return -E_PRITYPE;

    pri_data = (proxy_private_t*)dev->pri_data;
    if (pri_data)
    {
        nmp_mutex_lock(pri_data->lock);
        if (pri_data->data[type].offset)
        {
            ret = 0;
            memcpy(pvalue, ((char*)pri_data) + pri_data->data[type].offset, 
                pri_data->data[type].size);
        }
        else
            ret = -E_NO_PRIDATA;
        nmp_mutex_unlock(pri_data->lock);
    }
    else
        ret = -E_NO_PRIDATA;

    return ret;
}

int proxy_set_device_private(proxy_device_t *dev, int type, int size, void *pvalue)
{
    proxy_private_t *pri_data, *old = NULL;
    int new_size = 0, header_size, offset = 0;

    NMP_ASSERT(dev && pvalue);

    if (type >= DEV_PRI_MAX || type <= DEV_PRI_MIN)
        return -E_PRITYPE;

    header_size = ALIGN(sizeof(proxy_private_t), sizeof(void*));

    pri_data = (proxy_private_t*)dev->pri_data;
    if (!dev->pri_data)
    {
        pri_data = nmp_alloc0(header_size);
        pri_data->lock = nmp_mutex_new();
        dev->pri_data = (void*)pri_data;
    }

    nmp_mutex_lock(pri_data->lock);

    if (!pri_data->data[type].offset)
    {
        new_size = pri_data->total_size;
        new_size += header_size;
        new_size += ALIGN(size, sizeof(void*));
    }

    if (new_size)
    {
        old = (proxy_private_t*)dev->pri_data;
        offset = old->total_size;

        pri_data = (proxy_private_t*)nmp_alloc0(new_size);
        memcpy(pri_data, old, header_size + old->total_size);

        pri_data->total_size += ALIGN(size, sizeof(void*));
        dev->pri_data = pri_data;
    }

    //  pri_data = (proxy_private_t*)dev->pri_data;
    if (pri_data->data[type].offset)
    {
        if (pri_data->data[type].size != size)
            BUG();
        memcpy(((char*)pri_data) + pri_data->data[type].offset, pvalue, size);
    }
    else
    {
        pri_data->data[type].offset = header_size + offset;
        pri_data->data[type].size = size;
        memcpy(((char*)pri_data) + pri_data->data[type].offset, pvalue, size);
    }

    nmp_mutex_unlock(pri_data->lock);

    if (old)
        nmp_dealloc(old, header_size + old->total_size);

    return 0;
}

int proxy_get_device_state(proxy_device_t *dev)
{
    int state;
    NMP_ASSERT(dev);

    nmp_mutex_lock(dev->lock);
    state = dev->state.state;
    nmp_mutex_unlock(dev->lock);

    return state;
}

int proxy_set_device_state(proxy_device_t *dev, proxy_state_t *dev_state)
{
    int old;
    #define ASCENSIONAL_RANGE   1
    NMP_ASSERT(dev);

    old = proxy_get_device_state(dev);
    if ((old+ASCENSIONAL_RANGE) < dev_state->state)
    {//状态不允许向上跳级转变
        show_warn("Update Forbid[old:%d, new:%d]!!!!!!!!!!!!!!!!!!!\n", 
            old, dev_state->state);
        return STATE_UNKNOWN;
    }

    nmp_mutex_lock(dev->lock);
    if (__proxy_device_killed(dev))
    {
        nmp_mutex_unlock(dev->lock);
        return STATE_UNKNOWN;
    }

    dev->state.state = dev_state->state;
    dev->state.error = dev_state->error;
    nmp_mutex_unlock(dev->lock);

    if (dev_state->error || (old != dev_state->state))
        config_broadcast_device_state(dev, dev_state);

    return dev_state->state;
}

void proxy_deliver_device_msg(proxy_device_t *dev, msg_engine_t *me, msg_t *msg)
{
    int flag = 0;
    NMP_ASSERT(dev && msg);

    nmp_mutex_lock(dev->lock);
    if (!__proxy_device_killed(dev))
    {
        flag = 1;
    }
    nmp_mutex_unlock(dev->lock);

    if (flag && dev->sdk_srv)
    {
        deliver_msg(me, msg);
    }
}
void proxy_send_device_msg(proxy_device_t *dev, msg_t *msg)
{
    int flag = 0;
    NMP_ASSERT(dev && msg);

    nmp_mutex_lock(dev->lock);
    if (!__proxy_device_killed(dev))
    {
        flag = 1;
    }
    nmp_mutex_unlock(dev->lock);

    if (flag && dev->plt_srv)
    {
        send_msg(msg);
    }
}


