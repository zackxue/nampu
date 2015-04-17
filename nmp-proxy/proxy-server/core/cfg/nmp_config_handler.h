
#ifndef __CONFIG_HANDLER_H__
#define __CONFIG_HANDLER_H__

#include "nmp_msg_impl.h"
#include "nmp_proxy_server.h" 
#include "nmp_config_srv_impl.h"

#define MAX_USER_PAGE_SIZE          25
#define MAX_DEVICE_PAGE_SIZE        25


enum
{
    HANDLE_ADD_USER = 0,
    HANDLE_DEL_USER,
    HANDLE_GET_USER,
    HANDLE_SET_USER,

    HANDLE_ADD_DEVICE,
    HANDLE_DEL_DEVICE,
    HANDLE_GET_DEVICE,
    HANDLE_SET_DEVICE,
    HANDLE_GET_FACTORY,

    HANDLE_GET_CONFIG,
    HANDLE_SET_CONFIG,

    HANDLE_BACKUP,
    HANDLE_REVERT,

    HANDLE_LIMIT,

    MAX_HANDLE_SIZE
};

enum
{
    BROADCAST_ADD_USER,
    BROADCAST_DEL_USER,
    BROADCAST_ADD_DEVICE,
    BROADCAST_DEL_DEVICE,
    BROADCAST_MODIFY_DEVICE,
    BROADCAST_DEVICE_STATUS,
};

#ifdef __cplusplus
extern "C" {
#endif

void config_register_all_msg_handlers(msg_engine_t *me);



#ifdef __cplusplus
    }
#endif


#endif  //__CONFIG_HANDLER_H__

