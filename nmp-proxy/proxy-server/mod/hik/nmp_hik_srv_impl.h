#ifndef __HIK_SRV_IMPL_H__
#define __HIK_SRV_IMPL_H__

#include "HCNetSDK.h"
#include "nmp_proxy_sdk.h"

#ifdef __cplusplus
extern "C" {
#endif

extern handler_table_t hik_handler_table[];

void hik_exception_call_back(DWORD type, LONG user_id, LONG handle, void *user_data);
BOOL hik_message_call_back(LONG cmd, char *dev_ip, char *buf, DWORD buf_len);


#ifdef __cplusplus
    }
#endif


#endif  //__HIK_SRV_IMPL_H__


