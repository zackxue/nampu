#ifndef __RTSP_UTC_H__
#define __RTSP_UTC_H__

#include "def.h"

#define RTSP_UTC_STR_LEN sizeof("20090615T114900.440Z")

BEGIN_NAMESPACE


uint32_t rtsp_str2utc(char *utc_str);
char* rtsp_utc2str(uint32_t utc, char* utc_str);

END_NAMESPACE

#endif	//__RTSP_URL_H__
