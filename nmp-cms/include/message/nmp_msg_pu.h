#ifndef __NMP_MOD_PU_MESSAGES_EXTERNAL_H__
#define __NMP_MOD_PU_MESSAGES_EXTERNAL_H__

#include "nmp_msg_share.h"

typedef struct _JpfPuRegInfo JpfPuRegInfo;
struct _JpfPuRegInfo
{
    gchar       puid[MAX_ID_LEN];
    gchar       cms_ip[MAX_IP_LEN];
    gchar       pu_ip[MAX_IP_LEN];
    gint          pu_type;
    gint     pu_version;
};

typedef struct _JpfPuRegRes JpfPuRegRes;
struct _JpfPuRegRes
{
    JpfMsgErrCode     code;
    gchar        puid[MAX_ID_LEN];
    gchar        mdu_ip[MAX_IP_LEN];
    gint         mdu_port;
    gint         keep_alive_time;
};

typedef struct _JpfPuHeart JpfPuHeart;
struct _JpfPuHeart
{
    gchar        puid[MAX_ID_LEN];
    gchar        pu_ip[MAX_IP_LEN];
};

typedef struct _JpfPuHeartResp JpfPuHeartResp;
struct _JpfPuHeartResp
{
    JpfMsgErrCode	code;
    gchar        puid[MAX_ID_LEN];
    gchar			server_time[TIME_INFO_LEN];
};

typedef struct _JpfGetMdsInfo JpfGetMdsInfo;
struct _JpfGetMdsInfo
{
    gchar       puid[MAX_ID_LEN];
    gchar       cms_ip[MAX_IP_LEN];
};

typedef struct _JpfGetMdsInfoRes JpfGetMdsInfoRes;
struct _JpfGetMdsInfoRes
{
    JpfMsgErrCode     code;
    gchar        puid[MAX_ID_LEN];
    gchar        mdu_ip[MAX_IP_LEN];
    gint         mdu_port;
    gint         keep_alive_time;
};


typedef struct _JpfPuOnlineStatusChange JpfPuOnlineStatusChange;
struct _JpfPuOnlineStatusChange
{
    gchar       domain_id[DOMAIN_ID_LEN];
    gchar       puid[MAX_ID_LEN];
    gchar       pu_ip[MAX_IP_LEN];
    gint          new_status;   //pu online status
    gchar       cms_ip[MAX_IP_LEN];
    gchar       pu_last_alive_time[TIME_INFO_LEN];
};


typedef struct _JpfPuOwnToAllCu JpfPuOwnToAllCu;
struct _JpfPuOwnToAllCu
{
    JpfMsgErrCode      code;
    JpfPuOnlineStatusChange  pu_state;
    gint total_num;
    JpfAllCuOwnPu cu_list[0];
};

typedef struct _JpfTwScr JpfTwScr;
struct _JpfTwScr
{
    gint tw_id;
    gint scr_id;
};

typedef struct _JpfPuTwScr JpfPuTwScr;
struct _JpfPuTwScr
{
    JpfMsgErrCode      code;
    gint state;
    gint total_num;
    JpfTwScr scr_list[0];
};

typedef struct _JpfChangeDispatch JpfChangeDispatch;
struct _JpfChangeDispatch
{
   gchar        puid[MAX_ID_LEN];
};

typedef struct _JpfSubmitFormatPos JpfSubmitFormatPos;
struct _JpfSubmitFormatPos
{
    gchar        domain_id[DOMAIN_ID_LEN];
    gchar        puid[MAX_ID_LEN];
    gint           disk_no;
    gint           format_pro;
};

typedef struct _JpfSubmitFormatPosRes JpfSubmitFormatPosRes;
struct _JpfSubmitFormatPosRes
{
    JpfMsgErrCode      code;
	JpfSubmitFormatPos     	pro;
    gint                 total_num;
    JpfAllCuOwnPu cu_list[0];
};

typedef struct _JpfSubmitAlarm JpfSubmitAlarm;
struct _JpfSubmitAlarm
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

typedef struct _JpfSubmitAlarmRes JpfSubmitAlarmRes;
struct _JpfSubmitAlarmRes
{
    JpfMsgErrCode      code;
    JpfSubmitAlarm     	alarm;
    gint                 total_num;
    JpfAllCuOwnPu cu_list[0];
};

typedef struct _JpfPuGetDivMode JpfPuGetDivMode;
struct _JpfPuGetDivMode
{
    gchar  puid[MAX_ID_LEN];
    gint req_num;
    gint start_num;
};

#endif   //__NMP_MOD_PU_MESSAGES_EXTERNAL_H__



