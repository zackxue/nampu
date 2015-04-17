
#include "nmp_config_info.h"
#include "nmp_config_backup.h"
#include "nmp_config_handler.h"
#include "nmp_proxy_device.h"



static void config_free_msg(void *data, size_t size)
{
    msg_t *msg = (msg_t*)data;
    NMP_ASSERT(msg);
    free_msg(msg);
}

static void 
config_destory_page_user_info(void *data, size_t size)
{
    NMP_ASSERT(data && size);
    nmp_dealloc(data, size);
}
static void 
config_destory_factory_st(void *data, size_t size)
{
    NMP_ASSERT(data && size);
    nmp_dealloc(data, size);
}
static void 
config_destory_page_device_info(void *data, size_t size)
{
    NMP_ASSERT(data && size);
    nmp_dealloc(data, size);
}

////////////////////////////////////////////////////////////////////////////////////////////

static int 
config_broadcast_add_user(config_service_t *cfg_srv, const char *username)
{
    msg_t *bc_msg;
    proxy_task_t *task;

    NMP_ASSERT(cfg_srv && username);

    bc_msg = alloc_new_msg(BROADCAST_ADD_USER_ID, (void*)username, strlen(username), 0);

    task = proxy_new_task(BROADCAST_EVENT, bc_msg, sizeof(msg_t), 
            config_free_msg, NULL);
    if (task)
        proxy_thread_pool_push(task);
    else
        config_free_msg((void*)bc_msg, sizeof(msg_t));

    return 0;
}
static int 
config_broadcast_del_user(config_service_t *cfg_srv, const char *username)
{
    msg_t *bc_msg;
    proxy_task_t *task;

    NMP_ASSERT(cfg_srv && username);

    bc_msg = alloc_new_msg(BROADCAST_DEL_USER_ID, (void*)username, strlen(username), 0);

    task = proxy_new_task(BROADCAST_EVENT, bc_msg, sizeof(msg_t), 
            config_free_msg, NULL);
    if (task)
        proxy_thread_pool_push(task);
    else
        config_free_msg((void*)bc_msg, sizeof(msg_t));

    return 0;
}
static int 
config_broadcast_add_device(config_service_t *cfg_srv, int dev_id)
{
    msg_t *bc_msg;
    proxy_task_t *task;

    NMP_ASSERT(cfg_srv);

    bc_msg = alloc_new_msg(BROADCAST_ADD_DEVICE_ID, (void*)&dev_id, sizeof(int), 0);

    task = proxy_new_task(BROADCAST_EVENT, bc_msg, sizeof(msg_t), 
            config_free_msg, NULL);
    if (task)
        proxy_thread_pool_push(task);
    else
        config_free_msg((void*)bc_msg, sizeof(msg_t));

    return 0;
}
static int 
config_broadcast_del_device(config_service_t *cfg_srv, int dev_id)
{
    msg_t *bc_msg;
    proxy_task_t *task;

    NMP_ASSERT(cfg_srv);

    bc_msg = alloc_new_msg(BROADCAST_DEL_DEVICE_ID, (void*)&dev_id, sizeof(int), 0);

    task = proxy_new_task(BROADCAST_EVENT, bc_msg, sizeof(msg_t), 
            config_free_msg, NULL);
    if (task)
        proxy_thread_pool_push(task);
    else
        config_free_msg((void*)bc_msg, sizeof(msg_t));

    return 0;
}
static int 
config_broadcast_modify_device(config_service_t *cfg_srv, int dev_id)
{
    msg_t *bc_msg;
    proxy_task_t *task;

    NMP_ASSERT(cfg_srv);

    bc_msg = alloc_new_msg(BROADCAST_MODIFY_DEVICE_ID, (void*)&dev_id, sizeof(int), 0);

    task = proxy_new_task(BROADCAST_EVENT, bc_msg, sizeof(msg_t), 
            config_free_msg, NULL);
    if (task)
        proxy_thread_pool_push(task);
    else
        config_free_msg((void*)bc_msg, sizeof(msg_t));

    return 0;
}

int 
config_broadcast_device_state(proxy_device_t *dev, proxy_state_t *dev_state)
{
    msg_t *bc_msg;
    proxy_task_t *task;
    prx_device_state prx_dev_state;

    prx_dev_state.dev_id = dev->fastenings.dev_id;
    prx_dev_state.dev_state = dev_state->state;
    prx_dev_state.err_code = dev_state->error;

    bc_msg = alloc_new_msg(BROADCAST_DEVICE_STATUS_ID, 
                (void*)&prx_dev_state, sizeof(prx_device_state), 0);

    show_info("Broadcast [dev_id: %d, state:%d, error:%d]\n", 
        dev->fastenings.dev_id, dev_state->state, dev_state->error);

    task = proxy_new_task(BROADCAST_EVENT, bc_msg, sizeof(msg_t), 
            config_free_msg, NULL);
    if (task)
        proxy_thread_pool_push(task);
    else
        config_free_msg((void*)bc_msg, sizeof(msg_t));

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
static int 
config_add_user_handle(void *init_data, msg_t *msg, int parm_id)
{
    int ret;
    JUserInfo *user_info;
    
    config_service_t *cfg_srv;
    config_user_info_t one_user;
    config_user_list_t *user_list;

    NMP_ASSERT(init_data && msg);

    BUG_ON(ADD_USER_REQUEST_ID != MSG_ID(msg)); 

    cfg_srv = (config_service_t*)init_data;
    user_info = (JUserInfo*)MSG_DATA(msg);
    user_list = config_get_user_list((struct service*)cfg_srv);

    one_user.status = LOGOUT;
    memcpy(one_user.username, user_info->username, 
        sizeof(one_user.username));
    memcpy(one_user.password, user_info->password, 
        sizeof(one_user.password));

    //检查是否有权限
    ret = config_check_user_is_admin((struct service*)cfg_srv);

    if (!ret)
    {
        ret = config_add_new_user_info(user_list, &one_user);
        if (!ret)
        {
            config_save_user_list(user_list);
            config_broadcast_add_user(cfg_srv, (const char*)user_info->username);
        }
    }

    set_msg_id(msg, ADD_USER_RESULT_ID);
    set_msg_data(msg, (void*)&ret, sizeof(int));

    return RET_BACK;
}
static int 
config_del_user_handle(void *init_data, msg_t *msg, int parm_id)
{
    int ret;
    JUserInfo *user_info;

    config_service_t *cfg_srv;
    config_user_list_t *user_list;

    NMP_ASSERT(init_data && msg);

    BUG_ON(DEL_USER_REQUEST_ID != MSG_ID(msg));

    cfg_srv = (config_service_t*)init_data;
    user_info = (JUserInfo*)MSG_DATA(msg);
    user_list = config_get_user_list((struct service*)cfg_srv);

    //检查是否有权限
    ret = config_check_user_is_admin((struct service*)cfg_srv);
    if (!ret)
    {
        if (!strcmp(DEF_ADMIN_USERNAME, (const char*)user_info->username))
            ret = -1;
        else
        {
            ret = config_remove_one_user_info(user_list, 
                    (const char*)user_info->username);
            if (!ret)
            {
                config_save_user_list(user_list);//TODO: 保存文件失败后，将用户重新加入到用户列表中
                config_broadcast_del_user(cfg_srv, (const char*)user_info->username);
            }
        }
    }

    set_msg_id(msg, DEL_USER_RESULT_ID);
    set_msg_data(msg, (void*)&ret, sizeof(int));

    return RET_BACK;
}
static int 
config_get_user_handle(void *init_data, msg_t *msg, int parm_id)
{
    int flag, arrary_count, count, i;
    int pri_size;
    void *pri_data;
    struct _prx_page_user *page_user_info;
    struct _prx_page_user *one_page_user;

    config_service_t *cfg_srv;
    config_user_info_t *user_array;
    config_user_list_t *user_list;
    config_user_factor_t user_factor;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_USER_INFO_REQUEST_ID != MSG_ID(msg));

    flag = 0;
    memset(&user_factor, 0, sizeof(config_user_factor_t));

    cfg_srv = (config_service_t*)init_data;
    page_user_info = (struct _prx_page_user*)MSG_DATA(msg);
    user_list = config_get_user_list((struct service*)cfg_srv);

    if (MAX_USER_PAGE_SIZE > page_user_info->count)
        ;
    else
        page_user_info->count = MAX_USER_PAGE_SIZE;

    if (0 < page_user_info->count)
    {
        arrary_count = page_user_info->count;
        user_array = nmp_new0(config_user_info_t, arrary_count);
        
        user_factor.offset = page_user_info->offset;
        user_factor.count = page_user_info->count;
        strncpy(user_factor.username, page_user_info->username, 
            sizeof(user_factor.username)-1);
        user_factor.username[sizeof(user_factor.username)-1] = '\0';

        count = config_get_one_page_user_info(user_list, 
                user_array, &user_factor);

        if (count)
        {
            pri_size = sizeof(struct _prx_page_user) + sizeof(JUserInfo)*count;
            pri_data = nmp_alloc(pri_size);
            
            one_page_user = (struct _prx_page_user*)pri_data;
            one_page_user->user_info = pri_data + sizeof(struct _prx_page_user);
            
            one_page_user->count = count;
            one_page_user->total = user_factor.total;
            one_page_user->offset = user_factor.offset;
            strncpy(one_page_user->username, user_factor.username, 
                sizeof(one_page_user->username)-1);
            one_page_user->username[sizeof(one_page_user->username)-1] = '\0';

            for (i=0; i<one_page_user->count; i++)
            {
                strncpy((char*)one_page_user->user_info[i].username, 
                    user_array[i].username, 
                    sizeof(one_page_user->user_info[i].username)-1);
                one_page_user->user_info[i].username[
                    sizeof(one_page_user->user_info[i].username)-1] = '\0';
                printf("user: %s\n", one_page_user->user_info[i].username);
            }

            set_msg_id(msg, GET_USER_INFO_RESPONSE_ID);
            set_msg_data_2(msg, (void*)pri_data, 
                    pri_size, config_destory_page_user_info);
        }
        else
            flag = -1;

        nmp_del(user_array, config_user_info_t, arrary_count);
    }
    else
    {
        show_debug("Count invalid![Get info]\n");
        flag = -1;
    }

    if (-1 == flag)
    {
        set_msg_id(msg, GET_USER_INFO_RESPONSE_ID);
        memset(page_user_info, 0, sizeof(struct _prx_page_user));
    }

    return RET_BACK;    
}
static int 
config_set_user_handle(void *init_data, msg_t *msg, int parm_id)
{
    int ret = -1, flag = -1;
    struct _prx_modify_pwd *modify;
    
    config_service_t *cfg_srv;
    config_user_list_t *user_list;
    config_user_info_t *cur_user;
    config_user_info_t new_user;

    NMP_ASSERT(init_data && msg);

    BUG_ON(MODIFY_PASSWORD_REQUEST_ID != MSG_ID(msg));

    memset(&new_user, 0, sizeof(config_user_info_t));
    
    cfg_srv = (config_service_t*)init_data;
    modify = (struct _prx_modify_pwd*)MSG_DATA(msg);
    user_list = config_get_user_list((struct service*)cfg_srv);
    cur_user = &cfg_srv->client.user_info;

    //检查是否有权限
    if (!config_check_user_is_admin((struct service*)cfg_srv) || 
        !strcmp(cur_user->username, modify->username))
    {
        if (!strcmp(cur_user->password, modify->old_pwd))
            flag = 0;
    }

    if (!flag)
    {
        strncpy(new_user.username, modify->username, 
            sizeof(new_user.username)-1);
        new_user.username[sizeof(new_user.username)-1] = '\0';
        strncpy(new_user.password, modify->new_pwd, 
            sizeof(new_user.password)-1);
        new_user.password[sizeof(new_user.password)-1] = '\0';
        if (!(ret = config_modify_user_password(user_list, &new_user)))
        {
            if (!strcmp(cur_user->username, new_user.username))
            {
                strncpy(cur_user->password, 
                    new_user.password, sizeof(cur_user->password)-1);
                cur_user->password[sizeof(cur_user->password)-1] = '\0';
            }
            config_save_user_list(user_list);
        }
    }

    set_msg_id(msg, MODIFY_PASSWORD_RESULT_ID);
    set_msg_data(msg, (void*)&ret, sizeof(int));

    return RET_BACK;
}
static int 
config_add_device_handle(void *init_data, msg_t *msg, int parm_id)
{
    int dev_id;
    struct _prx_device_info *dev_info;

    config_service_t *cfg_srv;
    config_device_info_t one_dev;
    config_device_list_t *dev_list;

    NMP_ASSERT(init_data && msg);

    BUG_ON(ADD_DEVICE_REQUEST_ID != MSG_ID(msg));

    cfg_srv = (config_service_t*)init_data;
    dev_list = config_get_device_list((struct service*)cfg_srv);
    dev_info = (struct _prx_device_info*)MSG_DATA(msg);

    memcpy(&one_dev.dev_info, dev_info, sizeof(prx_device_info));
    dev_id = config_add_new_device_info(dev_list, &one_dev);
    if (-1 != dev_id)
    {
        config_save_device_list(dev_list);
        config_broadcast_add_device(cfg_srv, dev_id);
    }

    set_msg_id(msg, ADD_DEVICE_RESULT_ID);
    set_msg_data(msg, (void*)&dev_id, sizeof(int));

    return RET_BACK;
}
static int 
config_del_device_handle(void *init_data, msg_t *msg, int parm_id)
{
    int ret, dev_id;
    
    config_service_t *cfg_srv;
    config_device_list_t *dev_list;

    NMP_ASSERT(init_data && msg);

    BUG_ON(DEL_DEVICE_REQUEST_ID != MSG_ID(msg));

    cfg_srv = (config_service_t*)init_data;
    dev_list = config_get_device_list((struct service*)cfg_srv);
    dev_id = *((int*)MSG_DATA(msg));
    
    ret = config_remove_one_device_info(dev_list, dev_id);
    if (!ret)
    {
        config_save_device_list(dev_list);
        config_broadcast_del_device(cfg_srv, dev_id);
    }
    
    set_msg_id(msg, DEL_DEVICE_RESULT_ID);
    set_msg_data(msg, (void*)&ret, sizeof(int));
    
    return RET_BACK;
}
static int 
config_get_device_handle(void *init_data, msg_t *msg, int parm_id)
{
    int flag = 0, count, i, size;
    int dev_info_size;
    struct _prx_page_device *page_dev_info;
    struct _prx_page_device *one_page_dev;

    config_service_t *cfg_srv;
    config_device_info_t *dev_info;
    config_device_list_t *dev_list;
    struct config_device_factor dev_factor;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_DEVICE_INFO_REQUEST_ID != MSG_ID(msg));

    memset(&dev_factor, 0, sizeof(struct config_device_factor));
    
    cfg_srv = (config_service_t*)init_data;
    dev_list = config_get_device_list((struct service*)cfg_srv);
    page_dev_info = (struct _prx_page_device*)MSG_DATA(msg);
    
    if (MAX_DEVICE_PAGE_SIZE > page_dev_info->count)
        ;
    else
        page_dev_info->count = MAX_DEVICE_PAGE_SIZE;

    if (0 < page_dev_info->count)
    {
        dev_info_size = sizeof(config_device_info_t) * page_dev_info->count;
        dev_info = nmp_alloc0(dev_info_size);

        dev_factor.offset = page_dev_info->offset;
        dev_factor.count = page_dev_info->count;
        dev_factor.dev_id = page_dev_info->dev_id;
        dev_factor.machine_type = page_dev_info->machine_type;
        strncpy(dev_factor.sdk_version, page_dev_info->sdk_version, 
            sizeof(dev_factor.sdk_version)-1);
        dev_factor.sdk_version[sizeof(dev_factor.sdk_version)-1] = '\0';
        strncpy(dev_factor.factory_name, page_dev_info->factory_name, 
            sizeof(dev_factor.factory_name)-1);
        dev_factor.factory_name[sizeof(dev_factor.factory_name)-1] = '\0';

        count = config_get_one_page_device_info(dev_list, 
                    dev_info, &dev_factor);

        if (count)
        {
            size = sizeof(struct _prx_page_device) + sizeof(struct _prx_device_info)*count;

            one_page_dev = nmp_alloc0(size);
            one_page_dev->dev_info = (prx_device_info*)((char*)one_page_dev + sizeof(prx_page_device));

            one_page_dev->count = count;
            one_page_dev->total = dev_factor.total;
            one_page_dev->offset = dev_factor.offset;
            one_page_dev->dev_id = dev_factor.dev_id;
            one_page_dev->machine_type = dev_factor.machine_type;
            strncpy(one_page_dev->sdk_version, dev_factor.sdk_version, 
                sizeof(one_page_dev->sdk_version)-1);
            one_page_dev->sdk_version[sizeof(one_page_dev->sdk_version)-1] = '\0';
            strncpy(one_page_dev->factory_name, dev_factor.factory_name, 
                sizeof(one_page_dev->factory_name)-1);
            one_page_dev->factory_name[sizeof(one_page_dev->factory_name)-1] = '\0';

            for (i=0; i<one_page_dev->count; i++)
            {
                memcpy(&one_page_dev->dev_info[i], &dev_info[i].dev_info, 
                    sizeof(struct _prx_device_info));
            }

            set_msg_id(msg, GET_DEVICE_INFO_RESULT_ID);
            set_msg_data_2(msg, (void*)one_page_dev, 
                    size, config_destory_page_device_info);
        }
        else
        {
            flag = -1;
            show_debug("Count invalid[device info]!<--------------\n");
        }

        nmp_dealloc(dev_info, dev_info_size);
    }
    else
    {
        flag = -1;
        show_debug("Count invalid[Get info]!<--------------\n");
    }

    if (-1 == flag)
    {
        set_msg_id(msg, GET_DEVICE_INFO_RESULT_ID);
        memset(page_dev_info, 0, sizeof(struct _prx_page_device));
    }

    return RET_BACK;    
}
static int 
config_set_device_handle(void *init_data, msg_t *msg, int parm_id)
{
    int ret;
    struct _prx_device_info *dev_info;

    config_service_t *cfg_srv;
    config_device_info_t one_dev;
    config_device_list_t *dev_list;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_DEVICE_INFO_REQUEST_ID != MSG_ID(msg));

    cfg_srv = (config_service_t*)init_data;
    dev_list = config_get_device_list((struct service*)cfg_srv);
    dev_info = (struct _prx_device_info*)MSG_DATA(msg);

    memcpy(&one_dev.dev_info, dev_info, sizeof(prx_device_info));
    
    ret = config_modify_device_info(dev_list, &one_dev);
    if (!ret)
    {
        config_save_device_list(dev_list);
        config_broadcast_modify_device(cfg_srv, dev_info->device_id);
    }

    set_msg_id(msg, SET_DEVICE_INFO_RESULT_ID);
    set_msg_data(msg, (void*)&ret, sizeof(int));

    return RET_BACK;
}

static int 
config_get_factory_handle(void *init_data, msg_t *msg, int parm_id)
{
    nmp_list_t *list;
    nmpio_t *net_io;
    int count, size, i;
    struct _prx_factory_list *factory_st;

    config_service_t *cfg_srv;
    config_factory_info_t *fct_info;
    config_factory_list_t *fct_list;

    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_FACTORY_REQUEST_ID != MSG_ID(msg));

    net_io = MSG_IO(msg);
    cfg_srv = (config_service_t*)init_data;
    fct_list = config_get_factory_list((struct service*)cfg_srv);

    count = 0;
    nmp_mutex_lock(fct_list->lock);
    
    list = nmp_list_first(fct_list->list);
    while (list)
    {
        count++;
        list = nmp_list_next(list);
    }

    size = sizeof(struct _prx_factory_list) + sizeof(struct _prx_factory_info)*count;
    factory_st = nmp_alloc0(size);

    factory_st->count = count;
    factory_st->factory = (prx_factory_info*)((char*)factory_st + sizeof(prx_factory_list));

    list = nmp_list_first(fct_list->list);
    for (i=0; i<factory_st->count; i++)
    {
        fct_info = (config_factory_info_t*)nmp_list_data(list);
        memcpy(&factory_st->factory[i], &fct_info->fct_info, 
            sizeof(struct _prx_factory_info));
        
        list = nmp_list_next(list);
    }
    
    nmp_mutex_unlock(fct_list->lock);

    set_msg_id(msg, GET_FACTORY_RESPONSE_ID);
    set_msg_data_2(msg, (void*)factory_st, size, config_destory_factory_st);
    
    return RET_BACK;
}
static int 
config_get_server_config_handle(void *init_data, 
    msg_t *msg, int parm_id)
{
    proxy_config_t local_cfg;
    config_service_t *cfg_srv;
    prx_server_config *srv_cfg;
    
    NMP_ASSERT(init_data && msg);

    BUG_ON(GET_SERVER_CONFIG_REQUEST_ID != MSG_ID(msg));    
    
    cfg_srv = (config_service_t*)init_data;
    srv_cfg = (prx_server_config*)MSG_DATA(msg);

    config_get_server_config(&local_cfg);
    strcpy(srv_cfg->server_ip, local_cfg.host);
    srv_cfg->listen_port = local_cfg.cmd_port;
    srv_cfg->rtsp_port = local_cfg.rtsp_port;
    
    set_msg_id(msg, GET_SERVER_CONFIG_RESPONSE_ID);
    
    return RET_BACK;
}
static int 
config_set_server_config_handle(void *init_data, 
    msg_t *msg, int parm_id)
{
    int ret;
    proxy_config_t local_cfg;
    config_service_t *cfg_srv;
    prx_server_config *srv_cfg;

    NMP_ASSERT(init_data && msg);

    BUG_ON(SET_SERVER_CONFIG_REQUEST_ID != MSG_ID(msg));    

    cfg_srv = (config_service_t*)init_data;
    srv_cfg = (prx_server_config*)MSG_DATA(msg);
    
    strcpy(local_cfg.host, srv_cfg->server_ip);
    local_cfg.cmd_port = srv_cfg->listen_port;
    local_cfg.rtsp_port = srv_cfg->rtsp_port;
    ret = config_set_server_config(&local_cfg);
    if (1 == ret)
    {
        proxy_task_t *task;
        task = proxy_new_task(REBOOT_SERVER, NULL, 0, NULL, NULL);
        if (task)
        {
            proxy_thread_pool_push(task);
            ret = 0;
        }
    }
    else if (0 == ret)
        ;//没有变化，直接返回
    
    set_msg_id(msg, SET_SERVER_CONFIG_RESULT_ID);
    set_msg_data(msg, (void*)&ret, sizeof(int));

    return RET_BACK;
}

static int 
config_backup_handle(void *init_data, msg_t *msg, int parm_id)
{
    int len, ret = -1;
    int port, listener;
    prx_backup *backup;
    config_backup_t *cfg_bkp;
    config_service_t *cfg_srv;

    NMP_ASSERT(init_data && msg);

    BUG_ON(DOWNLOAD_DATA_REQUEST_ID != MSG_ID(msg));

    cfg_srv = (config_service_t*)init_data;
    backup  = (prx_backup*)MSG_DATA(msg);
    cfg_bkp = get_config_backup();

    if (NORMAL != try_change_backup_state(cfg_bkp, BACKUP))
    {
        len = config_figure_up_file_len(cfg_bkp);
        if (len)
        {
            backup->size = len;
            port = config_create_backup_listener(&listener);
            if (port > 0)
            {
                ret = 0;
                backup->magic = DEF_BACKUP_MAGIC;
                backup->port  = port;

                config_start_backup_listen_thread(listener);
            }
        }
    }
    else
        show_warn("Somebody backing up!!!\n");

    backup->result = ret;
    set_msg_id(msg, DOWNLOAD_DATA_RESPONSE_ID);

    return RET_BACK;
}
static int 
config_revert_handle(void *init_data, msg_t *msg, int parm_id)
{
    int ret = -1;
    int port, listener;
    prx_backup *backup;
    config_backup_t *cfg_bkp;
    config_service_t *cfg_srv;

    NMP_ASSERT(init_data && msg);

    BUG_ON(UPLOAD_DATA_REQUEST_ID != MSG_ID(msg));

    cfg_srv = (config_service_t*)init_data;
    backup  = (prx_backup*)MSG_DATA(msg);
    cfg_bkp = get_config_backup();

    if (NORMAL != try_change_backup_state(cfg_bkp, REVERT))
    {
        if (0 < backup->size)
        {
            port = config_create_backup_listener(&listener);
            if (port > 0)
                ret = 0;
        }

        if (!ret)
        {
            nmp_mutex_lock(cfg_bkp->lock);
            cfg_bkp->state= REVERT;
            cfg_bkp->left = backup->size;
            nmp_mutex_unlock(cfg_bkp->lock);

            backup->magic = DEF_REVERT_MAGIC;
            backup->port  = port;

            config_start_backup_listen_thread(listener);
        }
    }
    else
        show_warn("Somebody backing up!!!\n");

    backup->result = ret;
    set_msg_id(msg, UPLOAD_DATA_RESPONSE_ID);

    return RET_BACK;
}
static int 
config_limit_broadcast_handle(void *init_data, msg_t *msg, int parm_id)
{
    prx_limit *limit;
    config_service_t *cfg_srv;

    NMP_ASSERT(init_data && msg);

    BUG_ON(LIMIT_BROADCASE_STATUE_ID != MSG_ID(msg));    

    cfg_srv = (config_service_t*)init_data;
    limit = (prx_limit*)MSG_DATA(msg);

    nmp_mutex_lock(cfg_srv->client.lock);
    cfg_srv->client.range.start = limit->start;
    cfg_srv->client.range.end   = limit->end;
    nmp_mutex_unlock(cfg_srv->client.lock);

    return RET_DROP;
}

void config_register_all_msg_handlers(msg_engine_t *me)
{
    NMP_ASSERT(me);

    register_msg_handler(me, ADD_USER_REQUEST_ID, HANDLE_ADD_USER, config_add_user_handle);
    register_msg_handler(me, DEL_USER_REQUEST_ID, HANDLE_DEL_USER, config_del_user_handle);
    register_msg_handler(me, GET_USER_INFO_REQUEST_ID, HANDLE_GET_USER, config_get_user_handle);
    register_msg_handler(me, MODIFY_PASSWORD_REQUEST_ID, HANDLE_SET_USER, config_set_user_handle);

    register_msg_handler(me, ADD_DEVICE_REQUEST_ID, HANDLE_ADD_DEVICE, config_add_device_handle);
    register_msg_handler(me, DEL_DEVICE_REQUEST_ID, HANDLE_DEL_DEVICE, config_del_device_handle);
    register_msg_handler(me, GET_DEVICE_INFO_REQUEST_ID, HANDLE_GET_DEVICE, config_get_device_handle);
    register_msg_handler(me, SET_DEVICE_INFO_REQUEST_ID, HANDLE_SET_DEVICE, config_set_device_handle);

    register_msg_handler(me, GET_FACTORY_REQUEST_ID, HANDLE_GET_FACTORY, config_get_factory_handle);

    register_msg_handler(me, GET_SERVER_CONFIG_REQUEST_ID, HANDLE_GET_CONFIG, config_get_server_config_handle);
    register_msg_handler(me, SET_SERVER_CONFIG_REQUEST_ID, HANDLE_SET_CONFIG, config_set_server_config_handle);

    register_msg_handler(me, DOWNLOAD_DATA_REQUEST_ID, HANDLE_BACKUP, config_backup_handle);
    register_msg_handler(me, UPLOAD_DATA_REQUEST_ID, HANDLE_REVERT, config_revert_handle);

    register_msg_handler(me, LIMIT_BROADCASE_STATUE_ID, HANDLE_LIMIT, config_limit_broadcast_handle);
}


