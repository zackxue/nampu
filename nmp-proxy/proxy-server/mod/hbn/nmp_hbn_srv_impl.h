#ifndef __HBN_SRV_IMPL_H__
#define __HBN_SRV_IMPL_H__

#include "HBNetSDK.h"

#include "nmp_proxy_sdk.h"

#ifdef __cplusplus
extern "C" {
#endif

extern handler_table_t hbn_handler_table[];

void hbn_process_alarm_info(long command, HB_NET_ALARM *alarm_info);


#ifdef __cplusplus
    }
#endif


#endif  //__HBN_SRV_IMPL_H__



