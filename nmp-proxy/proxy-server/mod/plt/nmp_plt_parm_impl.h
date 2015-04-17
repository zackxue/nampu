
#ifndef __J_PLT_PARM_IMPL_H__
#define __J_PLT_PARM_IMPL_H__

#include "nmp_platform_service.h"


#ifdef __cplusplus
extern "C" {
#endif


void init_parm_module(platform_service_t *plt_srv, plt_parm_t *pm);
void cleanup_parm_module(platform_service_t *plt_srv, plt_parm_t *pm);

void control_parm_module(platform_service_t *plt_srv, int cmd, void *user);
void check_parm_module(platform_service_t *plt_srv, proxy_plt_t *plt_info);

void finalize_parm_io(platform_service_t *plt_srv, nmpio_t *io, int err);

#ifdef __cplusplus
    }
#endif

#endif  //__J_PLT_PARM_IMPL_H__

