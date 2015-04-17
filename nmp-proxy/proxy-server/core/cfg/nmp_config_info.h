
#ifndef __CONFIG_INFO_H__
#define __CONFIG_INFO_H__

#include "proxy_info.h"
#include "nmp_proxy_server.h"


#define DEF_SYS_PATH_LENGHT        128
#define DEF_SYS_COMMAND_SIZE       512

#define DEF_USER_INFO_FILE          "user_info.xml"     //用户配置文件
#define DEF_DEVICE_INFO_FILE        "device_info.xml"   //设备配置文件
#define DEF_FACTORY_INFO_FILE       "factory_info.xml"  //设备类型文件
#define DEF_SERVER_CONFIG_FILE      "server_config.xml" //设备类型文件

#define DEF_ADMIN_USERNAME          "admin"


typedef enum
{
    LOGOUT=-1,
    LOGIN_SUCCESS,
    LOGIN_FAILURE,
}user_status;


typedef struct config_user_info config_user_info_t;

struct config_user_info
{
    user_status status;
    char username[MAX_USR_LEN];         //用户名
    char password[MAX_PSW_LEN];         //密码
};

typedef struct config_user_list config_user_list_t;

struct config_user_list
{
    nmp_mutex_t *lock;
    nmp_list_t  *list;
};

typedef struct config_device_info config_device_info_t;

struct config_device_info
{
    prx_device_info dev_info;
    struct proxy_device *pxy_dev;   //pointor back to device
};

typedef struct config_device_list config_device_list_t;

struct config_device_list
{
    nmp_mutex_t *lock;
    nmp_list_t  *list;
};

typedef struct config_factory_info config_factory_info_t;

struct config_factory_info
{
    prx_factory_info fct_info;
};

typedef struct config_factory_list config_factory_list_t;

struct config_factory_list
{
    nmp_mutex_t *lock;
    nmp_list_t  *list;
};

typedef struct config_user_factor config_user_factor_t;

struct config_user_factor
{
    int total;
    int offset;
    int count;
    char username[MAX_USR_LEN];
};




enum
{
    FACTOR_FACTORY = 0x1,
    FACTOR_MACHINE = 0x2,
    FACTOR_VERSION = 0x4,
};

typedef struct config_device_factor config_device_factor_t;

struct config_device_factor
{
    int total;
    int offset;
    int count;
    int dev_id;
    int machine_type;
    char sdk_version[J_PRX_MAX_VERSION_LEN];
    char factory_name[J_PRX_MAX_NAME_LEN];
};

#define CMP_DEVICE_ID(factor, device) \
        (factor->dev_id == device->dev_info.device_id)
#define CMP_MACHINE(factor, device) \
        (factor->machine_type == device->dev_info.pu_type)
#define CMP_FACTORY(factor, device) \
        (!strcmp(factor->factory_name, device->dev_info.factory))
#define CMP_VERSION(factor, device) \
        (!strcmp(factor->sdk_version, device->dev_info.sdk_version))


#ifdef __cplusplus
extern "C" {
#endif


config_user_list_t *
    config_create_user_list();
config_device_list_t *
    config_create_device_list();
config_factory_list_t *
    config_create_factory_list();

config_user_list_t *
    config_get_user_list(struct service *srv);
config_device_list_t *
    config_get_device_list(struct service *srv);
config_factory_list_t *
    config_get_factory_list(struct service *srv);

int config_save_user_list(config_user_list_t *user_list);
int config_save_device_list(config_device_list_t *dev_list);
int config_save_factory_list(config_factory_list_t *fct_list);

int config_add_new_user_info(config_user_list_t *user_list, 
        config_user_info_t *user_info);
int config_add_new_device_info(config_device_list_t *dev_list, 
        config_device_info_t *dev_info);
int config_add_new_factory_info(config_factory_list_t *fct_list, 
        config_factory_info_t *fct_info);

int config_remove_one_user_info(config_user_list_t *user_list, 
        const char *username);
int config_remove_one_device_info(config_device_list_t *dev_list, 
        int dev_id);
int config_remove_one_factory_info(config_factory_list_t *fct_list, 
        const char *factory_name);

int config_user_login_request(struct service *srv, config_user_info_t *user_info);
int config_check_user_is_login(struct service *srv, config_user_info_t *user_info);
int config_check_user_is_exist(config_user_list_t *user_list, const char *username);
int config_check_user_is_admin(struct service *srv);
int config_check_device_info(struct service *srv, config_device_info_t *dev_info);

int config_modify_user_password(config_user_list_t *user_list, 
        config_user_info_t *user_info);
int config_modify_user_status(config_user_list_t *user_list, 
        config_user_info_t *user_info);
int config_modify_user_info(config_user_list_t *user_list, 
        config_user_info_t *old_user, config_user_info_t *new_user);
int config_modify_device_info(config_device_list_t *dev_list, 
        config_device_info_t *dev_info);

config_user_info_t *
    config_find_user_by_name(config_user_list_t *user_list, 
        config_user_info_t *user_info, const char *username);

int config_get_user_total_count(config_user_list_t *user_list);
int config_get_user_by_fuzzy_name(config_user_list_t *user_list, 
        const char *username);
int config_get_one_page_user_info(config_user_list_t *user_list, 
        config_user_info_t *user_info, config_user_factor_t *user_factor);
int config_get_one_page_device_info(config_device_list_t *dev_list, 
        config_device_info_t *dev_info, config_device_factor_t *dev_factor);

int config_get_server_config(proxy_config_t *cfg);
int config_set_server_config(proxy_config_t *cfg);


#ifdef __cplusplus
        }
#endif



#endif  //__CONFIG_INFO_H__

