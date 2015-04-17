
#include <unistd.h>

#include "nmp_proxy_log.h"
#include "nmp_system_ctrl.h"
#include "nmp_resolve_host.h"

#include "nmp_proxy_server.h"
#include "nmp_proxy_device.h"


#define MAX_PROXY_THREAD_POOL_SIZE          32
#define EXIT_PROXY_SERVER(time, error)      do {sleep(time);exit(error);} while (0)


extern cfg_service_basic_t cfg_srv_basic;

extern int config_get_server_config(proxy_config_t *cfg);
extern int config_set_server_config(proxy_config_t *cfg);

struct proxy_ctrl
{
    struct service *srv;
    int    cmd;
    void  *user;
    size_t size;
    fin_t  fin;
};

struct proxy_task
{
    int    cmd;
    void  *data;
    size_t size;
    fin_t  fin;

    void *owner;        //proxy_device_t
};


int proxy_server_loop()
{
    while (1)
    {
        sleep(1);
        proxy_manage_device();
    }

    return 0;
}

#include "cfg/nmp_config_service.h"


static void 
proxy_broadcast_event(void *event)
{
    config_broadcast_event((cfg_service_basic_t*)&cfg_srv_basic, 
        (msg_t*)event);
}


static void 
process_proxy_thread(void *data, void *user)
{
    proxy_ctrl_t *ctrl = NULL;
    proxy_task_t *task = (proxy_task_t*)data;
    NMP_ASSERT(data);

    switch (task->cmd)
    {
        case BROADCAST_EVENT:
            proxy_broadcast_event(task->data);
            break;
        case CONTROL_DEVICE:
            ctrl = (proxy_ctrl_t*)task->data;
            control_service(ctrl->srv, ctrl->cmd, ctrl->user);
            break;
        case RESOLVE_HOST:
            resolve_host_proxy(task->data);
            break;
        case REBOOT_SERVER:
            proxy_destroy_all_device();
            cleanup_all_service_template();
            EXIT_PROXY_SERVER(3, 0);
            break;

        default:
            break;
    }

    proxy_free_task(task);
    return ;
}

void proxy_thread_pool_push(proxy_task_t *task)
{
    static nmp_threadpool_t *pool = NULL;
    if (!pool)
        pool = nmp_threadpool_new(process_proxy_thread, NULL, 
                    MAX_PROXY_THREAD_POOL_SIZE, NULL);

    nmp_threadpool_push(pool, (void*)task);
    return ;
}

proxy_ctrl_t *
proxy_new_ctrl(struct service *srv, int cmd, void *user)
{
    proxy_ctrl_t *ctrl;
    NMP_ASSERT(srv);

    ctrl = (proxy_ctrl_t*)nmp_new0(proxy_ctrl_t, 1);
    if (ctrl)
    {
        ctrl->srv = srv;
        ctrl->cmd = cmd;
        ctrl->user= user;
        ctrl->size= 0;
        ctrl->fin = NULL;
    }

    return ctrl;
}

proxy_ctrl_t *
proxy_new_ctrl_2(struct service *srv, int cmd, void *user, size_t size, fin_t fin)
{
    proxy_ctrl_t *ctrl;
    NMP_ASSERT(srv);
    NMP_ASSERT(!(user && !size) && !(user && !fin));

    ctrl = (proxy_ctrl_t*)nmp_new0(proxy_ctrl_t, 1);
    if (ctrl)
    {
        ctrl->srv = srv;
        ctrl->cmd = cmd;
        ctrl->user= user;
        ctrl->size= size;
        ctrl->fin = fin;
    }

    return ctrl;
}
void proxy_free_ctrl(void *data, size_t size)
{
    proxy_ctrl_t *ctrl = (proxy_ctrl_t*)data;
    NMP_ASSERT(ctrl);

    if (ctrl->user)
        if (ctrl->fin && ctrl->size)
            ctrl->fin(ctrl->user, ctrl->size);
    nmp_del(ctrl, proxy_ctrl_t, 1);
}

proxy_task_t *
proxy_new_task(int cmd, void *data, size_t size, fin_t fin, void *owner)
{
    proxy_task_t *task;
    NMP_ASSERT(!(data && !size) && !(data && !fin));

    task = (proxy_task_t*)nmp_new0(proxy_task_t, 1);
    if (task)
    {
        task->cmd  = cmd;
        task->data = data;
        task->size = size;
        task->fin  = fin;
        if (owner)
            task->owner= proxy_device_ref((proxy_device_t*)owner);
    }

    return task;
}
void proxy_free_task(proxy_task_t *task)
{
    NMP_ASSERT(task);

    if (task->data)
        task->fin(task->data, task->size);
    if (task->owner)
        proxy_device_unref((proxy_device_t*)task->owner);
    nmp_del(task, proxy_ctrl_t, 1);
}

//////////////////////////////////////////////////////////////////////////////
struct service *
create_new_service(void *init, int type, const char *name)
{
    struct service_template *tm;
    NMP_ASSERT(init && name);

    if ((tm = find_service_template_by_name((const char*)name)))
    {
        return (*tm->create_service)(tm, init);
    }
    else
        show_warn("service_template[%s] NULL!\n", (char*)name);

    return NULL;
}

void destory_service(struct service *srv)
{
    NMP_ASSERT(srv);

    if (srv->tm && srv->tm->delete_service)
    {
        (*srv->tm->delete_service)(srv->tm, srv);
    }
}

int check_service(struct service *srv)
{
    NMP_ASSERT(srv);
    return (*srv->tm->check_service)(srv->tm, srv);
}
int control_service(struct service *srv, int cmd, void *user)
{
    NMP_ASSERT(srv);
    return (*srv->tm->control_service)(srv->tm, srv, cmd, user);
}

int get_proxy_config(proxy_config_t *cfg)
{
    NMP_ASSERT(cfg);
    return config_get_server_config(cfg);
}
int set_proxy_config(proxy_config_t *cfg)
{
    NMP_ASSERT(cfg);
    return config_set_server_config(cfg);
}

