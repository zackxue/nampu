#ifndef __NMP_WDD_H__
#define __NMP_WDD_H__

#include <stdint.h>

#define DEV_PASSWD 				"918273645918273645918273"
#define USER_PASSWD			"91827345"
#define WDD_VERSION_1			0x01
#define WDD_LABEL_LEN			16
#define WDD_HARDWARE_ID_LEN	8
#define WDD_LABEL				"JPF-WDD-1"
#define WDD_MAKER_LABEL		"JPF-WDD-MAKER"
#define WDD_DECODER_LABEL		"JPF-WDD-DECODER"
#define WDD_TIME_LEN			32
#define MAKER_NAME_LEN			32
#define MAKER_PASSWD_LEN		16

#define LIMIT_MAX               ((uint32_t)-1)

#define SET_WDD_VERSION(_w, vernum) \
    ((wdd*)(_w))->version.ver_num = htonl(vernum)
#define GET_WDD_VERSION(_w) \
    ntohl(((wdd*)(_w))->version.ver_num)

#ifndef offsetof
#define offsetof(type, f) ((size_t) \
		((char *)&((type *)0)->f - (char *)(type *)0))
#endif


typedef struct __wdd_ver wdd_ver;
struct __wdd_ver
{
    uint32_t ver_num;       /* 数据结构版本：WDD_VERSION_1 */
};

typedef struct __wdd_label wdd_label;
struct __wdd_label
{
    uint8_t label[WDD_LABEL_LEN];   /* 卷标 */
};

typedef struct __wdd_cd wdd_cd;
struct __wdd_cd
{
    uint32_t major;					/* 主：批次*/
    uint32_t code;						/* 加密狗编号*/
    uint32_t maker_code;				/* 加密狗对应的制作者编号*/
    uint8_t hardware_id[WDD_HARDWARE_ID_LEN];	/* 硬件标识*/
};

enum {
    WDD_STYLE_DATE,
    WDD_STYLE_DAY,
    WDD_STYLE_MINUTE,
    WDD_STYLE_FOREVER
};

typedef struct __wdd_time wdd_time;
struct __wdd_time
{
    uint32_t style;         /* 计时方式 */

    /* 不激活绝对时间过期方式: WDD_STYLE_DATE */
    uint16_t expire_year;   /* 过期 年 */
    uint8_t expire_mon;     /* 过期 月 */
    uint8_t expire_date;    /* 过期 日 */

    /* 激活后天数过期方式:WDD_STYLE_DAY  */
    uint16_t start_year;    /* 激活时间 年 */
    uint8_t start_mon;      /* 激活时间 月 */
    uint8_t start_date;     /* 激活时间 日 */

    /* 使用分钟过期方式: WDD_STYLE_MINUTE */
    uint32_t expire_ttd;    /* 过期 还剩下的分钟数/或激活后还剩下的天数 */

};

enum {
    WDD_SYSTYPE_PF          /* 综合管理平台 */
};

enum {
    MODULES_CMS = 0,        /* 中心管理模块 */
    MODULES_MDS,            /* 转发模块 */
    MODULES_MSS,            /* 存储模块 */
    MODULES_ALM,            /* 报警管理模块 */
    MODULES_EM,             /* 电子地图模块 */
    MODULES_TW,             /* 电视墙模块 */
    MODULES_MAX = 32        /* 最大模块数 */
};

/* 中心管理子模块 */
enum{
	CMS_HIK = 0,            /* 海康威视 */
	CMS_DAH,                /* 浙江大华 */
	CMS_HBGK,               /* 汉邦高科 */
	CMS_HNW,               /* 霍尼韦尔 */
	CMS_XMT,               /* 杭州雄迈 */
	CMS_TPS,               /* 深圳天视通 */
};

/* 告警子模块 */
enum {
	LINKED_REC = 0,         /* 联动录像 */
	LINKED_MSG,             /* 联动短信 */
	LINKED_MMS,             /* 联动彩信 */
	LINKED_CAP,	              /* 联动抓拍 */
	LINKED_PRESET,          /* 联动预置 */
	LINKED_TW,                /* 联动电视墙 */
	LINKED_IO,                 /* 联动IO告警 */
	LINKED_EMAIL,           /* 联动EMAIL */
	LINKED_EMAP             /* 联动电子地图 */
};

enum {
	TW_TOUR = 0,            /* 巡回 */
	TW_GROUP,				/* 群组 */
	TW_KEYBOARD			/* 模拟键盘 */
};

enum {
	VER_STANDARD = 0,		/* 标准版 */
	VER_ENHANCED,			/* 增强版 */
	VER_ULTIMATE,			/* 旗舰版 */
	VER_NDMS,				/* NDMS版 */
	VER_TEST				/* 试用版 */
};

typedef struct __wdd_version1 wdd_version1;
struct __wdd_version1
{
    wdd_time authorize;
    uint8_t maker[MAKER_NAME_LEN];	/* 制作人 */
    uint8_t make_time[WDD_TIME_LEN];	/* 制作时间 */
    uint32_t sys_type;      /* 系统类型：WDD_SYSTYPE_PF-综合管理平台 */
    uint32_t sys_ver;		/* 系统版本 */
    uint32_t sys_modules;   /* 系统模块集合 */
    uint32_t modules_data[MODULES_MAX]; /* 各模块私有数据 */
    uint32_t sys_dec;       /* 允许接入的解码类型：软解码/硬解码 */
    uint32_t max_dev;       /* 允许接入的最大设备个数 */
    uint32_t max_av;        /* 音视频点最大个数, LIMIT_MAX代表无限制 */
    uint32_t max_ds;        /* 显示通道最大个数 */
    uint32_t max_ai;        /* 告警输入探头最大个数 */
    uint32_t max_ao;        /* 告警输出探头最大个数 */
};

typedef struct __wdd_maker0 wdd_maker0;
struct __wdd_maker0
{
    uint8_t	name[MAKER_NAME_LEN];	/* 制作人，即自己 */
    uint8_t	make_time[WDD_TIME_LEN];	/* 制作时间 */
    uint8_t	passwd[MAKER_PASSWD_LEN];	/* 密码 */
    uint32_t	authorize;					/* 制作人权限 */
    uint32_t	usb_keys;				/* 已制作USB-KEY的总个数 */
    uint32_t 	max_dev;       /* 设备个数 */
    uint32_t 	max_av;        /* 音视频点个数 */
    uint32_t 	max_ds;        /* 显示通道个数 */
    uint32_t 	max_ai;        /* 告警输入个数 */
    uint32_t 	max_ao;        /* 告警输出个数 */
};

typedef struct __wdd_decoder0 wdd_decoder0;
struct __wdd_decoder0
{
    wdd_time authorize;
    uint8_t maker[MAKER_NAME_LEN];	/* 制作人 */
    uint8_t make_time[WDD_TIME_LEN];	/* 制作时间 */
    uint32_t max_ds;        /* 显示通道最大个数 */
};


typedef struct __wdd_maker wdd_maker;
struct __wdd_maker
{
    wdd_label   label;
    wdd_cd      serial_code;
    wdd_ver     version;
    wdd_maker0	data;
};

typedef struct __wdd_decoder wdd_decoder;
struct __wdd_decoder
{
    wdd_label   label;
    wdd_cd      serial_code;
    wdd_ver     version;
    wdd_decoder0  data;
};

#define WDD_HEAD_LEN	(offsetof(wdd, data))

typedef struct __wdd wdd;
struct __wdd
{
    wdd_label   label;
    wdd_cd      serial_code;
    wdd_ver     version;
    wdd_version1 data;
};


#endif /* __NMP_WDD_H__ */
