#ifndef __HBN_SDKINFO_H__
#define __HBN_SDKINFO_H__

#include "nmp_proxy_server.h"


#define HBN_STREAM_HEAD_SIZE        40
#define HBN_NVR_CHBNNEL_OFFSET      32

#define HBN_INVALID_HANDLE          0


typedef enum
{
    HBN_UNKNOWN=-1,
    HBN_LOGOUT = 0,
    HBN_LOGING = 1,
    HBN_LOGIN  = 2,
}HBN_STATE_E;

typedef struct get_store get_store_t;

struct get_store
{
    int  channel;
    void*buffer;
    int  b_size;
};

#endif  //__HBN_SDKINFO_H__

