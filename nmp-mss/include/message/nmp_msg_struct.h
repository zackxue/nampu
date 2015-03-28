#ifndef __NMP_MSG_STRUCT_H__
#define __NMP_MSG_STRUCT_H__

#include "nmp_share_struct.h"

/* Message Structs */

#define MAX_ID_LEN          32
#define MSS_ID_LEN			16
#define MDS_ID_LEN			16
//#define MAX_IP_LEN          32
#define DOMAIN_ID_LEN		32
#define TIME_INFO_LEN		32
#define MAX_URL_LEN         128
#define MAX_DAY_NUM         10
#define MAX_DATE_LEN        32
#define WEEKDAYS            7
#define TIME_SEG_LEN        32
#define TIME_SEG_NUM        8
#define USER_NAME_LEN       64
#define GROUP_NAME_LEN      64
#define HD_NAME_LEN         32
#define HD_GROUP_NUM        5
#define HD_GROUP_ID_LEN     8
#define SESSION_ID_LEN      16
#define TIME_LEN            64
#define FILE_PROPERTY_LEN   128
#define MAX_INITIATOR_NAME_LEN	128
#define MAX_TARGET_NAME_LEN		256
#define GENERAL_MSG_PARM_LEN		64
#define DESCRIPTION_INFO_LEN		512

typedef struct _NmpMsgErrCode NmpMsgErrCode;
struct _NmpMsgErrCode
{
	int			err_no;
};

#define RES_CODE(res) (((NmpMsgErrCode*)(res))->err_no)
#define SET_CODE(res, code) (RES_CODE(res) = (code))


typedef struct _NmpMssRegister NmpMssRegister;
struct _NmpMssRegister
{
    char mss_id[MSS_ID_LEN];
};

typedef struct _NmpmssRegisterRes NmpmssRegisterRes;
struct _NmpmssRegisterRes
{
    NmpMsgErrCode       code;
    char               domain_id[DOMAIN_ID_LEN];
    char               mss_id[MSS_ID_LEN];	
    int                keep_alive_time;
    gint               storage_type;
    gint               mode;	    
    char			server_time[TIME_INFO_LEN];
    char			mss_name[MSS_NAME_LEN];
};

typedef struct _NmpMssHeart NmpMssHeart;
struct _NmpMssHeart
{
    char		mss_id[MSS_ID_LEN];
};

typedef struct _NmpMssHeartRes NmpMssHeartRes;
struct _NmpMssHeartRes
{
    NmpMsgErrCode	code;
    char			server_time[TIME_INFO_LEN];
};

typedef struct _NmpMssGetGuid NmpMssGetGuid;
struct _NmpMssGetGuid
{
    char		mss_id[MSS_ID_LEN];
    int        req_num;
    int        start_row;
};

typedef struct _NmpMssGuid NmpMssGuid;
struct _NmpMssGuid
{
    char         guid[MAX_ID_LEN];
    char         domain_id[DOMAIN_ID_LEN];		
};

typedef struct _NmpMssGetGuidRes NmpMssGetGuidRes;
struct _NmpMssGetGuidRes
{
    NmpMsgErrCode	code;
    char		   mss_id[MSS_ID_LEN];
    int            total_count;		
    int            back_count;
    NmpMssGuid guid_info[0];
};

typedef struct _NmpMssGetRecordPolicy NmpMssGetRecordPolicy;
struct _NmpMssGetRecordPolicy
{
    char		 mss_id[MSS_ID_LEN];
    char         guid[MAX_ID_LEN];
    char         domain_id[DOMAIN_ID_LEN];	
};

typedef struct _NmpTimeSeg NmpTimeSeg;
struct _NmpTimeSeg
{
    char                time_seg[TIME_SEG_LEN]; 	
};

typedef struct _NmpWeekday NmpWeekday;
struct _NmpWeekday
{
    int            weekday;     //0表示周日, 1-6就表示周一到周六, 7表示每天
    int            time_seg_num;
    NmpTimeSeg     time_segs[TIME_SEG_NUM]; 	
};

typedef struct _Nmpdate Nmpdate;
struct _Nmpdate
{
    char           date[MAX_DATE_LEN];     //
    int            time_seg_num;
    NmpTimeSeg     time_segs[TIME_SEG_NUM]; 	
};

typedef struct _NmpRecordPolicy NmpRecordPolicy;
struct _NmpRecordPolicy
{
    int                weekday_num;
    NmpWeekday         weekdays[WEEKDAYS]; 	
    int                day_num;
    Nmpdate            day[MAX_DAY_NUM];	
};

typedef struct _NmpHdGroup NmpHdGroup;
struct _NmpHdGroup
{	
    gchar              hd_group_id[HD_GROUP_ID_LEN];	
};

typedef struct _NmpMssGetRecordPolicyRes NmpMssGetRecordPolicyRes;
struct _NmpMssGetRecordPolicyRes
{
    NmpMsgErrCode	code;
    char		    mss_id[MSS_ID_LEN];		
    char            guid[MAX_ID_LEN];
    char            domain_id[DOMAIN_ID_LEN];
    gint            level;	
    NmpHdGroup		hd_group[HD_GROUP_NUM];  
    NmpRecordPolicy record_policy;   

};

typedef struct _NmpMssGetRoute NmpMssGetRoute;
struct _NmpMssGetRoute
{
    char		  mss_id[MSS_ID_LEN];
    char          guid[MAX_ID_LEN];
    char          domain_id[DOMAIN_ID_LEN];
    char          cms_ip[MAX_IP_LEN];
};

typedef struct _NmpMssGetRouteRes NmpMssGetRouteRes;
struct _NmpMssGetRouteRes
{
    NmpMsgErrCode	code;
    char		    mss_id[MSS_ID_LEN];
	char            guid[MAX_ID_LEN];
    char            domain_id[DOMAIN_ID_LEN];
    char		    url[MAX_URL_LEN];
};

typedef struct _NmpMssGetMds NmpMssGetMds;
struct _NmpMssGetMds
{
    char	   mss_id[MSS_ID_LEN];
    int        req_num;
    int        start_row;
};

typedef struct _NmpMssMds NmpMssMds;
struct _NmpMssMds
{
    char         mds_id[MDS_ID_LEN];
};

typedef struct _NmpMssGetMdsRes NmpMssGetMdsRes;
struct _NmpMssGetMdsRes
{
    NmpMsgErrCode  code;
    int            total_count;		
    int            back_count;
    NmpMssMds      mds_info[0];
};

typedef struct _NmpMssGetMdsIp NmpMssGetMdsIp;
struct _NmpMssGetMdsIp
{
    gchar		mss_id[MSS_ID_LEN];
    gchar		mds_id[MDS_ID_LEN];	
    gchar       cms_ip[MAX_IP_LEN];			
};

typedef struct _NmpMssGetMdsIpRes NmpMssGetMdsIpRes;
struct _NmpMssGetMdsIpRes
{
    NmpMsgErrCode	code;
    gchar		    mss_id[MSS_ID_LEN];
    gchar		    mds_id[MDS_ID_LEN];		
    gchar           mds_ip[MAX_IP_LEN];		
    gint            port;	
};

typedef struct _NmpNotifyPolicyChange NmpNotifyPolicyChange;
struct _NmpNotifyPolicyChange
{
    gchar         guid[MAX_ID_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
    gint          all_changed;
};

typedef struct _NmpErrRes NmpErrRes;
struct _NmpErrRes
{
    NmpMsgErrCode     code;	
    gchar			   session[USER_NAME_LEN];	
};


enum {
	REC_SYNCING = 0,
	REC_WAITING,
	REC_PREPARE,
	REC_URL,
	REC_REQUETING,
	REC_RECORDING,
	REC_DYING,
	REC_NODISKS,
	REC_INITSTAT
};

typedef struct __NmpQueryRecordStatus NmpQueryRecordStatus;
struct __NmpQueryRecordStatus
{
	gchar   session[USER_NAME_LEN];
    gchar   mss_id[MSS_ID_LEN];
    gchar   guid[MAX_ID_LEN];
    gchar   domain_id[DOMAIN_ID_LEN];	
};

typedef struct __NmpQueryRecordStatusRes NmpQueryRecordStatusRes;
struct __NmpQueryRecordStatusRes
{
    NmpMsgErrCode code;
    gchar         session[USER_NAME_LEN];
    gint          status;
    gint          status_code;
};

typedef struct _NmpAddHdGroup NmpAddHdGroup;
struct _NmpAddHdGroup
{
    gchar   mss_id[MSS_ID_LEN];
    gchar   hd_group_name[GROUP_NAME_LEN];	
    gchar   session[USER_NAME_LEN];
};

typedef struct  _NmpAddHdGroupRes NmpAddHdGroupRes;
struct _NmpAddHdGroupRes
{
    NmpMsgErrCode     code;	
    gchar			   session[USER_NAME_LEN];
    gchar   mss_id[MSS_ID_LEN];	
    gint hd_group_id;
};

typedef struct _NmpAddHdToGroup NmpAddHdToGroup;
struct _NmpAddHdToGroup
{
    gchar     mss_id[MSS_ID_LEN];
    gint      hd_group_id;	
    gint      hd_id;		
    gchar	  session[USER_NAME_LEN];
};

typedef struct  _NmpAddHdToGroupRes NmpAddHdToGroupRes;
struct _NmpAddHdToGroupRes
{
    NmpMsgErrCode     code;	
    gchar			  session[USER_NAME_LEN];
};

typedef struct _NmpDelHdFromGroup NmpDelHdFromGroup;
struct _NmpDelHdFromGroup
{
    gchar   mss_id[MSS_ID_LEN];
    gint    hd_group_id;	
    gint    hd_id;		
    gchar	session[USER_NAME_LEN];
};

typedef struct  _NmpDelHdFromGroupRes NmpDelHdFromGroupRes;
struct _NmpDelHdFromGroupRes
{
    NmpMsgErrCode     code;	
    gchar			   session[USER_NAME_LEN];
};

typedef struct _NmpHdGroupInfo NmpHdGroupInfo;
struct _NmpHdGroupInfo
{
    gint hd_group_id;
    gchar   hd_group_name[GROUP_NAME_LEN];
};

typedef struct _NmpQueryAllHdGroup NmpQueryAllHdGroup;
struct _NmpQueryAllHdGroup
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpQueryAllHdGroupRes NmpQueryAllHdGroupRes;
struct _NmpQueryAllHdGroupRes
{
    NmpMsgErrCode     code;	
    gchar			   session[USER_NAME_LEN];
    gint                     total_num;	
    NmpHdGroupInfo    group_info[0];
};

typedef struct _NmpHdInfo NmpHdInfo;
struct _NmpHdInfo
{
	gchar hd_name[HD_NAME_LEN];
    gint hd_id;
    gint total_size;
    gint usedSize;
    gint hd_status;
    gint fs_type;
};

typedef struct _NmpQueryHdGroupInfo NmpQueryHdGroupInfo;
struct _NmpQueryHdGroupInfo
{
    gchar   mss_id[MSS_ID_LEN];
    gint      hd_group_id;	
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpQueryHdGroupInfoRes NmpQueryHdGroupInfoRes;
struct _NmpQueryHdGroupInfoRes
{
    NmpMsgErrCode     code;	
    gchar			   session[USER_NAME_LEN];
    gint                     total_num;	
    NmpHdInfo    hd_info[0];
};

typedef struct _NmpQueryAllHd NmpQueryAllHd;
struct _NmpQueryAllHd
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpQueryAllHdRes NmpQueryAllHdRes;
struct _NmpQueryAllHdRes
{
    NmpMsgErrCode     code;	
    gchar			   session[USER_NAME_LEN];
    gint                     total_num;	
    NmpHdInfo    hd_info[0];
};

typedef struct _NmpDelHdGroup NmpDelHdGroup;
struct _NmpDelHdGroup
{
    gchar   mss_id[MSS_ID_LEN];
    gint      hd_group_id;	
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpDelHdGroupRes NmpDelHdGroupRes;
struct _NmpDelHdGroupRes
{
    NmpMsgErrCode     code;	
    gchar			   session[USER_NAME_LEN];
};

typedef struct _NmpGetHdFormatProgress NmpGetHdFormatProgress;
struct _NmpGetHdFormatProgress
{
    gchar			   session[USER_NAME_LEN];
};

typedef struct _NmpGetHdFormatProgressRes NmpGetHdFormatProgressRes;
struct _NmpGetHdFormatProgressRes
{
    NmpMsgErrCode     code;		
    gchar			   session[USER_NAME_LEN];
    gint      hd_group_id;	
	gchar     hd_name[HD_NAME_LEN];
    gint      hd_id;
    gint      percent;    
};

typedef struct _NmpMssId NmpMssId;
struct _NmpMssId
{
    gchar   mss_id[MSS_ID_LEN];
};

typedef struct _NmpGetMssStoreLog NmpGetMssStoreLog;
struct _NmpGetMssStoreLog
{
    gchar		         session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                guid[MAX_ID_LEN];
    gchar                mss_id[MSS_ID_LEN];
    gchar                hd_group_id[HD_GROUP_ID_LEN];	
    gint                 record_type;	
    gchar                begin_time[TIME_LEN];
    gchar                end_time[TIME_LEN];
    gint                 begin_node;
    gint                 end_node;	
    gint                 sessId;
};

typedef struct _NmpStoreLog NmpStoreLog;
struct _NmpStoreLog
{
    gint                 record_type;
    gchar                begin_time[TIME_LEN];
    gchar                end_time[TIME_LEN];
    gchar                property[FILE_PROPERTY_LEN];	    
    gint                 file_size;
};

typedef struct _NmpGetStoreLogRes NmpGetStoreLogRes;
struct _NmpGetStoreLogRes
{
    NmpMsgErrCode      code;	
    gchar		       session[SESSION_ID_LEN];
    gchar              domain_id[DOMAIN_ID_LEN];
    gchar              guid[MAX_ID_LEN];		
    gint               sessId;	
    gint               total_num;
    gint               req_num;		
    NmpStoreLog        store_list[0];	    
};


typedef struct _NmpMssChange NmpMssChange;
struct _NmpMssChange
{
    gint      storage_type;  //磁盘类型：0：磁盘阵列，1：本地硬盘
    gint      mode;   
};


typedef struct _NmpMssAddOneIpsan NmpMssAddOneIpsan;
struct _NmpMssAddOneIpsan
{
	gchar		session[USER_NAME_LEN];
	NmpIpsanIp	info;
};

typedef struct  _NmpMssAddOneIpsanRes NmpMssAddOneIpsanRes;
struct _NmpMssAddOneIpsanRes
{
	NmpMsgErrCode	code;
	gchar			session[USER_NAME_LEN];
};


typedef struct _NmpMssGetIpsans NmpMssGetIpsans;
struct _NmpMssGetIpsans
{
	gchar		session[USER_NAME_LEN];
};

typedef struct _NmpMssGetIpsansRes NmpMssGetIpsansRes;
struct _NmpMssGetIpsansRes
{
	NmpMsgErrCode	code;
	gchar			session[USER_NAME_LEN];
	NmpIpsanInfo		info;
};


typedef struct _NmpMssSetIpsans NmpMssSetIpsans;
struct _NmpMssSetIpsans
{
	gchar		session[USER_NAME_LEN];
	NmpIpsanInfo	info;
};

typedef struct  _NmpMssSetIpsansRes NmpMssSetIpsansRes;
struct _NmpMssSetIpsansRes
{
	NmpMsgErrCode	code;
	gchar			session[USER_NAME_LEN];
};


typedef struct _NmpMssGetInitiatorName NmpMssGetInitiatorName;
struct _NmpMssGetInitiatorName
{
	gchar		session[USER_NAME_LEN];
};

typedef struct _NmpMssGetInitiatorNameRes NmpMssGetInitiatorNameRes;
struct _NmpMssGetInitiatorNameRes
{
	NmpMsgErrCode	code;
	gchar			session[USER_NAME_LEN];
	gchar			name[MAX_INITIATOR_NAME_LEN];
};


typedef struct _NmpMssSetInitiatorName NmpMssSetInitiatorName;
struct _NmpMssSetInitiatorName
{
	gchar		session[USER_NAME_LEN];
	gchar		name[MAX_INITIATOR_NAME_LEN];
};

typedef struct _NmpMssSetInitiatorNameRes NmpMssSetInitiatorNameRes;
struct _NmpMssSetInitiatorNameRes
{
	NmpMsgErrCode	code;
	gchar			session[USER_NAME_LEN];
};


typedef struct _NmpMssGetOneIpsanDetail NmpMssGetOneIpsanDetail;
struct _NmpMssGetOneIpsanDetail
{
	gchar		session[USER_NAME_LEN];
	NmpIpsanIp	info;
};

typedef struct _NmpMssGetOneIpsanDetailRes NmpMssGetOneIpsanDetailRes;
struct _NmpMssGetOneIpsanDetailRes
{
	NmpMsgErrCode	code;
	gchar			session[USER_NAME_LEN];
	gint				connect_state;
	gchar			target_name[MAX_TARGET_NAME_LEN];
};


typedef struct _NmpMssDelOneIpsan NmpMssDelOneIpsan;
struct _NmpMssDelOneIpsan
{
	gchar		session[USER_NAME_LEN];
	NmpIpsanIp	info;
};

typedef struct  _NmpMssDelOneIpsanRes NmpMssDelOneIpsanRes;
struct _NmpMssDelOneIpsanRes
{
	NmpMsgErrCode	code;
	gchar			session[USER_NAME_LEN];
};


typedef enum
{
	MSS_IPSAN_DISCONNECTED = 100, 
	MSS_IPSAN_CONNECTED
} MSS_NOTIFY_TYPE;

typedef struct _NmpNotifyMessage NmpNotifyMessage;
struct _NmpNotifyMessage
{
	gint		msg_id;  //消息ID，MSS_NOTIFY_TYPE
	gchar	param1[GENERAL_MSG_PARM_LEN];
	gchar	param2[GENERAL_MSG_PARM_LEN];
	gchar	param3[GENERAL_MSG_PARM_LEN];
	gchar	content[DESCRIPTION_INFO_LEN];
};


typedef struct _NmpAlarmLinkRecord NmpAlarmLinkRecord;
struct _NmpAlarmLinkRecord
{
	gchar	guid[MAX_ID_LEN];
	gchar	domain_id[DOMAIN_ID_LEN];
	guint	time_len;
	guint	alarm_type;
};


typedef struct _NmpSystemReboot NmpSystemReboot;
struct _NmpSystemReboot
{
	gchar		session[USER_NAME_LEN];
};


typedef struct _NmpSystemRebootRes NmpSystemRebootRes;
struct _NmpSystemRebootRes
{
	NmpMsgErrCode	code;
	gchar		session[USER_NAME_LEN];
};



/* internal msg struct */
typedef struct _NmpRegistedNotifyMessage NmpRegistedNotifyMessage;
struct _NmpRegistedNotifyMessage
{
	gint		if_query_ids;
};

typedef struct _NmpRegistedNotifyMessageRes NmpRegistedNotifyMessageRes;
struct _NmpRegistedNotifyMessageRes
{
	gint		if_query_ids;
};


#endif	/* __NMP_MSG_STRUCT_H__ */
