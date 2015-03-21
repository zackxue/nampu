/*
 *	@author:	zyt
 *	@time:	2013/1/9
 */
#ifndef __NMP_SHARE_STRUCT_H__
#define __NMP_SHARE_STRUCT_H__

#include "nmp_msg_share.h"



typedef struct _NmpTimeSeg NmpTimeSeg;
struct _NmpTimeSeg
{
	gint			seg_enable;		//时间段有效性:0-无效，1-有效
	gchar		time_seg[TIME_SEG_LEN];
};

typedef struct _NmpWeekday NmpWeekday;
struct _NmpWeekday
{
	gint			weekday;		//0表示周日, 1-6就表示周一到周六, 7表示每天
	gint			time_seg_num;
	NmpTimeSeg	time_segs[TIME_SEG_NUM];
};


typedef struct _NmpShareMssId NmpShareMssId;
struct _NmpShareMssId
{
    gchar			mss_id[MSS_ID_LEN];
};

typedef struct _NmpShareGuid NmpShareGuid;
struct _NmpShareGuid
{
	gchar		domain_id[DOMAIN_ID_LEN];
	gchar		guid[MAX_ID_LEN];
};

typedef struct _NmpSegment NmpSegment;
struct _NmpSegment
{
    gint  open;//开关
    gint  begin_sec; //开始时间秒数 3600*hour+60*min+sec
    gint  end_sec; //结束时间秒数 3600*hour+60*min+sec
};

typedef struct _IrcutTimerSwitch IrcutTimerSwitch;
struct _IrcutTimerSwitch
{
    	gint			weekday;		//0表示周日, 1-6就表示周一到周六, 7表示每天
	gint			time_seg_num;
    NmpSegment seg_time[TIME_SEG_NUM]; //时间段（4段）  in segment time open ir light, otherwise close it
};



#endif	/* __NMP_SHARE_STRUCT_H__ */
