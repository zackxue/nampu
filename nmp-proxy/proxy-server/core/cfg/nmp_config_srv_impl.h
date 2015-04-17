
#ifndef __CONFIG_SRV_IMPL_H__
#define __CONFIG_SRV_IMPL_H__

#include "nmp_msg_impl.h"
#include "nmp_proxy_device.h"
#include "nmp_config_service.h"


#ifdef __cplusplus
extern "C" {
#endif

cfg_service_basic_t *
    config_get_service_basic(config_service_t *cfg_srv);
nmp_net_t *
    config_get_2lp_net(config_service_t *cfg_srv);
msg_engine_t *
    config_get_msg_engine(config_service_t *cfg_srv);

int config_new_client_connect(cfg_service_basic_t *cfg_basic, nmpio_t *io);
void config_finalize_client(config_service_t *cfg_srv, nmpio_t *io, int err);

void config_reap_timeout_client(cfg_service_basic_t *cfg_basic);


#ifdef __cplusplus
    }
#endif


#endif  //__CONFIG_SRV_IMPL_H__

