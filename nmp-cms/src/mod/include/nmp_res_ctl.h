
#ifndef __NMP_RESCTL_H__
#define __NMP_RESCTL_H__

#include <glib.h>
#include "nmp_msg_share.h"
#include "nmp_wdd.h"

enum
{
	RESOURCE_PTZ = 0,
	RESOURCE_MAX
};

typedef enum
{
	WDD_EXPIRED_DATE = 0,
	WDD_REMAINED_TIME,
	WDD_FOREVER_TIME,
	WDD_DOG_INEXISTENT,
	WDD_DOG_FORBIDDEN
} WDD_TIME_TYPE;

#define SYS_MODULE_CMS         0
#define SYS_MODULE_MDS         1
#define SYS_MODULE_MSS         2
#define SYS_MODULE_ALM         3
#define SYS_MODULE_EM         4
#define SYS_MODULE_TW         5

#define MODULE_CMS_BIT	(1<<SYS_MODULE_CMS)
#define MODULE_MDS_BIT	(1<<SYS_MODULE_MDS)
#define MODULE_MSS_BIT	(1<<SYS_MODULE_MSS)
#define MODULE_ALM_BIT	(1<<SYS_MODULE_ALM)
#define MODULE_EM_BIT	      (1<<SYS_MODULE_EM)
#define MODULE_TW_BIT	      (1<<SYS_MODULE_TW)

#define MF_HIK    "HIK"
#define MF_DAH    "DAH"
#define MF_HBN    "HBN"
#define MF_HNW   "HNW"
#define MF_XMT    "XMT"
#define MF_TPS    "TPS"
#define MF_JXJ     "JXJ"
#define MF_ENC    "ENC"

#define 	CMS_MF_HIK       0            /* 海康威视 */
#define 	CMS_MF_DAH      1        /* 浙江大华 */
#define 	CMS_MF_HBGK    2            /* 汉邦高科 */
#define 	CMS_MF_HNW     3            /* 霍尼韦尔 */
#define 	CMS_MF_XMT      4            /* 杭州雄迈 */
#define 	CMS_MF_TPS       5            /* 深圳天视通 */

#define MF_HIK_BIT	(1<<CMS_MF_HIK)
#define MF_DAH_BIT	(1<<CMS_MF_DAH)
#define MF_HBGK_BIT	(1<<CMS_MF_HBGK)
#define MF_HNW_BIT	(1<<CMS_MF_HNW)
#define MF_XMT_BIT	(1<<CMS_MF_XMT)
#define MF_TPS_BIT	(1<<CMS_MF_TPS)

#define CMS_TOUR       0            /*  巡回*/
#define CMS_GROUP      1        /* 群组 */
#define CMS_KEYBOARD 2       /* 模拟键盘 */

#define TW_TOUR_BIT	(1 << CMS_TOUR)
#define TW_GROUP_BIT	(1 << CMS_GROUP)
#define TW_KEYBOARD_BIT	(1 << CMS_KEYBOARD)


#define	ACTION_RECORD  0         /* 联动录像 */
#define	ACTION_SMS  1             /* 联动短信 */
#define	ACTION_MMS  2             /* 联动彩信 */
#define	ACTION_CAP   3			/* 联动抓拍 */
#define	ACTION_PRESET  4          /* 联动预置 */
#define	ACTION_TW  5           /* 联动电视墙 */
#define	ACTION_IO  6              /* 联动IO告警 */
#define	ACTION_EMAIL	7	    /* 联动EMAIL */
#define	ACTION_EMAP	8         /* 联动电子地图 */


#define ACTION_RECORD_BIT	(1<<ACTION_RECORD)
#define ACTION_SMS_BIT	(1<<ACTION_SMS)
#define ACTION_MMS_BIT	(1<<ACTION_MMS)
#define ACTION_CAP_BIT	(1<<ACTION_CAP)
#define ACTION_PRESET_BIT	(1<<ACTION_PRESET)
#define ACTION_TW_BIT	(1<<ACTION_TW)
#define ACTION_IO_BIT	(1<<ACTION_IO)
#define ACTION_EMAIL_BIT	(1<<ACTION_EMAIL)
#define ACTION_EMAP_BIT	(1<<ACTION_EMAP)


typedef struct __JpfResource JpfResource;
struct __JpfResource
{
	gint weight;	//last weight of operator, 0 means no
	gint64 last_ts;
};


typedef struct __JpfResourcesCtl JpfResourcesCtl;
struct __JpfResourcesCtl
{
	JpfResource res[RESOURCE_MAX];
};


typedef struct __JpfResourcesCap JpfResourcesCap;
struct __JpfResourcesCap
{
	guint system_version;
	guint module_bits;
	guint modules_data[MODULES_MAX];
	JpfExpiredTime expired_time;
	gint dev_count;
	gint av_count;
	gint ds_count;
	gint ai_count;
	gint ao_count;
};

void jpf_mod_init_resource(JpfResourcesCtl *res);
gint  jpf_mod_ctl_resource(JpfResourcesCtl *res, gint type, gint weight);
void jpf_mod_init_resource_cap();
gint jpf_mod_get_capability_av();
gint jpf_mod_get_capability_ds();
gint jpf_mod_get_capability_ai();
gint jpf_mod_get_capability_ao();
void jpf_mod_set_resource_cap(JpfResourcesCap *res_cap);
void jpf_mod_get_resource_cap(JpfResourcesCap *req_cap);

#endif
