#ifndef __TPS_SRV_IMPL_H__
#define __TPS_SRV_IMPL_H__

#include "NetSDKDLL.h"

#include "nmp_proxy_sdk.h"

#ifdef __cplusplus
extern "C" {
#endif

extern handler_table_t tps_handler_table[];

void tps_reconnect_call_back(long user_id, char *dev_ip, long dev_port, unsigned long user_data);
void tps_disconnet_call_back(long user_id, char *dev_ip, long dev_port, unsigned long user_data);
bool tps_message_call_back(long user_id, char *buf, unsigned long buf_len, long user);

LONG tps_status_event_call_back(LONG user_id, LONG state, char *resp, void *user);


#ifdef __cplusplus
    }
#endif


#endif  //__TPS_SRV_IMPL_H__

