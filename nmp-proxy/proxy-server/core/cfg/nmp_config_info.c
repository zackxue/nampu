//#include <unistd.h>
//#include <string.h>

#include "nmp_proxy_rw_file.h"
#include "nmp_system_ctrl.h"
#include "nmp_config_service.h"
#include "nmp_config_srv_impl.h"

#include "nmp_config_info.h"


#define MAX_WRITE_XML_LEN           (2*4096)

#define MIN_DEVICE_USER_PORT        1024
#define DEFAULT_LISTEN_PORT         8551
#define DEFAULT_DATA_PORT           6554


#define TEMP_USER_INFO_FILE         "temp_user_info.xml"
#define TEMP_DEVICE_INFO_FILE       "temp_device_info.xml"
#define TEMP_FACTORY_INFO_FILE      "temp_factory_info.xml"
#define TEMP_SERVER_CONFIG_FILE     "temp_server_config.xml"



static __inline__ void 
config_show_user_list(config_user_list_t *user_list)
{
    int index = 0;
    nmp_list_t *list;
    config_user_info_t *user_info;
    
    NMP_ASSERT(user_list);

    nmp_mutex_lock(user_list->lock);  
    list = nmp_list_first(user_list->list);
    while (list)
    {
        user_info = (config_user_info_t*)nmp_list_data(list);

        printf("user info [%d]\n", index++ +1);
        printf("    status  : %d\n", user_info->status);
        printf("    username: %s\n", user_info->username);
        printf("    password: %s\n", user_info->password);

        list = nmp_list_next(list);
    }
    nmp_mutex_unlock(user_list->lock);
    
    return ;
}
static __inline__ void 
config_show_factory_list(config_factory_list_t *fct_list)
{
    int i = 0;
    nmp_list_t *list;
    config_factory_info_t *fct_info;
    
    NMP_ASSERT(fct_list);

    nmp_mutex_lock(fct_list->lock);
    
    list = nmp_list_first(fct_list->list);
    while (list)
    {
        fct_info = (config_factory_info_t*)nmp_list_data(list);
        
        printf("factory info [%d]\n", i++ +1);
        printf("    name      : %s\n", fct_info->fct_info.factory_name);
        int n, m;
        for (n=0; n<fct_info->fct_info.type_count; n++)
        {
            printf("    machine   : %d\n", fct_info->fct_info.type[n]);
            for (m=0; m<fct_info->fct_info.ver_count[n]; m++)
            {
                printf("        sdkVersion: %s\n", fct_info->fct_info.sdk_version[n][m]);
            }
        }

        list = nmp_list_next(list);
    }
    nmp_mutex_unlock(fct_list->lock);
    
    return ;
}
static __inline__ void 
config_show_device_list(config_device_list_t *dev_list)
{
    int i = 0;
    nmp_list_t *list;
    config_device_info_t *dev_info;
    
    NMP_ASSERT(dev_list);

    nmp_mutex_lock(dev_list->lock);
    
    list = nmp_list_first(dev_list->list);
    while (list)
    {
        dev_info = (config_device_info_t*)nmp_list_data(list);
        
        printf("device info [%d]\n", i++ +1);
        printf("    device_id     : %d\n", dev_info->dev_info.device_id);
        printf("    pu_type       : %d\n", dev_info->dev_info.pu_type);
        printf("    device_port   : %d\n", dev_info->dev_info.device_port);
        printf("    platform_port : %d\n", dev_info->dev_info.platform_port);
        printf("    pu_id         : %s\n", dev_info->dev_info.pu_id);
        printf("    factory       : %s\n", dev_info->dev_info.factory);
        printf("    sdk_version   : %s\n", dev_info->dev_info.sdk_version);
        printf("    device_ip     : %s\n", dev_info->dev_info.device_ip);
        printf("    platform_ip   : %s\n", dev_info->dev_info.platform_ip);
        printf("    username      : %s\n", dev_info->dev_info.username);
        printf("    password      : %s\n", dev_info->dev_info.password);

        list = nmp_list_next(list);
    }
    nmp_mutex_unlock(dev_list->lock);
    
    return ;
}

static int config_read_user_from_file(const char *file_name, 
        config_user_list_t *user_list)
{
    char *buffer;
    int size, err;
    
    prx_user_st *user_st;
    NmpXmlMsg *xml_msg;
    
    config_user_info_t user_info;

    NMP_ASSERT(file_name && user_list);
    
    size = read_file(&buffer, file_name);
    if (0 < size)
    {
        xml_msg = parse_xml(buffer, size, &err, 0);
        nmp_del(buffer, char, size);

        if (xml_msg)
        {
            if (xml_msg->priv_obj)
            {
                int i;
                user_st = xml_msg->priv_obj;

                if (!user_st->count)
                    goto ADD_DEF_ADMIN;

                for (i=0; i<user_st->count; i++)
                {
                    memset(&user_info, 0, sizeof(user_info));
                    user_info.status = LOGOUT;
                    strncpy(user_info.username, (const char*)user_st->user_info[i].username, 
                        sizeof(user_info.username)-1);
                    user_info.username[sizeof(user_info.username)-1] = '\0';
                    strncpy(user_info.password, (const char*)user_st->user_info[i].password, 
                        sizeof(user_info.password)-1);
                    user_info.password[sizeof(user_info.password)-1] = '\0';

                    config_add_new_user_info(user_list, &user_info);
                }
            }
            nmp_xml_msg_destroy(xml_msg);
        }
        else
        {
            goto ADD_DEF_ADMIN;
        }
    }
    else
        goto ADD_DEF_ADMIN;

    //config_show_user_list(user_list);
    return 0;

ADD_DEF_ADMIN:
    memset(&user_info, 0, sizeof(user_info));
    user_info.status = LOGOUT;
    snprintf(user_info.username, sizeof(user_info.username)-1, "admin");
    snprintf(user_info.password, sizeof(user_info.password)-1, "Admin");
    
    config_add_new_user_info(user_list, &user_info);
    config_save_user_list(user_list);

    return 0;
}
static int config_read_device_from_file(const char *file_name, 
        config_device_list_t *dev_list)
{
    char *buffer;
    int size, ret = -1;

    NmpXmlMsg *xml_msg;
    prx_device_st *dev_st;

    config_device_info_t dev_info;

    NMP_ASSERT(file_name && dev_list);

    size = read_file(&buffer, file_name);
    if (0 < size)
    {
        xml_msg = parse_xml(buffer, size, &ret, 0);
        if (xml_msg && xml_msg->priv_obj)
        {
            int i;
            dev_st = xml_msg->priv_obj;
            for (i=0; i<dev_st->count; i++)
            {
                memset(&dev_info, 0, sizeof(dev_info));
                memcpy(&dev_info.dev_info, &dev_st->device_info[i], 
                    sizeof(prx_device_info));
                prx_device_info *di = &dev_info.dev_info;
                printf("id: %d, puid: %s, ip: %s\n", 
                    di->device_id, 
                    di->pu_id, 
                    di->device_ip);

                config_add_new_device_info(dev_list, &dev_info);
            }
            //config_show_device_list(dev_list);
            ret = 0;
        }

        if (xml_msg)
            nmp_xml_msg_destroy(xml_msg);
        
        nmp_del(buffer, char, size);
    }

    return ret;
}
static int config_read_factory_from_file(const char *file_name, 
        config_factory_list_t *fct_list)
{
    char *buffer;
    int size, ret = -1;

    prx_factory_list *factory_st;
    NmpXmlMsg *xml_msg;

    config_factory_info_t fct_info;

    NMP_ASSERT(file_name && fct_list);

    size = read_file(&buffer, file_name);
    if (0 < size)
    {
        xml_msg = parse_xml(buffer, size, &ret, 0);
        if (xml_msg && xml_msg->priv_obj)
        {
            int i;
            factory_st = xml_msg->priv_obj;
            for (i=0; i<factory_st->count; i++)
            {
                memset(&fct_info.fct_info, 0, sizeof(fct_info.fct_info));
                memcpy(&fct_info.fct_info, &factory_st->factory[i], 
                    sizeof(prx_factory_info));
                
                config_add_new_factory_info(fct_list, &fct_info);
            }
            //config_show_factory_list(fct_list);
            ret = 0;
        }

        if (xml_msg)
            nmp_xml_msg_destroy(xml_msg);
        
        nmp_del(buffer, char, size);
    }

    return ret;
}
static int config_read_server_config_from_file(const char *file_name, 
        prx_server_config *srv_cfg)
{
    char *buffer;
    int size, ret=-1;

    NmpXmlMsg *xml_msg;

    NMP_ASSERT(file_name);

    size = read_file(&buffer, file_name);
    if (0 < size)
    {
        xml_msg = parse_xml(buffer, size, &ret, 0);
        if (xml_msg && XML_MSG_DATA(xml_msg))
        {
            if (SET_SERVER_CONFIG_REQUEST_ID == XML_MSG_ID(xml_msg))
            {
                ret = 0;
                memcpy(srv_cfg, XML_MSG_DATA(xml_msg), sizeof(prx_server_config));
                show_debug("server_ip  : %s\n", srv_cfg->server_ip);
                show_debug("listen_port: %d\n", srv_cfg->listen_port);
                show_debug("rtsp_port  : %d\n", srv_cfg->rtsp_port);
            }
        }

        if (xml_msg)
            nmp_xml_msg_destroy(xml_msg);
        nmp_del(buffer, char, size);
    }

    return ret;
}

static int config_write_user_to_file(const char *file_name, 
        config_user_list_t *user_list)
{
    nmp_list_t *list;
    int size, ret;
    int index, count;

    char buffer[MAX_WRITE_XML_LEN];
    NmpXmlMsg *xml_msg;

    struct _prx_user_st prx_user_list;
    JUserInfo *prx_one_user;

    config_user_info_t *one_user;

    NMP_ASSERT(file_name && user_list);

    count = 0;
    index = 0;
    memset(&prx_user_list, 0, sizeof(prx_user_st));

    nmp_mutex_lock(user_list->lock);

    list = nmp_list_first(user_list->list);
    while (list)
    {
        count++;
        list = nmp_list_next(list);
    }

    prx_user_list.count = count;
    prx_user_list.user_info = nmp_new0(JUserInfo, count);

    list = nmp_list_first(user_list->list);
    while (list)
    {
        one_user = nmp_list_data(list);

        prx_one_user = &prx_user_list.user_info[index++];
        strncpy((char*)prx_one_user->username, one_user->username, 
            sizeof(prx_one_user->username)-1);
        prx_one_user->username[sizeof(prx_one_user->username)-1] = '\0';
        strncpy((char*)prx_one_user->password, one_user->password, 
            sizeof(prx_one_user->password)-1);
        prx_one_user->password[sizeof(prx_one_user->password)-1] = '\0';

        list = nmp_list_next(list);
    }

    nmp_mutex_unlock(user_list->lock);

    xml_msg = nmp_xml_msg_new(USER_LIST_INFO_ID, (void*)&prx_user_list, sizeof(prx_user_st));
    size = create_xml(xml_msg, buffer, sizeof(buffer), 0);
    if (MAX_WRITE_XML_LEN > size)
    {
        if (0 < write_file(buffer, size, file_name))
            ret = 0;
        else
            ret = -1;
    }
    else
        ret = -1;

    nmp_del(prx_user_list.user_info, JUserInfo, count);
    nmp_xml_msg_destroy(xml_msg);

config_show_user_list(user_list);

    return ret;
}
static int config_write_device_to_file(const char *file_name, 
        config_device_list_t *dev_list)
{
    nmp_list_t *list;
    int size, len, ret;
    int index, count;
    char *buffer;
    NmpXmlMsg *xml_msg;

    struct _prx_device_st prx_dev_list;
    struct _prx_device_info *prx_one_dev;
    
    config_device_info_t *one_dev;
    
    NMP_ASSERT(file_name && dev_list);
    
    count = 0;
    index = 0;
    memset(&prx_dev_list, 0, sizeof(prx_device_st));
    
    nmp_mutex_lock(dev_list->lock);

    list = nmp_list_first(dev_list->list);
    while (list)
    {
        count++;
        list = nmp_list_next(list);
    }

    prx_dev_list.count = count;
    prx_dev_list.device_info = nmp_new0(struct _prx_device_info, count);

    list = nmp_list_first(dev_list->list);
    while (list)
    {
        one_dev = nmp_list_data(list);

        prx_one_dev = &prx_dev_list.device_info[index++];
        memcpy(prx_one_dev, &one_dev->dev_info, sizeof(struct _prx_device_info));
        
        list = nmp_list_next(list);
    }
    
    nmp_mutex_unlock(dev_list->lock);
    
    xml_msg = nmp_xml_msg_new(DEVICE_LIST_INFO_ID, 
        (void*)&prx_dev_list, sizeof(prx_device_st));
    
    size = 200 + count * 400;
    buffer = nmp_alloc(size);
    len = create_xml(xml_msg, buffer, size, 0);
    printf("size[%d], len[%d]\n", size, len);
    if (len > 0 && size > len)
    {
        if (0 < write_file(buffer, len, file_name))
            ret = 0;
        else
            ret = -1;
    }
    else
        ret = -1;
    
    nmp_dealloc(buffer, size);
    nmp_del(prx_dev_list.device_info, struct _prx_device_info, count);
    nmp_xml_msg_destroy(xml_msg);
    
    return ret;
}
static int config_write_factory_to_file(const char *file_name, 
        config_factory_list_t *fct_list)
{
    return 0;
}
static int config_write_server_config_to_file(const char *file_name, 
        prx_server_config *srv_cfg)
{
    int size, ret;
    
    NmpXmlMsg *xml_msg;
    char buffer[MAX_WRITE_XML_LEN];
    
    NMP_ASSERT(file_name && srv_cfg);

    xml_msg = nmp_xml_msg_new(SET_SERVER_CONFIG_REQUEST_ID, 
        (void*)srv_cfg, sizeof(prx_server_config));
    
    size = create_xml(xml_msg, buffer, sizeof(buffer), 0);
    if (MAX_WRITE_XML_LEN > size)
    {
        if (0 < write_file(buffer, size, file_name))
            ret = 0;
        else
            ret = -1;
    }
    else
        ret = -1;

    nmp_xml_msg_destroy(xml_msg);

    return ret;
}

static __inline__ config_user_info_t *
__config_find_user_info(config_user_list_t *user_list, 
        const char *username)
{
    nmp_list_t *list;
    config_user_info_t *one_user;

    list = nmp_list_first(user_list->list);
    while (list)
    {
        one_user = nmp_list_data(list);
        if (!strcmp((const char*)one_user->username, username))
        {
            return one_user;
        }
        list = nmp_list_next(list); 
    }

    return NULL;
}
static __inline__ config_device_info_t *
__config_find_device_by_id(config_device_list_t *dev_list, 
        int dev_id)
{
    nmp_list_t *list;
    config_device_info_t *one_dev;

    list = nmp_list_first(dev_list->list);
    while (list)
    {
        one_dev = nmp_list_data(list);
        if (dev_id == one_dev->dev_info.device_id)
        {
            return one_dev;
        }
        list = nmp_list_next(list); 
    }

    return NULL;
}
static __inline__ config_device_info_t *
__config_find_device_by_puid(config_device_list_t *dev_list, 
        const char *pu_id)
{
    nmp_list_t *list;
    config_device_info_t *one_dev;

    list = nmp_list_first(dev_list->list);
    while (list)
    {
        one_dev = nmp_list_data(list);
        if (!strcmp(pu_id, one_dev->dev_info.pu_id))
        {
            return one_dev;
        }
        list = nmp_list_next(list); 
    }

    return NULL;
}
static __inline__ config_factory_info_t *
__config_find_factory_info(config_factory_list_t *fct_list, 
        const char *factory_name)
{
    nmp_list_t *list;
    config_factory_info_t *one_factory;

    list = nmp_list_first(fct_list->list);
    while (list)
    {
        one_factory = nmp_list_data(list);
        if (!strcmp((const char*)one_factory->fct_info.factory_name, factory_name))
        {
            return one_factory;
        }
        list = nmp_list_next(list); 
    }

    return NULL;
}

int config_check_user_is_login(struct service *srv, 
        config_user_info_t *user_info)
{
    int result;
    nmp_list_t *list;
    config_user_list_t *user_list;
    config_user_info_t *one_user;

    NMP_ASSERT(srv && user_info);

    result = LOGOUT;
    user_list = config_get_user_list(srv);

    nmp_mutex_lock(user_list->lock);

    list = nmp_list_first(user_list->list);
    while (list)
    {
        one_user = nmp_list_data(list);
        if (!strcmp(user_info->username, one_user->username) && 
            !strcmp(user_info->password, one_user->password))
        {
            switch (one_user->status)
            {
                case LOGOUT:
                    result = LOGIN_SUCCESS;
                    break;
                case LOGIN_SUCCESS:
                    result = LOGIN_FAILURE;
                    break;
                    
                default:
                    result = LOGOUT;
                    break;
            }
        }
        list = nmp_list_next(list); 
    }
    nmp_mutex_unlock(user_list->lock);

    return result;
}
int __config_check_user_is_exist(config_user_list_t *user_list, 
        const char *username)
{
    if (__config_find_user_info(user_list, username))
        return 1;
    else
        return 0;
}
int config_check_user_is_exist(config_user_list_t *user_list, 
        const char *username)
{
    int result;

    NMP_ASSERT(user_list && username);

    nmp_mutex_lock(user_list->lock);

    if (__config_find_user_info(user_list, username))
        result = 1;
    else
        result = 0;

    nmp_mutex_unlock(user_list->lock);

    return result;
}
int config_check_user_is_admin(struct service *srv)
{
    int result;
    client_t *client;
    config_service_t *cfg_srv;

    NMP_ASSERT(srv);

    cfg_srv = (config_service_t*)srv;
    client = &cfg_srv->client;

    nmp_mutex_lock(client->lock);
    if (!strcmp(DEF_ADMIN_USERNAME, client->user_info.username))
        result = 0;
    else
        result = -1;
    nmp_mutex_unlock(client->lock);

    return result;
}
int __config_check_device_is_exist(config_device_list_t *dev_list, 
        const char *pu_id)
{
    nmp_list_t *list;
    config_device_info_t *one_dev;

    list = nmp_list_first(dev_list->list);
    while (list)
    {
        one_dev = nmp_list_data(list);
        if (!strcmp(pu_id, one_dev->dev_info.pu_id))
        {
            return 1;
        }
        list = nmp_list_next(list); 
    }

    return 0;
}
int config_check_device_is_exist(config_device_list_t *dev_list, 
        const char *pu_id)
{
    int result;

    NMP_ASSERT(dev_list && pu_id);

    nmp_mutex_lock(dev_list->lock);

    if (__config_check_device_is_exist(dev_list, pu_id))
        result = 1;
    else
        result = 0;

    nmp_mutex_unlock(dev_list->lock);

    return result;
}
int config_check_device_info(struct service *srv, 
        config_device_info_t *dev_info)
{
    config_service_t *cfg_srv;
    NMP_ASSERT(srv && dev_info);

    cfg_srv = (config_service_t*)srv;
    return 0;
}

config_user_list_t *
config_create_user_list()
{
    config_user_list_t *user_list;
    char path[DEF_SYS_PATH_LENGHT] = {0};

    user_list = (config_user_list_t*)nmp_new0(config_user_list_t, 1);
    user_list->lock = nmp_mutex_new();

    snprintf(path, sizeof(path), "%s/%s", (char*)proxy_get_data_file_path(), 
        DEF_USER_INFO_FILE);
    config_read_user_from_file((const char*)path, user_list);

    return user_list;
}
config_device_list_t *
config_create_device_list()
{
    config_device_list_t *dev_list;
    char path[DEF_SYS_PATH_LENGHT] = {0};

    dev_list = (config_device_list_t*)nmp_new0(config_device_list_t, 1);
    dev_list->lock = nmp_mutex_new();

    snprintf(path, sizeof(path), "%s/%s", (char*)proxy_get_data_file_path(), 
        DEF_DEVICE_INFO_FILE);
    config_read_device_from_file((const char*)path, dev_list);

    return dev_list;
}
config_factory_list_t *
config_create_factory_list()
{
    config_factory_list_t *fct_list;
    char path[DEF_SYS_PATH_LENGHT] = {0};

    fct_list = (config_factory_list_t*)nmp_new0(config_factory_list_t, 1);
    fct_list->lock = nmp_mutex_new();

    snprintf(path, sizeof(path), "%s/%s", (char*)proxy_get_data_file_path(), 
        DEF_FACTORY_INFO_FILE);
    config_read_factory_from_file((const char*)path, fct_list);

    return fct_list;
}

config_user_list_t *
config_get_user_list(struct service *srv)
{
    cfg_service_basic_t *cfg_basic;

    NMP_ASSERT(srv);

    cfg_basic = config_get_service_basic((config_service_t*)srv);

    return cfg_basic->usr_list;
}
config_device_list_t *
config_get_device_list(struct service *srv)
{
    cfg_service_basic_t *cfg_basic;

    NMP_ASSERT(srv);

    cfg_basic = config_get_service_basic((config_service_t*)srv);

    return cfg_basic->dev_list;
}
config_factory_list_t *
config_get_factory_list(struct service *srv)
{
    cfg_service_basic_t *cfg_basic;

    NMP_ASSERT(srv);

    cfg_basic = config_get_service_basic((config_service_t*)srv);

    return cfg_basic->fct_list;
}

int config_save_user_list(config_user_list_t *user_list)
{
    int ret;
    char path0[DEF_SYS_PATH_LENGHT] = {0};
    char path1[DEF_SYS_PATH_LENGHT] = {0};
    char sys_cmd[DEF_SYS_COMMAND_SIZE] = {0};

    NMP_ASSERT(user_list);

    snprintf(path0, sizeof(path0), "%s/%s", (char*)proxy_get_data_file_path(), 
        TEMP_USER_INFO_FILE);
    ret = config_write_user_to_file((const char*)path0, user_list);
    if (!ret)
    {
        snprintf(path1, sizeof(path1), "%s/%s", (char*)proxy_get_data_file_path(), 
            DEF_USER_INFO_FILE);
        snprintf(sys_cmd, sizeof(sys_cmd), "mv %s %s", path0, path1);

        ret = system(sys_cmd);
    }

    return -ret;
}
int config_save_device_list(config_device_list_t *dev_list)
{
    int ret;
    char path0[DEF_SYS_PATH_LENGHT] = {0};
    char path1[DEF_SYS_PATH_LENGHT] = {0};
    char sys_cmd[DEF_SYS_COMMAND_SIZE] = {0};

    NMP_ASSERT(dev_list);

    snprintf(path0, sizeof(path0), "%s/%s", (char*)proxy_get_data_file_path(), 
        TEMP_DEVICE_INFO_FILE);
    ret = config_write_device_to_file((const char*)path0, dev_list);
    if (!ret)
    {
        snprintf(path1, sizeof(path1), "%s/%s", (char*)proxy_get_data_file_path(), 
            DEF_DEVICE_INFO_FILE);
        snprintf(sys_cmd, sizeof(sys_cmd), "mv %s %s", path0, path1);

        ret = system(sys_cmd);
    }

    return -ret;
}
int config_save_factory_list(config_factory_list_t *fct_list)
{
    int ret;
    char path0[DEF_SYS_PATH_LENGHT] = {0};
    char path1[DEF_SYS_PATH_LENGHT] = {0};
    char sys_cmd[DEF_SYS_COMMAND_SIZE] = {0};

    NMP_ASSERT(fct_list);

    snprintf(path0, sizeof(path0), "%s/%s", (char*)proxy_get_data_file_path(), 
        TEMP_FACTORY_INFO_FILE);
    ret = config_write_factory_to_file((const char*)path0, fct_list);
    if (!ret)
    {
        snprintf(path1, sizeof(path1), "%s/%s", (char*)proxy_get_data_file_path(), 
            DEF_FACTORY_INFO_FILE);
        snprintf(sys_cmd, sizeof(sys_cmd), "mv %s %s", path0, path1);

        ret = system(sys_cmd);
    }

    return -ret;
}
static int 
config_save_server_config(prx_server_config *srv_cfg)
{
    int ret;
    char path0[DEF_SYS_PATH_LENGHT] = {0};
    char path1[DEF_SYS_PATH_LENGHT] = {0};
    char sys_cmd[DEF_SYS_COMMAND_SIZE] = {0};

    NMP_ASSERT(srv_cfg);

    snprintf(path0, sizeof(path0), "%s/%s", (char*)proxy_get_data_file_path(), 
        TEMP_SERVER_CONFIG_FILE);

    ret = config_write_server_config_to_file((const char*)path0, srv_cfg);
    if (!ret)
    {
        snprintf(path1, sizeof(path1), "%s/%s", (char*)proxy_get_data_file_path(), 
            DEF_SERVER_CONFIG_FILE);
        snprintf(sys_cmd, sizeof(sys_cmd), "mv %s %s", path0, path1);
        ret = system(sys_cmd);
    }

    return -ret;
}

int config_add_new_user_info(config_user_list_t *user_list, 
        config_user_info_t *user_info)
{
    int ret = -1;
    config_user_info_t *new_user;

    NMP_ASSERT(user_list && user_info);

    nmp_mutex_lock(user_list->lock);
    if (!__config_check_user_is_exist(user_list, user_info->username))
    {
        ret = 0;
        new_user = (config_user_info_t*)nmp_new0(config_user_info_t, 1);
        memcpy(new_user, user_info, sizeof(config_user_info_t));

        user_list->list = nmp_list_add_tail(user_list->list, new_user);
    }
    nmp_mutex_unlock(user_list->lock);

    return ret;
}
int config_add_new_device_info(config_device_list_t *dev_list, 
        config_device_info_t *dev_info)
{
    int i, dev_id = -1;
    static int id_generator = 1;

    proxy_plt_t plt_info;
    proxy_sdk_t sdk_info;

    proxy_device_t *new_dev;
    config_device_info_t *one_dev;

    NMP_ASSERT(dev_list && dev_info);

    if (config_check_device_is_exist(dev_list, dev_info->dev_info.pu_id))
        return -1;

    memset(&plt_info, 0, sizeof(proxy_plt_t));
    memset(&sdk_info, 0, sizeof(proxy_sdk_t));

    plt_info.pu_type  = dev_info->dev_info.pu_type;
    plt_info.cms_port = dev_info->dev_info.platform_port;
    plt_info.protocol = dev_info->dev_info.protocol;
    plt_info.keep_alive_freq = DEFAULT_HEARTBEAT_SECS;

    strncpy(plt_info.pu_id, dev_info->dev_info.pu_id, 
        sizeof(plt_info.pu_id)-1);
    strncpy(plt_info.cms_host, dev_info->dev_info.platform_ip, 
        sizeof(plt_info.cms_host)-1);

    sdk_info.dev_port = dev_info->dev_info.device_port;
    strncpy(sdk_info.dev_host, dev_info->dev_info.device_ip, 
        sizeof(sdk_info.dev_host)-1);
    strncpy(sdk_info.username, dev_info->dev_info.username, 
        sizeof(sdk_info.username)-1);
    strncpy(sdk_info.password, dev_info->dev_info.password, 
        sizeof(sdk_info.password)-1);

    for (i=0; i<MAX_SDK; i++)
    {
        if (!strcmp(dev_info->dev_info.factory, g_sdk_items[i].fct_name))
        {
            strncpy(sdk_info.sdk_name, g_sdk_items[i].sdk_name, 
                sizeof(sdk_info.sdk_name)-1);
        }
    }

    new_dev = proxy_create_new_device(id_generator, &plt_info, &sdk_info);
    if (new_dev)
    {
        dev_id = id_generator++;

        one_dev = (config_device_info_t*)nmp_new0(config_device_info_t, 1);
        memcpy(one_dev, dev_info, sizeof(config_device_info_t));

        one_dev->dev_info.device_id = dev_id;
        one_dev->pxy_dev = new_dev;

        nmp_mutex_lock(dev_list->lock);
        dev_list->list = nmp_list_add_tail(dev_list->list, one_dev);
        nmp_mutex_unlock(dev_list->lock);
    }

    return dev_id;
}
int config_add_new_factory_info(config_factory_list_t *fct_list, 
        config_factory_info_t *fct_info)
{
    int ret = -1;
    config_factory_info_t *new_factory;
    
    NMP_ASSERT(fct_list && fct_info);
    
    nmp_mutex_lock(fct_list->lock);
    
    if (!__config_find_factory_info(fct_list, fct_info->fct_info.factory_name))
    {
        ret = 0;
        new_factory = (config_factory_info_t*)nmp_new0(config_factory_info_t, 1);
        memcpy(&new_factory->fct_info, &fct_info->fct_info, sizeof(prx_factory_info));

        fct_list->list = nmp_list_add_tail(fct_list->list, new_factory);
    }
    
    nmp_mutex_unlock(fct_list->lock);
    
    return ret;
}

int config_remove_one_user_info(config_user_list_t *user_list, 
        const char *username)
{
    int ret = -1;
    config_user_info_t *one_user;

    NMP_ASSERT(user_list && username);

    nmp_mutex_lock(user_list->lock);
    one_user = __config_find_user_info(user_list, username);
    if (one_user)
    {
        ret = 0;
        user_list->list = nmp_list_remove(user_list->list, one_user);     
        nmp_del(one_user, config_user_info_t, 1);
    }
    nmp_mutex_unlock(user_list->lock);
    show_info("Remove user [username:%s], err: %d\n", username, ret);
    return ret;
}
int config_remove_one_device_info(config_device_list_t *dev_list, 
        int dev_id)
{
    int flag = -1;
    proxy_device_t *prx_dev;
    config_device_info_t *dev_info;

    NMP_ASSERT(dev_list);

    prx_dev = proxy_find_device_by_dev_id(dev_id);//It would ref that if found!
    if (prx_dev)
    {
        flag = 1;
        nmp_mutex_lock(prx_dev->lock);
        if (prx_dev->killed != STATE_KILLED)
            prx_dev->killed = STATE_KILLED;
        nmp_mutex_unlock(prx_dev->lock);
        proxy_device_unref(prx_dev);
        proxy_device_unref(prx_dev);//destroy!!!
    }

    nmp_mutex_lock(dev_list->lock);
    if (flag)
    {
        dev_info = __config_find_device_by_id(dev_list, dev_id);
        if (dev_info)
        {
            flag = 0;
            dev_list->list = nmp_list_remove(dev_list->list, dev_info);
            nmp_del(dev_info, config_device_info_t, 1);
        }
        else
            flag = -2;
    }
    nmp_mutex_unlock(dev_list->lock);
    show_info("Remove device [dev_id:%d], err: %d\n", dev_id, flag);
    return flag;
}
int config_remove_one_factory_info(config_factory_list_t *fct_list, 
        const char *factory_name)
{
    return 0;
}
config_user_info_t *
config_find_user_by_name(config_user_list_t *user_list, 
        config_user_info_t *user_info, const char *username)
{
    int flag = -1;
    config_user_info_t *one_user;
    
    NMP_ASSERT(user_list && user_info);

    nmp_mutex_lock(user_list->lock);
    one_user = __config_find_user_info(user_list, username);
    if (one_user)
    {
        flag = 0;
        memcpy(user_info, one_user, sizeof(config_user_info_t));
    }
    nmp_mutex_unlock(user_list->lock);

    if (!flag)
        return user_info;
    else
        return NULL;
}
config_device_info_t *
config_find_device_by_id(config_device_list_t *dev_list, 
        config_device_info_t *dev_info, int dev_id)
{
    int flag = -1;
    config_device_info_t *one_dev;
    
    NMP_ASSERT(dev_list && dev_info);

    nmp_mutex_lock(dev_list->lock);
    one_dev = __config_find_device_by_id(dev_list, dev_id);
    if (one_dev)
    {
        flag = 0;
        memcpy(dev_info, one_dev, sizeof(config_device_info_t));
    }
    nmp_mutex_unlock(dev_list->lock);

    if (!flag)
        return dev_info;
    else
        return NULL;
}

int config_modify_user_password(config_user_list_t *user_list, 
        config_user_info_t *user_info)
{
    int flag = -1;
    config_user_info_t *one_user;
    
    nmp_mutex_lock(user_list->lock);
    one_user = __config_find_user_info(user_list, user_info->username);
    if (one_user)
    {
        flag = 0;
        memcpy(one_user->password, user_info->password, sizeof(one_user->password));
    }
    nmp_mutex_unlock(user_list->lock);
    show_info("Modify password [username:%s], err: %d\n", user_info->username, flag);
    return flag;
}
int config_modify_user_status(config_user_list_t *user_list, 
        config_user_info_t *user_info)
{
    int flag = -1;
    config_user_info_t *one_user;
    
    nmp_mutex_lock(user_list->lock);
    one_user = __config_find_user_info(user_list, user_info->username);
    if (one_user)
    {
        flag = 0;
        one_user->status = user_info->status;
    }
    nmp_mutex_unlock(user_list->lock);

    return flag;
}

int config_modify_user_info(config_user_list_t *user_list, 
        config_user_info_t *old_user, config_user_info_t *new_user)
{
    int flag = -1;
    config_user_info_t *one_user;
    
    NMP_ASSERT(user_list && new_user);
    
show_debug("new status :%d\n", new_user->status);
show_debug("username   :%s\n", new_user->username);
show_debug("password   :%s\n", new_user->password);

    nmp_mutex_lock(user_list->lock);
    one_user = __config_find_user_info(user_list, new_user->username);
    if (one_user)
    {
        if (old_user && strlen(old_user->username))
        {
            if (!strcmp(one_user->password, old_user->password))
            {
                memcpy(one_user, new_user, sizeof(config_user_info_t));
                flag = 0;
            }
        }
        else
        {
            memcpy(one_user, new_user, sizeof(config_user_info_t));
            flag = 0;
        }
    }
    nmp_mutex_unlock(user_list->lock);

    return flag;
}
int config_modify_device_info(config_device_list_t *dev_list, 
        config_device_info_t *dev_info)
{
    int i, flag = -1;
    proxy_device_t *dev;
    config_device_info_t *one_dev;
    
    proxy_plt_t plt_info;
    proxy_sdk_t sdk_info;
    
    NMP_ASSERT(dev_list && dev_info);

    memset(&plt_info, 0, sizeof(proxy_plt_t));
    memset(&sdk_info, 0, sizeof(proxy_sdk_t));
    
    plt_info.cms_port = dev_info->dev_info.platform_port;
    plt_info.pu_type  = dev_info->dev_info.pu_type;
    plt_info.protocol = dev_info->dev_info.protocol;
    plt_info.keep_alive_freq = DEFAULT_HEARTBEAT_SECS;
    
    strncpy(plt_info.pu_id, dev_info->dev_info.pu_id, 
        sizeof(plt_info.pu_id)-1);
    strncpy(plt_info.cms_host, dev_info->dev_info.platform_ip, 
        sizeof(plt_info.cms_host)-1);
    
    sdk_info.dev_port = dev_info->dev_info.device_port;
    strncpy(sdk_info.dev_host, dev_info->dev_info.device_ip, 
        sizeof(sdk_info.dev_host)-1);
    strncpy(sdk_info.username, dev_info->dev_info.username, 
        sizeof(sdk_info.username)-1);
    strncpy(sdk_info.password, dev_info->dev_info.password, 
        sizeof(sdk_info.password)-1);

    for (i=0; i<MAX_SDK; i++)
    {
        if (!strcmp(dev_info->dev_info.factory, g_sdk_items[i].fct_name))
        {
            strncpy(sdk_info.sdk_name, g_sdk_items[i].sdk_name, 
                sizeof(sdk_info.sdk_name)-1);
        }
    }

    nmp_mutex_lock(dev_list->lock);
    one_dev = __config_find_device_by_puid(dev_list, dev_info->dev_info.pu_id);
    if (one_dev && 
        dev_info->dev_info.device_id != one_dev->dev_info.device_id)
    {
        nmp_mutex_unlock(dev_list->lock);
        return flag;
    }

    one_dev = __config_find_device_by_id(dev_list, dev_info->dev_info.device_id);
    if (one_dev)
    {
        dev = proxy_find_device_by_dev_id(one_dev->dev_info.device_id);
        if (dev)
        {
            flag = 0;
            control_service(dev->sdk_srv, CTRL_CMD_LOGOUT, NULL);

            memcpy(&one_dev->dev_info, &dev_info->dev_info, sizeof(prx_device_info));
            proxy_set_device_private(dev, DEV_PRI_PLT_SRV, 
                    sizeof(proxy_plt_t), &plt_info);
            proxy_set_device_private(dev, DEV_PRI_PRX_SDK, 
                    sizeof(proxy_sdk_t), &sdk_info);
            
            proxy_device_unref(dev);
        }
    }
    nmp_mutex_unlock(dev_list->lock);
    show_info("Modify device [dev_id:%d], err: %d\n", dev_info->dev_info.device_id, flag);
    return flag;
}

int config_user_login_request(struct service *srv, 
        config_user_info_t *user_info)
{
    nmp_list_t *list;
    int result = LOGOUT;
    config_user_list_t *user_list;
    config_user_info_t *one_user;

    NMP_ASSERT(srv && user_info);

    user_list = config_get_user_list(srv);

    nmp_mutex_lock(user_list->lock);

    list = nmp_list_first(user_list->list);
    while (list)
    {
        one_user = nmp_list_data(list);
        if (!strcmp(user_info->username, one_user->username) && 
            !strcmp(user_info->password, one_user->password))
        {
            switch (one_user->status)
            {
                case LOGOUT:
                    result = LOGIN_SUCCESS;
                    one_user->status = result;
                    break;
                case LOGIN_SUCCESS:
                    result = LOGIN_FAILURE;
                    break;
                default:
                    result = LOGOUT;
                    break;
            }
        }

        list = nmp_list_next(list); 
    }
    
    nmp_mutex_unlock(user_list->lock);

    return result;
}

int config_get_user_total_count(config_user_list_t *user_list)
{
    int count;
    nmp_list_t *list;
    
    NMP_ASSERT(user_list);

    count = 0;
    nmp_mutex_lock(user_list->lock);
    
    list = nmp_list_first(user_list->list);

    while (list)
    {
        count++;
        list = nmp_list_next(list);
    }
    
    nmp_mutex_unlock(user_list->lock);
    
    return count;
}
int config_get_user_by_fuzzy_name(config_user_list_t *user_list, 
        const char *username)
{
    int count;
    nmp_list_t *list;
    
    config_user_info_t *one_user;
    
    NMP_ASSERT(user_list && username);

    count = 0;
    nmp_mutex_lock(user_list->lock);
    
    list = nmp_list_first(user_list->list);

    while (list)
    {
        one_user = nmp_list_data(list);
        if (strstr((const char*)one_user->username, username))
        {
            count++;
        }
        list = nmp_list_next(list);
    }
    
    nmp_mutex_unlock(user_list->lock);
    
    return count;
}

int config_get_one_page_user_info(config_user_list_t *user_list, 
        config_user_info_t *user_info, config_user_factor_t *user_factor)
{
    nmp_list_t *list;
    int count, offset;
    
    config_user_info_t *one_user;
    
    NMP_ASSERT(user_list && user_info && user_factor);
    
    nmp_mutex_lock(user_list->lock);
    
    count = 0;
    offset = user_factor->offset;
    
    list = nmp_list_first(user_list->list);
    while (list)
    {
        one_user = nmp_list_data(list);

        if (strlen(user_factor->username))
        {
            if (!strstr(one_user->username, user_factor->username))
            {
                list = nmp_list_next(list);
                continue;
            }
        }
        
        user_factor->total++;
        if (offset-- > 0)
        {
            list = nmp_list_next(list);
            continue;
        }
        
        if (user_factor->count > count)
        {
            strncpy(user_info[count].username, 
                one_user->username, 
                sizeof(user_info[count].username)-1);
            user_info[count].username[sizeof(user_info[count].username)-1] = '\0';

            count++;
        }
        
        list = nmp_list_next(list);
    }
    
    nmp_mutex_unlock(user_list->lock);
    
    return count;
}

static __inline__ int 
process_device_factor(config_device_factor_t *dev_factor, 
    config_device_info_t *one_dev)
{
    int flag = -1;
    int power = 0;

    if (strlen(dev_factor->factory_name))
        power += FACTOR_FACTORY;

    if (-1 < dev_factor->machine_type)
        power += FACTOR_MACHINE;

    if (strlen(dev_factor->sdk_version))
        power += FACTOR_VERSION;
    
    switch (power)
    {
        case 0:
            flag = 0;
            break;
        case 1:
            if (CMP_FACTORY(dev_factor, one_dev))
                flag = 0;
            break;
        case 2:
            if (CMP_MACHINE(dev_factor, one_dev))
                flag = 0;
            break;
        case 3:
            if (CMP_FACTORY(dev_factor, one_dev) && 
                CMP_MACHINE(dev_factor, one_dev))
                flag = 0;
            break;
        case 4:
            if (CMP_VERSION(dev_factor, one_dev))
                flag = 0;
            break;
        case 5:
            if (CMP_FACTORY(dev_factor, one_dev) && 
                CMP_VERSION(dev_factor, one_dev))
                flag = 0;
            break;
        case 6:
            if (CMP_MACHINE(dev_factor, one_dev) && 
                CMP_VERSION(dev_factor, one_dev))
                flag = 0;
            break;
        case 7:
            if (CMP_FACTORY(dev_factor, one_dev) && 
                CMP_MACHINE(dev_factor, one_dev) && 
                CMP_VERSION(dev_factor, one_dev))
                flag = 0;
            break;

        default:
            break;
    }

    return flag;
}

int config_get_one_page_device_info(config_device_list_t *dev_list, 
        config_device_info_t *dev_info, config_device_factor_t *dev_factor)
{
    nmp_list_t *list;
    int flag, offset, count = 0;
    
    config_device_info_t *one_dev;
    
    NMP_ASSERT(dev_list && dev_info && dev_factor);
    
    nmp_mutex_lock(dev_list->lock);
    
    offset = dev_factor->offset;
    list = nmp_list_first(dev_list->list);
    while (list)
    {
        one_dev = nmp_list_data(list);
        
        if (-1 == dev_factor->dev_id)
            flag = process_device_factor(dev_factor, one_dev);
        else
        {
            if (CMP_DEVICE_ID(dev_factor, one_dev))
                flag = 0;
            else
                flag = -1;
        }
        
        if (!flag)
        {
            dev_factor->total++;
            if (offset-- > 0)
            {
                list = nmp_list_next(list);
                continue;
            }
            
            if (dev_factor->count > count)
            {
                memcpy(&dev_info[count], one_dev, sizeof(dev_info[count]));
                dev_info[count].dev_info.device_st = one_dev->pxy_dev->state.state;
                dev_info[count].dev_info.device_err = one_dev->pxy_dev->state.error;
                
                count++;
            }
        }
        
        list = nmp_list_next(list);
    }
    
    nmp_mutex_unlock(dev_list->lock);
    
    return count;
}

int config_get_server_config(proxy_config_t *cfg)
{
    prx_server_config srv_cfg;
    char path[DEF_SYS_PATH_LENGHT];

    memset(&srv_cfg, 0, sizeof(srv_cfg));
    memset(path, 0, DEF_SYS_PATH_LENGHT);

    snprintf(path, sizeof(path), "%s/%s", (char*)proxy_get_data_file_path(), 
        DEF_SERVER_CONFIG_FILE);
    if (!config_read_server_config_from_file((const char*)path, &srv_cfg))
    {
        if (MIN_DEVICE_USER_PORT >= srv_cfg.listen_port)
            srv_cfg.listen_port = DEFAULT_LISTEN_PORT;
        if (MIN_DEVICE_USER_PORT >= srv_cfg.rtsp_port)
            srv_cfg.rtsp_port = DEFAULT_DATA_PORT;
    }
    else
    {
        strcpy(srv_cfg.server_ip, "127.0.0.1");
        srv_cfg.listen_port = DEFAULT_LISTEN_PORT;
        srv_cfg.rtsp_port = DEFAULT_DATA_PORT;
        config_save_server_config(&srv_cfg);
    }

    strncpy(cfg->host, srv_cfg.server_ip, sizeof(cfg->host)-1);
    cfg->cmd_port = srv_cfg.listen_port;
    cfg->rtsp_port = srv_cfg.rtsp_port;

    return 0;
}
int config_set_server_config(proxy_config_t *cfg)
{
    int flag;
    proxy_config_t local_cfg;
    prx_server_config srv_cfg;
    NMP_ASSERT(cfg);

    if (MIN_DEVICE_USER_PORT >= cfg->rtsp_port)
        return -1;
    if (MIN_DEVICE_USER_PORT >= cfg->rtsp_port)
        return -1;

    config_get_server_config(&local_cfg);
    if (strcmp(cfg->host, local_cfg.host) || 
        cfg->cmd_port  != local_cfg.cmd_port ||
        cfg->rtsp_port != local_cfg.rtsp_port)
    {
        strncpy(srv_cfg.server_ip, cfg->host, sizeof(srv_cfg.server_ip)-1);
        srv_cfg.server_ip[sizeof(srv_cfg.server_ip)-1] = '\0';
        srv_cfg.listen_port = cfg->cmd_port;
        srv_cfg.rtsp_port   = cfg->rtsp_port;
        config_save_server_config(&srv_cfg);

        flag = 1;
    }
    else
        flag = 0;

    return flag;
}


