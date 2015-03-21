#ifndef __NMP_MOD_SHARED_H__
#define __NMP_MOD_SHARED_H__

#include <glib.h>
#include "nmp_sysmsg.h"
#include "nmp_msg_share.h"


#define DEFALUT_CMS_IP  "0.0.0.0"
#define DECODER_TYPE "DEC"
#define ZONE_GMT8 "GMT-8"
#define MANUFACTUR_JXJ	"JXJ"
#define MANUFACTUR_JXJ_2	"ENC"

#define TIME_STRING_LEN          32
#define TIMEOUT_N_PERIODS     3
#define MAX_KEEPALIVE_SECS   999
#define MIN_KEEPALIVE_SECS   3
#define WAIT_AFTER_CONNECTED	3
#define DEV_TYPE_LEN  4
#define MF_LEN    3
#define MAX_HOST_IPS	8

#define guid_reg "[A-Z]{3}-[A-Z]{3}-\\d{8}-[A-Z]{2}-[A-Z0-9]{1}-\\d{2}"
#define int_reg  "[0-9]{1,6}"
#define puid_reg "[A-Z]{3}-[A-Z]{3}-\\d{8}"
#define mds_reg "[A-Z]{3}-[A-Z]{1}-\\d{3}"
#define mss_reg "[A-Z]{3}-\\d{4}"
#define domain_reg "[A-Z]{2}-\\d{5}"
#define manufactur_reg  "[A-Z]{3}"
#define exclude_reg "[\\+~`!@#$%^&\\*\\(\\)\\{\\}\\[\\]=|\\;:'\"\\<\\>\\?,\\s\\\\]"
#define mf_exclude_reg "[\\+~`!@#$%^&\\*\\(\\)\\{\\}\\[\\]=|\\;'\"\\<\\>\\?,\\s]"

#define  cms_debug(fmt, arg...)  \
	do{printf("FILE %s FUNC %s (%d)=>|"fmt,__FILE__, __FUNCTION__, __LINE__ , ##arg);	}while(0)


typedef enum
{
	MEDIA_VIDEO_BROWSE	= 0,
	MEDIA_SNAP,
	MEDIA_VIDEO_VOD,
	MEDIA_VIDEO_DOWN,
	MEDIA_PIC_BACK
}JpfMediaType;

typedef struct __JpfHostIp JpfHostIp;
struct __JpfHostIp
{
	gchar ip[MAX_IP_LEN];
};

typedef struct __JpfHostIps JpfHostIps;
struct __JpfHostIps
{
	gint count;
	JpfHostIp ips[MAX_HOST_IPS];
};

typedef enum
{
	TYPE_DVR = 0,
	TYPE_DVS,
	TYPE_IPC,
	TYPE_IPNC,
	TYPE_SDEC,
	TYPE_DEC,
	TYPE_NVR,
	TYPE_HVR,
	TYPE_OTHER = 255,
}JpfPuType;

typedef enum
{
	DBS_TYPE_DVR = 0,
	DBS_TYPE_DVS,
	DBS_TYPE_IPC,
	DBS_TYPE_OTH,
	DBS_TYPE_NVR,
	DBS_TYPE_DEC,
	DBS_TYPE_ALM,
}JpfDbsPuType;


#define NMP_COPY_VAL(dst_p, src_p, dst_len) do {	\
	dst_p[dst_len - 1] = '\0';	\
	strncpy(dst_p, src_p, dst_len - 1);	\
} while (0)


gint jpf_get_puid_from_guid(gchar *guid, gchar *puid);

gint jpf_get_channel_from_guid(gchar *guid, gint *channel);

void jpf_get_dev_type_from_puid(gchar *puid, gchar *dev_type);

void jpf_get_mf_from_guid(char *puid, char *mf);

gint jpf_get_level_from_guid(gchar *guid, gint *level);

gint jpf_set_guid_level(gchar *guid);

gint jpf_set_system_time(gchar *sys_time, gchar *zone);

gint
jpf_get_utc_time(gchar *sys_time);

gint
jpf_get_current_zone_time(gchar *sys_time);

gint jpf_check_string(gchar *string, gint size);

const gchar *nmp_app_core_get_domain();

gchar *jpf_get_local_domain_id();

void jpf_set_domain_id(gchar *value);

void jpf_check_keepalive_time(gint *keep_alive_time);

void
jpf_get_ip_from_socket(JpfNetIO *io, gchar *ip);

gboolean regex_mached(const gchar *string,const gchar *reg);

gint get_mached_string(const gchar *string,gchar *get_string,
	gint size, const gchar *reg);

time_t jpf_make_time_t(gchar *str);

void jpf_get_host_ips(JpfHostIps *ips);

gint jpf_set_system_time_zone(gchar *zone);

void
jpf_covert_pu_type(gint *new_type, gint *old_type);

gint
jpf_compare_manufacturer(guint module_data, gchar *mf);

#endif    //__NMP_MOD_SHARED_H__
