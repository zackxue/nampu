#ifndef __HIE_SDKINFO_H__
#define __HIE_SDKINFO_H__

#include "nmp_proxy_server.h"


#define HIE_STREAM_HEAD_SIZE        40
#define HIE_NVR_CHIENEL_OFFSET      32

#define HIE_INVALID_HANDLE          0


typedef enum
{
    HIE_UNKNOWN=-1,
    HIE_LOGOUT = 0,
    HIE_LOGING = 1,
    HIE_LOGIN  = 2,
}HIE_STATE_E;

typedef struct get_store get_store_t;

struct get_store
{
    int  channel;
    void*buffer;
    int  b_size;
};

#endif  //__HIE_SDKINFO_H__


