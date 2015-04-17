/*
 *          file: nmp_proxy_sdk.h
 *          description:
 *
 *          hegui,heguijiss@gmail.com
 *          June 06th, 2013
 */

#ifndef __PROXY_SDK_H__
#define __PROXY_SDK_H__


#include "nmp_proxy_server.h"

#define DEF_GET_SUCCESS    0
#define DEF_SET_FAILURE    -1


enum
{
    GET_DEVICE_CONFIG,
    GET_SERIAL_CONFIG,
    SET_SERIAL_CONFIG,
    GET_DEVICE_TIME,
    SET_DEVICE_TIME,
    GET_NTP_CONFIG,
    SET_NTP_CONFIG,
    GET_NETWORK_CONFIG,
    SET_NETWORK_CONFIG,
    GET_PPPOE_CONFIG,
    SET_PPPOE_CONFIG,
    GET_FTP_CONFIG,
    SET_FTP_CONFIG,
    GET_SMTP_CONFIG,
    SET_SMTP_CONFIG,
    GET_DDNS_CONFIG,
    SET_DDNS_CONFIG,
    GET_UPNP_CONFIG,
    SET_UPNP_CONFIG,
    GET_DISK_LIST,
    SET_DISK_FORMAT,
    CONTROL_DEVICE_CMD,

    GET_ENCODE_CONFIG,
    SET_ENCODE_CONFIG,
    GET_DISPLAY_CONFIG,
    SET_DISPLAY_CONFIG,
    GET_OSD_CONFIG,
    SET_OSD_CONFIG,
    GET_PTZ_CONFIG,
    SET_PTZ_CONFIG,
    GET_RECORD_CONFIG,
    SET_RECORD_CONFIG,
    GET_HIDE_CONFIG,
    SET_HIDE_CONFIG,

    GET_MOTION_CONFIG,
    SET_MOTION_CONFIG,
    GET_VIDEO_LOST_CONFIG,
    SET_VIDEO_LOST_CONFIG,
    GET_HIDE_ALARM_CONFIG,
    SET_HIDE_ALARM_CONFIG,
    GET_IO_ALARM_CONFIG,
    SET_IO_ALARM_CONFIG,

    GET_STORE_LOG,

    CONTROL_PTZ_CMD,
    SET_PRESET_CONFIG,
    GET_CRUISE_CONFIG,
    SET_CRUISE_CONFIG,
    ADD_CRUISE_CONFIG,
    MDF_CRUISE_CONFIG,

    GET_CAPABILITY_SET,

    MAX_SDK_CONFIG,
};

typedef int (*sdk_handler_t)(struct service*, int, int, void*);

typedef struct sdk_item sdk_item_t;

struct sdk_item
{
    SDK_TYPE_E type;
    char sdk_name[MAX_USR_LEN];
    char fct_name[MAX_USR_LEN];
};

typedef struct handler_table handler_table_t;

struct handler_table
{
    int id;
    sdk_handler_t handler;
};


#ifdef __cplusplus
extern "C" {
#endif

extern sdk_item_t g_sdk_items[];


handler_table_t *
    get_sdk_msg_handler(struct service *srv);

stream_operation_t *
    get_sdk_stream_opt(struct service *srv);

talk_opt_t *
    get_sdk_talk_opt(struct service *srv);



#ifdef __cplusplus
    }
#endif


#endif	//__PROXY_SDK_H__


