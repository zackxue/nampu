/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_RTSP_DEF_H__
#define __TINY_RAIN_RTSP_DEF_H__

#include "def.h"

BEGIN_NAMESPACE

#define MAX_RTSP_BUF_SIZE       4096
#define MAX_TRANS_TEXT          128
#define MAX_HOST_LENGTH         128

#define TRACK_INDICATOR         "trackID="
#define TRACK_SUFFIX            "/"TRACK_INDICATOR  

enum
{
    MS_LIVE     = 0,
    MS_SNAP     = 1,
    MS_VOD      = 2,
    MS_DOWNLD   = 3,
    MS_PIC      = 4
};

enum
{
    RTP_TRANS_INVALID,
    RTP_OVER_TCP,
    RTP_OVER_UDP,
    RTP_OVER_RTSP
};

typedef struct __port_range port_range;
struct __port_range
{
    int32_t min_port;
    int32_t max_port;
};

typedef struct __rtp_port_conf rtp_port_conf;
struct __rtp_port_conf
{
    int32_t trans;  /* RTP_OVER_ */
    port_range ports;
    void *rtsp_client;
    uint8_t host[MAX_HOST_LENGTH];
};

END_NAMESPACE

#endif  //__TINY_RAIN_RTSP_DEF_H__
