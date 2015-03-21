#ifndef __NMP_MOD_PU_MESSAGES_EXTERNAL_H__
#define __NMP_MOD_PU_MESSAGES_EXTERNAL_H__

#include "nmp_msg_share.h"

typedef struct _NmpPuRegInfo NmpPuRegInfo;
struct _NmpPuRegInfo
{
    gchar       puid[MAX_ID_LEN];
    gchar       cms_ip[MAX_IP_LEN];
    gchar       pu_ip[MAX_IP_LEN];
    gint          pu_type;
    gint     pu_version;
};

typedef struct _NmpPuRegRes NmpPuRegRes;
struct _NmpPuRegRes
{
    NmpMsgErrCode     code;
    gchar        puid[MAX_ID_LEN];
    gchar        mdu_ip[MAX_IP_LEN];
    gint         mdu_port;
    gint         keep_alive_time;
};

typedef struct _NmpPuHeart NmpPuHeart;
struct _NmpPuHeart
{
    gchar        puid[MAX_ID_LEN];
    gchar        pu_ip[MAX_IP_LEN];
};

typedef struct _NmpPuHeartResp NmpPuHeartResp;
struct _NmpPuHeartResp
{
    NmpMsgErrCode	code;
    gchar        puid[MAX_ID_LEN];
    gchar			server_time[TIME_INFO_LEN];
};

typedef struct _NmpGetMdsInfo NmpGetMdsInfo;
struct _NmpGetMdsInfo
{
    gchar       puid[MAX_ID_LEN];
    gchar       cms_ip[MAX_IP_LEN];
};

typedef struct _NmpGetMdsInfoRes NmpGetMdsInfoRes;
struct _NmpGetMdsInfoRes
{
    NmpMsgErrCode     code;
    gchar        puid[MAX_ID_LEN];
    gchar        mdu_ip[MAX_IP_LEN];
    gint         mdu_port;
    gint         keep_alive_time;
};


typedef struct _NmpPuOnlineStatusChange NmpPuOnlineStatusChange;
struct _NmpPuOnlineStatusChange
{
    gchar       domain_id[DOMAIN_ID_LEN];
    gchar       puid[MAX_ID_LEN];
    gchar       pu_ip[MAX_IP_LEN];
    gint          new_status;   //pu online status
    gchar       cms_ip[MAX_IP_LEN];
    gchar       pu_last_alive_time[TIME_INFO_LEN];
};


typedef struct _NmpPuOwnToAllCu NmpPuOwnToAllCu;
struct _NmpPuOwnToAllCu
{
    NmpMsgErrCode      code;
    NmpPuOnlineStatusChange  pu_state;
    gint total_num;
    NmpAllCuOwnPu cu_list[0];
};

typedef struct _NmpTwScr NmpTwScr;
struct _NmpTwScr
{
    gint tw_id;
    gint scr_id;
};

typedef struct _NmpPuTwScr NmpPuTwScr;
struct _NmpPuTwScr
{
    NmpMsgErrCode      code;
    gint state;
    gint total_num;
    NmpTwScr scr_list[0];
};

typedef struct _NmpChangeDispatch NmpChangeDispatch;
struct _NmpChangeDispatch
{
   gchar        puid[MAX_ID_LEN];
};

typedef struct _NmpSubmitFormatPos NmpSubmitFormatPos;
struct _NmpSubmitFormatPos
{
    gchar        domain_id[DOMAIN_ID_LEN];
    gchar        puid[MAX_ID_LEN];
    gint           disk_no;
    gint           format_pro;
};

typedef struct _NmpSubmitFormatPosRes NmpSubmitFormatPosRes;
struct _NmpSubmitFormatPosRes
{
    NmpMsgErrCode      code;
	NmpSubmitFormatPos     	pro;
    gint                 total_num;
    NmpAllCuOwnPu cu_list[0];
};

typedef struct _NmpSubmitAlarm NmpSubmitAlarm;
struct _NmpSubmitAlarm
{
    gchar        domain_id[DOMAIN_ID_LEN];
    gchar        puid[MAX_ID_LEN];
    gchar        guid[MAX_ID_LEN];
    gchar        pu_name[PU_NAME_LEN];
    gchar        gu_name[GU_NAME_LEN];
    gint           alarm_type;
    gchar	      alarm_time[TIME_INFO_LEN];
    gint           channel;
    gint           action_type;
    gchar		alarm_info[ALARM_INFO_LEN];
    gint           alarm_id;
    gchar	      submit_time[TIME_INFO_LEN];
};

typedef struct _NmpSubmitAlarmRes NmpSubmitAlarmRes;
struct _NmpSubmitAlarmRes
{
    NmpMsgErrCode      code;
    NmpSubmitAlarm     	alarm;
    gint                 total_num;
    NmpAllCuOwnPu cu_list[0];
};

typedef struct _NmpPuGetDivMode NmpPuGetDivMode;
struct _NmpPuGetDivMode
{
    gchar  puid[MAX_ID_LEN];
    gint req_num;
    gint start_num;
};

#endif   //__NMP_MOD_PU_MESSAGES_EXTERNAL_H__



