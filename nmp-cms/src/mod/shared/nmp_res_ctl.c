#include <errno.h>
#include <string.h>
#include "nmp_res_ctl.h"
#include "nmp_sysctl.h"
#include "nmp_internal_msg.h"

#define MAX_CONTROL_TIME   60

NmpResourcesCap nmp_resource_cap;


void nmp_mod_init_resource(NmpResourcesCtl *res)
{
    memset(res, 0, sizeof(NmpResourcesCtl));
}


gint  nmp_mod_ctl_resource(NmpResourcesCtl *res, gint type, gint weight)
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


static __inline__ void __nmp_mod_init_resource_cap(NmpResourcesCap *cap)
{
	memset(cap, 0, sizeof(NmpResourcesCap));
	cap->expired_time.type = WDD_AUTH_NO_SOFTDOG;
}

void nmp_mod_init_resource_cap()
{
	__nmp_mod_init_resource_cap(&nmp_resource_cap);
}


gint nmp_mod_get_capability_av()
{
	return nmp_resource_cap.av_count;
}


gint nmp_mod_get_capability_ds()
{
	return nmp_resource_cap.ds_count;
}


gint nmp_mod_get_capability_ai()
{
	return nmp_resource_cap.ai_count;
}


gint nmp_mod_get_capability_ao()
{
	return nmp_resource_cap.ao_count;
}

void nmp_mod_set_resource_cap(NmpResourcesCap *res_cap)
{
	memcpy(&nmp_resource_cap, res_cap, sizeof(nmp_resource_cap));;
}

void nmp_mod_get_resource_cap(NmpResourcesCap *req_cap)
{
	memcpy(req_cap, &nmp_resource_cap, sizeof(nmp_resource_cap));;
}


