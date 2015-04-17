
#ifndef __CONFIG_SERVICE_H__
#define __CONFIG_SERVICE_H__

#include "string.h"

#include "nmp_net.h"
#include "nmp_proxy_sdk.h"
#include "nmp_proto_impl.h"
#include "nmp_config_info.h"




typedef struct config_service_list config_service_list_t;

struct config_service_list
{
    nmp_mutex_t *lock;
    nmp_list_t  *list;
};


struct config_service_basic
{
    service_template_t tm;

    nmp_net_t                  *_2lp_net;        /* 2 levels protocol */
    nmpio_t                *listener;
    packet_opt_t          *pack_opt;
    msg_engine_t          *clt_me;

    config_user_list_t    *usr_list;
    config_device_list_t  *dev_list;
    config_factory_list_t *fct_list;

    config_service_list_t *srv_list;

    void *timer;
};

typedef struct range range_t;

struct range
{
    int start;
    int end;
    int total;
};

typedef struct client client_t;

struct client
{
    nmp_mutex_t *lock;
    nmpio_t *io;

    unsigned int ttl;
    unsigned int ttd;
    unsigned int seq;

    int keep_alive_freq;

    range_t range;
    config_user_info_t user_info;
};

typedef struct config_service config_service_t;

struct config_service
{
    struct service base;

    client_t client;
};


#ifdef __cplusplus
extern "C" {
#endif


void config_broadcast_event(cfg_service_basic_t *cfg_basic, msg_t *bc_msg);



#ifdef __cplusplus
    }
#endif

#endif  //__CONFIG_SERVICE_H__

