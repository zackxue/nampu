
#ifndef __J_PLT_RTSP_IMPL_H__
#define __J_PLT_RTSP_IMPL_H__

#include "stream_api.h"
#include "rtsp-server.h"

#include "nmp_platform_service.h"


#ifdef __cplusplus
extern "C" {
#endif

void init_rtsp_module(platform_service_t *plt_srv, plt_rtsp_t *rm);
void cleanup_rtsp_module(platform_service_t *plt_srv, plt_rtsp_t *rm);

void control_rtsp_module(platform_service_t *plt_srv, int cmd, void *user);
void check_rtsp_module(platform_service_t *plt_srv, proxy_plt_t *plt_info);


#ifdef __cplusplus
    }
#endif

#endif  //__J_PLT_RTSP_IMPL_H__

