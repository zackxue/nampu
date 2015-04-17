/*
 *          file: nmp_service_template.h
 *          description:抽象服务对象基类
 *
 *          hegui,heguijiss@gmail.com
 *          May 16th, 2013
 */

#ifndef __SERVICE_TEMPLATE_H__
#define __SERVICE_TEMPLATE_H__


typedef struct service_template service_template_t;

struct service_template
{
    char *service_name;

    int            (*init)           (struct service_template *self);
    int            (*cleanup)        (struct service_template *self);

    struct service*(*create_service) (struct service_template *self, void *init);
    void           (*delete_service) (struct service_template *self, struct service *srv);

    int            (*control_service)(struct service_template *self, struct service *srv, 
                                        int cmd, void *user);
    int            (*check_service)  (struct service_template *self, struct service *srv);
};

typedef struct remote_operate remote_operate_t;

struct remote_operate
{
    int (*get_config)(struct service *srv, int cmd, void *config);
    int (*set_config)(struct service *srv, int cmd, void *config);
};


#ifdef __cplusplus
extern "C" {
#endif

int add_service_template(service_template_t *srv_tm);
int rm_service_template(service_template_t *srv_tm);
service_template_t *
    find_service_template_by_name(const char *srv_name);

void cleanup_all_service_template();


#ifdef __cplusplus
    }
#endif


#endif  //__SERVICE_TEMPLATE_H__

