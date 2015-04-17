#ifndef __XMT_SRV_IMPL_H__
#define __XMT_SRV_IMPL_H__

#include "xmtnetsdk.h"

#include "nmp_proxy_sdk.h"

#ifdef __cplusplus
extern "C" {
#endif

extern handler_table_t xmt_handler_table[];

void xmt_reconnect_call_back(long user_id, char *dev_ip, long dev_port, unsigned long user_data);
void xmt_disconnet_call_back(long user_id, char *dev_ip, long dev_port, unsigned long user_data);
bool xmt_message_call_back(long user_id, char *buf, unsigned long buf_len, long user);


#ifdef __cplusplus
    }
#endif


#endif  //__XMT_SRV_IMPL_H__

