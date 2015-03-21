#include <errno.h>
#include <string.h>
#include "nmp_res_ctl.h"
#include "nmp_sysctl.h"
#include "nmp_internal_msg.h"

#define MAX_CONTROL_TIME   60

JpfResourcesCap jpf_resource_cap;


void nmp_mod_init_resource(JpfResourcesCtl *res)
{
    memset(res, 0, sizeof(JpfResourcesCtl));
}


gint  nmp_mod_ctl_resource(JpfResourcesCtl *res, gint type, gint weight)
{
	time_t t = time(NULL);

    if (res->res[type].weight <= weight ||
        t < res->res[type].last_ts ||
        t > (res->res[type].last_ts + MAX_CONTROL_TIME))
    {
        res->res[type].weight = weight;
	 res->res[type].last_ts = t;
	 return 0;
    }

    return -EBUSY;
}


static __inline__ void __nmp_mod_init_resource_cap(JpfResourcesCap *cap)
{
	memset(cap, 0, sizeof(JpfResourcesCap));
	cap->expired_time.type = WDD_AUTH_NO_SOFTDOG;
}

void nmp_mod_init_resource_cap()
{
	__nmp_mod_init_resource_cap(&jpf_resource_cap);
}


gint nmp_mod_get_capability_av()
{
	return jpf_resource_cap.av_count;
}


gint nmp_mod_get_capability_ds()
{
	return jpf_resource_cap.ds_count;
}


gint nmp_mod_get_capability_ai()
{
	return jpf_resource_cap.ai_count;
}


gint nmp_mod_get_capability_ao()
{
	return jpf_resource_cap.ao_count;
}

void nmp_mod_set_resource_cap(JpfResourcesCap *res_cap)
{
	memcpy(&jpf_resource_cap, res_cap, sizeof(jpf_resource_cap));;
}

void nmp_mod_get_resource_cap(JpfResourcesCap *req_cap)
{
	memcpy(req_cap, &jpf_resource_cap, sizeof(jpf_resource_cap));;
}


