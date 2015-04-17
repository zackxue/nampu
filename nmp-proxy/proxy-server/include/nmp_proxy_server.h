/*
 *          file: nmp_proxy_server.h
 *          description:
 *
 *          May 16th, 2013
 */

#ifndef __PROXY_SERVICE_H__
#define __PROXY_SERVICE_H__

#include "nmplib.h"
#include "nmp_net.h"
#include "nmp_xmlmsg.h"

#include "nmp_proxy_log.h"
#include "nmp_proxy_info.h"
#include "nmp_proxy_errno.h"

#include "nmp_service_template.h"
#include "nmp_stream_operation.h"
#include "nmp_talk_operation.h"



typedef struct hik_service_basic      hik_service_basic_t;
typedef struct dah_service_basic      dah_service_basic_t;
typedef struct hbn_service_basic      hbn_service_basic_t;
typedef struct bsm_service_basic      bsm_service_basic_t;
typedef struct _jny_service_basic     jny_service_basic_t;
typedef struct hie_service_basic      hie_service_basic_t;
typedef struct xmt_service_basic      xmt_service_basic_t;
typedef struct tps_service_basic      tps_service_basic_t;


typedef struct platform_service_basic plt_service_basic_t;
typedef struct config_service_basic   cfg_service_basic_t;

typedef void (*fin_t)(void*, size_t);

struct service  /* 服务对象*/
{
    service_template_t *tm;     /* 服务模板 */
};

typedef struct proxy_ctrl proxy_ctrl_t;
typedef struct proxy_task proxy_task_t;


#ifdef __cplusplus
extern "C" {
#endif


int  proxy_server_loop();
void proxy_thread_pool_push(proxy_task_t *task);

proxy_ctrl_t *
    proxy_new_ctrl(struct service *srv, int cmd, void *user);
proxy_ctrl_t *
    proxy_new_ctrl_2(struct service *srv, int cmd, void *user, size_t size, fin_t fin);
void 
    proxy_free_ctrl(void *ctrl, size_t size);

proxy_task_t *
    proxy_new_task(int cmd, void *data, size_t size, fin_t fin, void *owner);
void 
    proxy_free_task(proxy_task_t *task);




struct service* 
     create_new_service(void *owner, int type, const char *name);
void destory_service(struct service *srv);

int check_service(struct service *srv);
int control_service(struct service *srv, int cmd, void *user);

int get_proxy_config(proxy_config_t *cfg);
int set_proxy_config(proxy_config_t *cfg);




#ifdef __cplusplus
    }
#endif


#endif  //__PROXY_SERVICE_H__

