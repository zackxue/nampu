/*
 *          file: nmp_proxy_device.h
 *          description:抽象代理服务器设备对象
 *
 *          hegui,heguijiss@gmail.com
 *          May 16th, 2013
 */

#ifndef __PROXY_DEVICE_H__
#define __PROXY_DEVICE_H__

#include "nmplib.h"

#include "nmp_msg_impl.h"
#include "nmp_proxy_server.h"

#define STATE_OK            0
#define STATE_KILLED        1


typedef int (*get_device_config)(struct service *srv, int cmd, void *config);
typedef int (*set_device_config)(struct service *srv, int cmd, void *config);

typedef enum
{
    STATE_UNKNOWN = 0xffffffff,
    STATE_LOGOUT  = 0x0,                 //未连接设备, 错误码: 0 (连接设备失败, 错误码: xxx)
    STATE_LOGING  = 0x1,                 //正在连接设备, 错误码: 0 (正在登出设备，错误码: xxx)
    STATE_LOGIN   = 0x2,                 //已连接设备, 错误码: 0 
    STATE_DISCONNECT = STATE_LOGIN,      //连接CMS 失败, 错误码: xxx
    STATE_CONNECTING = 0x3,              //正在注册cms , 错误码: 0 
    STATE_UNREGISTER = STATE_DISCONNECT, //注册cms 失败, 错误码: xxx
    STATE_REGISTERED = 0x4,              //注册CMS 成功, 错误码: 0 
}PROXY_STATE_E;

typedef struct proxy_state proxy_state_t;

struct proxy_state
{
    int state;
    int error;
};


typedef struct proxy_private proxy_private_t;

struct proxy_private
{
    nmp_mutex_t *lock;
    int total_size;
    struct {
        int offset;
        int size;
    }data[DEV_PRI_MAX];
};

typedef struct fasten fasten_t;

struct fasten
{
    int dev_id;                 /* 外键: 主键在config_device_list_t. */
    int dev_type;
};

typedef struct proxy_device proxy_device_t;

struct proxy_device
{
    atomic_t ref_count;
    int      killed;

    fasten_t fastenings;        /* fastenings. */
    proxy_state_t state;

    struct service *plt_srv;    /* platform service. */
    struct service *sdk_srv;    /* sdk service. */

    void *pri_data;             /* device private data. */

    nmp_mutex_t *lock;
};


#ifdef __cplusplus
extern "C" {
#endif

int proxy_manage_device();
void proxy_destroy_all_device();

proxy_device_t *
    proxy_create_new_device(int index, proxy_plt_t *plt_info, proxy_sdk_t *sdk_info);

proxy_device_t *
    proxy_device_ref(proxy_device_t *dev);
void 
    proxy_device_unref(proxy_device_t *dev);

proxy_device_t *
    proxy_find_device_by_dev_id(int dev_id);
proxy_device_t *
    proxy_find_device_by_user_id(int user_id, const char *sdk_name);
proxy_device_t *
    proxy_find_device_by_dev_ip(char *dev_ip);
proxy_device_t *
    proxy_find_device_by_pu_id(char *pu_id);

int proxy_get_device_private(proxy_device_t *dev, int type, void *pvalue);
int proxy_set_device_private(proxy_device_t *dev, int type, int size, void *pvalue);

int proxy_get_device_state(proxy_device_t *dev);
int proxy_set_device_state(proxy_device_t *dev, proxy_state_t *dev_state);

void proxy_deliver_device_msg(proxy_device_t *dev, msg_engine_t *me, msg_t *msg);
void proxy_send_device_msg(proxy_device_t *dev, msg_t *msg);


#ifdef __cplusplus
    }
#endif


#endif  //__PROXY_DEVICE_H__

