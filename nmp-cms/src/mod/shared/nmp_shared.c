#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <glib.h>
#include <time.h>
#include "nmp_shared.h"
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_msg_share.h"
#include "nmp_shared.h"
#include "nmp_sysctl.h"
#include "nmp_res_ctl.h"


//gchar reg[100] = "[`~!@#$%^&*()+=|\\{}';:?]";


gchar g_domain_id[DOMAIN_ID_LEN] = {0};


gint
nmp_get_puid_from_guid(gchar *guid, gchar *puid)
{
    G_ASSERT(guid != NULL && puid != NULL);

    return sscanf(guid, "%16s", puid) != 1;
}

gint
nmp_get_channel_from_guid(gchar *guid, gint *channel)
{
	G_ASSERT(guid != NULL);

	gchar temp[GU_ID_LEN] = {0};

	return sscanf(guid, "%22s%2d", temp, channel) != 1;
}

gint
nmp_get_level_from_guid(gchar *guid, gint *level)
{
	G_ASSERT(guid != NULL);

	gchar temp[GU_ID_LEN] = {0};
	gint channel;

	return sscanf(guid, "%19s-%d-%2d", temp, level, &channel) != 1;
}


gint
nmp_set_guid_level(gchar *guid)
{
	G_ASSERT(guid != NULL);

	guid[20] = '0';
	return 0;
}


void
nmp_get_dev_type_from_puid(gchar *puid, gchar *dev_type)
{
    G_ASSERT(puid != NULL && dev_type != NULL);

    strncpy(dev_type, &puid[DEV_TYPE_LEN], DEV_TYPE_LEN - 1);
}


void
nmp_get_mf_from_guid(char *guid, char *mf)
{
    G_ASSERT(guid != NULL && mf != NULL);

    strncpy(mf, guid,  MF_LEN);
}


gint
nmp_get_utc_time(gchar *sys_time)
{
    G_ASSERT(sys_time != NULL);
    time_t timep;
    struct tm *p, tmp;

    timep = time(NULL);
    if (timep != (time_t)-1)
    {
        p = gmtime_r(&timep, &tmp);  //国际标准时间
	    snprintf(
	        sys_time, TIME_STRING_LEN - 1, "%d-%02d-%02d %02d:%02d:%02d",
	        (1900+p->tm_year), (1+p->tm_mon), p->tm_mday,p->tm_hour,
	        p->tm_min, p->tm_sec
	    );
	    return 0;
	}

    memset(sys_time, 0, TIME_STRING_LEN);
    return -errno;
}


gint
nmp_get_current_zone_time(gchar *sys_time)
{
    time_t timep;
    struct tm *p, tmp;
    G_ASSERT(sys_time != NULL);

    timep = time(NULL);
    if (timep != (time_t)-1)
    {
	    p = localtime_r(&timep, &tmp); //本地时间
	    snprintf(
	        sys_time, TIME_STRING_LEN - 1, "%d-%02d-%02d %02d:%02d:%02d",
	        (1900+p->tm_year), (1+p->tm_mon), p->tm_mday,p->tm_hour,
	        p->tm_min, p->tm_sec
	    );
	    return 0;
    }

    memset(sys_time, 0, TIME_STRING_LEN);
    return -errno;
}


gint nmp_set_system_time_zone(gchar *zone)
{
    gint ret = 0;

    if (zone)
    {
        ret = setenv("TZ", zone, 1);
        if (ret == -1)
        {
            nmp_warning( "Unable to set Time zone :%s", zone );
            return -errno;
        }
        tzset();
    }
    return ret;
}


gint nmp_set_system_time(gchar *times, gchar *zone)
{
    G_ASSERT(time != NULL);
    struct tm tm;
    struct tm _tm;
    struct timeval tv;
    time_t timep;

    printf("--------zone=%s,time=%s\n",zone,times);

    sscanf(
        times,
        "%d-%d-%d %d:%d:%d",
        &tm.tm_year,
        &tm.tm_mon,
        &tm.tm_mday,
        &tm.tm_hour,
        &tm.tm_min,
        &tm.tm_sec
        );

    _tm.tm_sec = tm.tm_sec;
    _tm.tm_min = tm.tm_min;
    _tm.tm_hour = tm.tm_hour;
    _tm.tm_mday = tm.tm_mday;
    _tm.tm_mon = tm.tm_mon - 1;
    _tm.tm_year = tm.tm_year - 1900;

    nmp_set_system_time_zone(zone);
    timep = mktime(&_tm);
    if (timep == (time_t)-1)
    {
        nmp_set_system_time_zone(nmp_get_sys_parm_str(SYS_PARM_TIMEZONE));
        return -errno;
    }

    nmp_set_system_time_zone(nmp_get_sys_parm_str(SYS_PARM_TIMEZONE));

    tv.tv_sec = timep;
    tv.tv_usec = 0;
    printf("----------set time second=%ld,now time=%ld\n",timep,time(NULL));
    if(settimeofday(&tv, NULL) < 0)
    {
        nmp_warning("Set system datatime error!");
	 perror("settimeofday");
        return -1;
    }

    system("hwclock --systohc");
    return 0;
}


gint
nmp_check_string(gchar *string, gint size)
{
    gchar ex_set[] = {'`','~','@', '#', '$', '%','^','&','*','(',')',' ','+','=','|','\\',',','.','/','<','>','?'};
    gint i, spec_no;
    #define ARRAY_SIZE(x)   (sizeof(x)/sizeof((x)[0]))

    for (i = 0; i < size; i++)
    {
        for (spec_no = 0; spec_no < ARRAY_SIZE(ex_set); spec_no++)
        {
            if (string[i] == ex_set[spec_no])
                return -E_STRINGFORMAT;
        }
    }

    return 0;
}


time_t
nmp_make_time_t(gchar *str)
{
	gint ret;
	struct tm tm;

	memset(&tm, 0, sizeof(tm));
	ret = sscanf(str, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon,
		&tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
	if (ret != 6)
		return (time_t)0;

	tm.tm_year -= 1900;
	tm.tm_mon -= 1;

	return mktime(&tm);
}

#if 0
gint regex(const gchar *string,const gchar *reg)
{
    GRegex* regex;   //正则表达式对象
    GMatchInfo *match_info = NULL;   //匹配后的集合
    GError *error = NULL;
    gint ret = 0;

    regex = g_regex_new(reg, 0, 0, &error);  //创建正则表达式
    g_regex_match(regex, string, 0, &match_info);   //匹配
    if(match_info)
    printf("---@@@@@@@@@--no spectial word\n");
    while (g_match_info_matches(match_info))
	{    //利用g_match_info_fetch把每一项fetch出来
    gchar* word = g_match_info_fetch(match_info, 0);
    printf("-&&&&&&$$$$$$$$$$$------spectical word:%s\n",word);
    g_free(word);
    g_match_info_next(match_info, NULL);
    ret = -E_STRINGFORMAT;
    }
    g_match_info_free(match_info);  //释放空间
    g_regex_unref(regex);
    return ret;
}
#endif

gint get_mached_string(const gchar *string,gchar *get_string, gint size, const gchar *reg)
{
    GRegex* regex;   //正则表达式对象
    GMatchInfo *match_info = NULL;   //匹配后的集合
    GError *error = NULL;
    gint ret = 0;

    regex = g_regex_new(reg, 0, 0, &error);  //创建正则表达式
    if (G_UNLIKELY(!regex))
        return FALSE;

    g_regex_match(regex, string, 0, &match_info);   //匹配
    if(G_UNLIKELY(!match_info))
    {
        g_regex_unref(regex);
        return FALSE;
    }

    if (!g_match_info_matches(match_info))
        ret = FALSE;

    while (g_match_info_matches(match_info))
    {    //利用g_match_info_fetch把每一项fetch出来
        gchar* word = g_match_info_fetch(match_info, 0);
        printf("-&&&&&&$$$$$$$$$$$------spectical word:%s\n",word);
        strncpy(get_string, word, size);
        g_free(word);
        g_match_info_next(match_info, NULL);
        ret = TRUE;
    }

    g_match_info_free(match_info);  //释放空间
    g_regex_unref(regex);

    return ret;
}


gboolean regex_mached(const gchar *string,const gchar *reg)
{
    GRegex* regex;   //正则表达式对象
    GMatchInfo *match_info = NULL;   //匹配后的集合
    GError *error = NULL;
    gint ret = TRUE;

    regex = g_regex_new(reg, 0, 0, &error);  //创建正则表达式
    if (G_UNLIKELY(!regex))
    	return FALSE;

    g_regex_match(regex, string, 0, &match_info);   //匹配
    if(G_UNLIKELY(!match_info))
    {
        g_regex_unref(regex);
    	 return FALSE;
    }

	if (!g_match_info_matches(match_info))
		ret = FALSE;

    g_match_info_free(match_info);  //释放空间
    g_regex_unref(regex);
	return ret;
}


gchar *nmp_get_local_domain_id()
{
	return g_domain_id;
}

void nmp_set_domain_id(gchar *value)
{
    g_domain_id[DOMAIN_ID_LEN - 1] = 0;
	strncpy(g_domain_id, value, DOMAIN_ID_LEN - 1);
	printf("==========g_domain_id=%s\n",g_domain_id);
}


void
__nmp_get_ip_from_socket(gint sock, gchar *ip, gint size)
{
    struct sockaddr_in ss;
    int len;
	char *p = NULL;

    len = sizeof(ss);
    getpeername(sock, (struct sockaddr*)&ss, (unsigned int *)&len);   //通过socket获取ip地址
    p = inet_ntoa(ss.sin_addr);
	if (p)
        strncpy(ip,p, size - 1); //获取IP地址
}

void
nmp_get_ip_from_socket(NmpNetIO *io, gchar *ip)
{
    gchar *tmp_ip;

    tmp_ip = nmp_net_get_io_peer_name(io);
    if (G_LIKELY(tmp_ip))
    {
        strncpy(ip, tmp_ip, MAX_IP_LEN - 1);
        ip[MAX_IP_LEN - 1] = 0;
        g_free(tmp_ip);
    }

}


void
nmp_get_host_ips(NmpHostIps *ips)
{
    struct ifaddrs *ifaddr = NULL, *ifs = NULL;
    void *addr = NULL;
    gchar addr_buf[INET_ADDRSTRLEN];

    memset(ips, 0, sizeof(*ips));
    if (getifaddrs(&ifaddr) == -1)
		return ;
    ifs = ifaddr;

    while (ifs)
    {
        if (ifs->ifa_addr->sa_family == AF_INET)
        {
            addr=&((struct sockaddr_in *)ifs->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, addr, addr_buf, INET_ADDRSTRLEN);
            strncpy(ips->ips[ips->count].ip, addr_buf, MAX_IP_LEN - 1);
            ++ips->count;
        }

	if (ips->count >= MAX_HOST_IPS)
	    break;

        ifs = ifs->ifa_next;
    }

    freeifaddrs(ifaddr);
}

void
nmp_check_keepalive_time(gint *keep_alive_time)
{
	if (*keep_alive_time > MAX_KEEPALIVE_SECS)
		*keep_alive_time = MAX_KEEPALIVE_SECS;

	if (*keep_alive_time <= 0)
	    *keep_alive_time = MIN_KEEPALIVE_SECS;
}


void
nmp_covert_pu_type(gint *new_type, gint *old_type)
{
	switch (*old_type){
	case TYPE_DVR:
	case TYPE_DVS:
	case TYPE_IPC:
		*new_type = *old_type;
		break;
	case TYPE_IPNC:
		*new_type = DBS_TYPE_IPC;
		break;
	case TYPE_SDEC:
		*new_type = DBS_TYPE_DEC;
		break;
	case TYPE_DEC:
		*new_type = DBS_TYPE_DEC;
		break;
	case TYPE_NVR:
		*new_type = DBS_TYPE_NVR;
		break;
      case TYPE_HVR:
		*new_type = DBS_TYPE_NVR;
		break;
      case TYPE_OTHER:
		*new_type = DBS_TYPE_OTH;
		break;
	default:
		*new_type = *old_type;
		break;
	}
}


gint
nmp_compare_manufacturer(guint module_data, gchar *mf)
{
    if (!strcmp(mf, MF_NMP) || !strcmp(mf, MF_ENC))
        return 0;

    if (!strcmp(mf, MF_HIK))
    {
        if (module_data&MF_HIK_BIT)
        	return 0;
        else
        	return -EPERM;
    }
    if (!strcmp(mf, MF_DAH))
    {
        if (module_data&MF_DAH_BIT)
        	return 0;
        else
        	return -EPERM;
    }
    if (!strcmp(mf, MF_HBN))
    {
        if (module_data&MF_HBGK_BIT)
        	return 0;
        else
        	return -EPERM;
    }
    if (!strcmp(mf, MF_HNW))
    {
        if (module_data&MF_HNW_BIT)
        	return 0;
        else
        	return -EPERM;
    }
    if (!strcmp(mf, MF_XMT))
    {
        if (module_data&MF_XMT_BIT)
        	return 0;
        else
        	return -EPERM;
    }
    if (!strcmp(mf, MF_TPS))
    {
        if (module_data&MF_TPS_BIT)
        	return 0;
        else
        	return -EPERM;
    }

    return -EPERM;
}

