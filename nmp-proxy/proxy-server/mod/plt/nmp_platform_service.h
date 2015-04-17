
#ifndef __PLATFORM_SERVICE_H__
#define __PLATFORM_SERVICE_H__

#include "talk_api.h"
#include "talk_server.h"

#include "nmp_proto_impl.h"
#include "nmp_proxy_device.h"
#include "nmp_proxy_server.h"

typedef void/*RTSP_server*/ rtsp_server_t;
typedef void/*Reverse_connector*/ reverse_cntr_t;

enum
{
    RTSP_UNINIT,
    RTSP_INIT,
    RTSP_STOP_AVS = RTSP_INIT,
    RTSP_START_AVS,
    RTSP_STOP_PF_SRV = RTSP_START_AVS,
    RTSP_START_PF_SRV,
};


struct platform_service_basic
{
    service_template_t tm;

    nmp_net_t         *_2lp_net;      /* 2 levels protocol network object. */
    packet_opt_t *pack_opt;
    rtsp_server_t*rtsp_srv;
    talk_hdl     *talk_srv;
    msg_engine_t *plt_me;
};


typedef struct plt_parm plt_parm_t;

struct plt_parm
{
    nmp_mutex_t      *lock;
    nmpio_t      *io;
    CONN_STATE_E state;
    unsigned int state_timer;
    unsigned int ttl;
    unsigned int ttd;
    unsigned int seq_generator;
    unsigned int unregistered;
};

typedef struct plt_rtsp plt_rtsp_t;

struct plt_rtsp
{
    nmp_mutex_t      *lock;
    CONN_STATE_E state;
    unsigned int state_timer;
    reverse_cntr_t *rev_cntr;
};

typedef struct platform_service platform_service_t;

struct platform_service
{
    struct service base;
    plt_parm_t     parm;
    plt_rtsp_t     rtsp;
    nmp_2proto_t        proto;
    proxy_device_t*owner;        /* point back to device object. */
};


#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
    }
#endif


#endif  //__PLATFORM_SERVICE_H__

