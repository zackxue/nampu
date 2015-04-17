#ifndef __JNY_SDKINFO_H__
#define __JNY_SDKINFO_H__

#include "nmp_proxy_server.h"


#define JNY_STREAM_HEAD_SIZE		40
#define JNY_NVR_CHANNEL_OFFSET		32

#define JNY_INVALID_HANDLE          -1


typedef enum
{
    JNY_UNKNOWN=-2,
    JNY_LOGOUT =-1,
    JNY_LOGING = 0,
    JNY_LOGIN  = 1,
}JNY_STATE_E;



#endif	//__JNY_SDKINFO_H__


