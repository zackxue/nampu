#include <time.h>

#include "rtsp_utc.h"

uint32_t rtsp_str2utc(char *utc_str)
{
    time_t ret;
    int year, mon, day, h, m, s;
    struct tm t;
	memset(&t, 0, sizeof(t));

    if(!utc_str)
    {
        return 0;
    }

    sscanf(utc_str, "%04d%02d%02dT%02d%02d%02d.%*dZ"
            , &year, &mon, &day, &h, &m, &s);

    t.tm_year = year-1900;
    t.tm_mon  = mon-1;
    t.tm_mday = day;
    t.tm_hour = h;
    t.tm_min  = m;
    t.tm_sec  = s;
	
    ret = mktime(&t);
    if(ret < 0) 
    {
        /* err */
        return 0;
    }
    return (uint32_t)ret;
}

char* rtsp_utc2str(uint32_t utc, char* utc_str)
{
    struct tm tm, *ret;
    time_t _utc;

    if(utc < 0)
    {
        utc = 0;
    }
    _utc = (utc < 0)?0:utc;
    
    if(!utc_str)
    {
        goto __error;
    }
    ret=localtime_r(&_utc, &tm);
    if(ret == NULL)
    {
        goto __error;
    }
    if(utc_str)
    {
        sprintf(utc_str, "%04d%02d%02dT%02d%02d%02d.00Z"
                , tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    }

    return utc_str;
__error:
    if(utc_str)
    {
        utc_str[0] = '\0';
    }
    return utc_str;
}




