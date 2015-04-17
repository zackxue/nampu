#ifndef __DAH_SRV_IMPL_H__
#define __DAH_SRV_IMPL_H__

#include "dhnetsdk.h"
#include "dhconfigsdk.h"

#include "nmp_proxy_sdk.h"

#ifdef __cplusplus
extern "C" {
#endif

extern handler_table_t dah_handler_table[];

void have_reconnect_call_back(LONG user_id, char *dev_ip, LONG dev_port, DWORD user_data);
void dah_disconnet_call_back(long int user_id, char *dev_ip, int dev_port, long int user_data);
BOOL dah_message_call_back(LONG command, LLONG user_id, char *buf, DWORD buf_len, char *dvr_ip, LONG port, LDWORD user);


#ifdef __cplusplus
    }
#endif


#endif  //__DAH_SRV_IMPL_H__


