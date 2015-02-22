#ifndef __NMP_MOD_CU_MESSAGES_EXTERNAL_H__
#define __NMP_MOD_CU_MESSAGES_EXTERNAL_H__

#include "nmp_msg_share.h"
#include "nmp_list_head.h"
#include "nmp_sysctl.h"
#include "nmp_share_struct.h"
#include "nmp_tw_interface.h"


typedef struct _JpfCuExecuteRes JpfCuExecuteRes;
struct _JpfCuExecuteRes
{
    JpfMsgErrCode      code;
    gchar		session[SESSION_ID_LEN];
};

typedef struct _JpfCuLoginInfo JpfCuLoginInfo;
struct _JpfCuLoginInfo
{
    gchar	username[USER_NAME_LEN];
    gchar	password[USER_PASSWD_LEN];
    gint     cu_version;
};

typedef struct _JpfCuHeart JpfCuHeart;
struct _JpfCuHeart
{
    gchar			session[SESSION_ID_LEN];
};

typedef struct _JpfCuHeartResp JpfCuHeartResp;
struct _JpfCuHeartResp
{
    JpfMsgErrCode	code;
    gchar			server_time[TIME_INFO_LEN];
};


typedef struct _JpfCuLoginResp JpfCuLoginResp;
struct _JpfCuLoginResp
{
    JpfMsgErrCode	code;
    gchar			session[SESSION_ID_LEN];
    gchar			domain_name[DOMAIN_NAME_LEN];
    gchar			domain_id[DOMAIN_ID_LEN];
    gchar                root_area_name[AREA_NAME_LEN];
    gint                   root_area_id;
    gint                   usr_permissions;
    gchar                cu_min_version[VERSION_LEN];
    gchar                cms_version[VERSION_LEN];
    gint                  module_sets;
};


typedef struct _JpfForceUsrOffline JpfForceUsrOffline;
struct _JpfForceUsrOffline
{
    gint	reason;
};

typedef struct _JpfCuReqArea JpfCuReqArea;
struct _JpfCuReqArea
{
    gchar    username[USER_NAME_LEN];
    gchar    domain_id[DOMAIN_ID_LEN];
};


typedef struct _JpfGetArea JpfGetArea;
struct _JpfGetArea
{
    gchar			session[SESSION_ID_LEN];
    gint            req_num;
    gint            start_num;
    gint            area_id;
};

typedef struct _JpfArea  JpfArea;
struct _JpfArea
{
    gint     area_id;
    gchar  area_name[AREA_NAME_LEN];
    gint     area_parent;
};


typedef struct _JpfGetAreaRes JpfGetAreaRes;
struct _JpfGetAreaRes
{
    JpfMsgErrCode     code;
    gchar			         session[SESSION_ID_LEN];
    gint                     req_num;
    gint                     total_num;
    JpfArea               area_info[0];
};

typedef struct _JpfGetAreaInfo JpfGetAreaInfo;
struct _JpfGetAreaInfo
{
    gchar			session[SESSION_ID_LEN];
    gchar           username[USER_NAME_LEN];
    gint            area_id;
};

typedef struct _JpfGetAreaInfoRes JpfGetAreaInfoRes;
struct _JpfGetAreaInfoRes
{
    JpfMsgErrCode     code;
    gchar			     session[SESSION_ID_LEN];
    gint    gu_count;
    gchar  area_name[AREA_NAME_LEN];
    gchar  user_name[USER_NAME_LEN];
    gchar  user_phone[PHONE_NUM_LEN];
    gchar  user_address[DESCRIPTION_INFO_LEN];
    gchar  description[DESCRIPTION_INFO_LEN];
};

typedef struct _JpfGetDefenceArea JpfGetDefenceArea;
struct _JpfGetDefenceArea
{
    gchar		session[SESSION_ID_LEN];
    gint            req_num;
    gint            start_num;
    gint            area_id;
};

typedef struct _JpfDefenceArea  JpfDefenceArea;
struct _JpfDefenceArea
{
    gint     area_id;
    gchar  area_name[AREA_NAME_LEN];
    gint     is_defence_area; //是否是防区
    gint     defence_enable;
    gchar  policy[POLICY_LEN];
};


typedef struct _JpfGetDefenceAreaRes JpfGetDefenceAreaRes;
struct _JpfGetDefenceAreaRes
{
    JpfMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    JpfDefenceArea    area_info[0];
};

typedef struct _JpfGetDefenceMap JpfGetDefenceMap;
struct _JpfGetDefenceMap
{
    gchar		session[SESSION_ID_LEN];
    gint            req_num;
    gint            start_num;
    gint            defence_area_id;
};

typedef struct _JpfDefenceMap  JpfDefenceMap;
struct _JpfDefenceMap
{
    gint     map_id;
    gchar  map_name[MAP_NAME_LEN];
    gchar  map_location[MAP_LOCATION_LEN];
};


typedef struct _JpfGetDefenceMapRes JpfGetDefenceMapRes;
struct _JpfGetDefenceMapRes
{
    JpfMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    JpfDefenceMap    map_info[0];
};

typedef struct _JpfGetDefenceGu JpfGetDefenceGu;
struct _JpfGetDefenceGu
{
    gchar		session[SESSION_ID_LEN];
    gchar		username[USER_NAME_LEN];
    gint            req_num;
    gint            start_num;
    gint            map_id;
};

typedef struct _JpfDefenceGu  JpfDefenceGu;
struct _JpfDefenceGu
{
    gchar  guid[MAX_ID_LEN];
    gchar  domain_id[DOMAIN_ID_LEN];
    gchar  gu_name[GU_NAME_LEN];
    gint     gu_type;
    gint     pu_online_state;
    double            coordinate_x;
    double            coordinate_y;
};


typedef struct _JpfGetDefenceGuRes JpfGetDefenceGuRes;
struct _JpfGetDefenceGuRes
{
    JpfMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    JpfDefenceGu      defence_gu[0];
};

typedef struct _JpfGetMapHref JpfGetMapHref;
struct _JpfGetMapHref
{
    gchar		session[SESSION_ID_LEN];
    gint            req_num;
    gint            start_num;
    gint            map_id;
};

typedef struct _JpfMapHref JpfMapHref;
struct _JpfMapHref
{
    gint                dst_map_id;
    gchar             map_name[MAP_NAME_LEN];
    gchar             map_location[MAP_LOCATION_LEN];
    double            coordinate_x;
    double            coordinate_y;
};


typedef struct _JpfGetMapHrefRes JpfGetMapHrefRes;
struct _JpfGetMapHrefRes
{
    JpfMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    JpfMapHref          map_href[0];
};

typedef struct _JpfGetGuMapLocation JpfGetGuMapLocation;
struct _JpfGetGuMapLocation
{
    gchar	session[SESSION_ID_LEN];
    gchar  domain_id[DOMAIN_ID_LEN];
    gchar  guid[MAX_ID_LEN];
};

typedef struct _JpfGetGuMapLocationRes JpfGetGuMapLocationRes;
struct _JpfGetGuMapLocationRes
{
    JpfMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                    defence_area_id;
    gint                    map_id;
    gchar                 map_name[MAP_NAME_LEN];
    gchar                 map_location[MAP_LOCATION_LEN];
    double               coordinate_x;
    double               coordinate_y;
};

typedef struct _JpfGetTw JpfGetTw;
struct _JpfGetTw
{
    gchar		session[SESSION_ID_LEN];
    gint            req_num;
    gint            start_num;
    gchar           username[USER_NAME_LEN];
};

typedef struct _JpfCuTwInfo JpfCuTwInfo;
struct _JpfCuTwInfo
{
    gint                tw_id;
    gchar             tw_name[TW_NAME_LEN];
};


typedef struct _JpfGetTwRes JpfGetTwRes;
struct _JpfGetTwRes
{
    JpfMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    JpfCuTwInfo                  tw_info[0];
};

typedef struct _JpfGetScreen JpfGetScreen;
struct _JpfGetScreen
{
	gchar			session[SESSION_ID_LEN];
	gchar			user[USER_NAME_LEN];
	gint				req_num;
	gint				start_num;
	gint				tw_id;
};

typedef struct _JpfScreen JpfScreen;
struct _JpfScreen
{
    gint                   scr_id;
    gint                   screen_num;
    gchar                screen_name[SCREEN_NAME_LEN];
    double               coordinate_x;
    double               coordinate_y;
    double               length;
    double               width;
    gint                  online_state;
    gint                  pu_minor_type;
};


typedef struct _JpfGetScreenRes JpfGetScreenRes;
struct _JpfGetScreenRes
{
    JpfMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    JpfScreen            screen_info[0];
};

typedef struct _JpfGetScrState JpfGetScrState;
struct _JpfGetScrState
{
    gchar		session[SESSION_ID_LEN];
    gint           tw_id;
    gint           screen_id;
    tw_general_guid  guid;
};

typedef struct _JpfScrStateInfo JpfScrStateInfo;
struct _JpfScrStateInfo
{
    gint                div_num;
    gchar              enc_name[GU_NAME_LEN];
    gint                enc_channel;
    gint                level;
    gint                action_type;
    gint                 action_result;
};

typedef struct _JpfFullScrState JpfFullScrState;
struct _JpfFullScrState
{
    gint                mode;	//全屏模式 0:非全屏，1:全屏
    gchar              enc_name[GU_NAME_LEN];
    gint                enc_channel;
    gint                level;
    gint                action_type;
    gint                action_result;
};


typedef struct _JpfGetScrStateRes JpfGetScrStateRes;
struct _JpfGetScrStateRes
{
    JpfMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                    div_id;
    gint                    scr_lock_state;
    JpfFullScrState     full_scr_state;
    gint                     back_num;
    JpfScrStateInfo      scr_state_info[0];
};

typedef struct _JpfChangeDivMode JpfChangeDivMode;
struct _JpfChangeDivMode
{
    gchar		session[SESSION_ID_LEN];
    gint           tw_id;
    gint           screen_id;
    gint           div_id;
    tw_general_guid  guid;
};

typedef struct _JpfChangeDivModeRes JpfChangeDivModeRes;
struct _JpfChangeDivModeRes
{
    JpfMsgErrCode      code;
    gchar		session[SESSION_ID_LEN];
};

typedef struct _JpfRunStep JpfRunStep;
struct _JpfRunStep
{
    gchar		session[SESSION_ID_LEN];
    gint           tw_id;
    gint           screen_id;
    gint           div_id;
    gint           div_num;
    gchar         enc_name[GU_NAME_LEN];
    gchar 		 domain_id[DOMAIN_ID_LEN];
    gchar         guid[MAX_ID_LEN];
};

typedef struct _JpfRunStepRes JpfRunStepRes;
struct _JpfRunStepRes
{
    JpfMsgErrCode      code;
    gchar		session[SESSION_ID_LEN];
};

typedef struct _JpfFullScreen JpfFullScreen;
struct _JpfFullScreen
{
    gchar		session[SESSION_ID_LEN];
    gint           tw_id;
    gint           screen_id;
    gint           div_id;
    gint           div_num;
    tw_general_guid         guid;
};

typedef struct _JpfExitFullScreen JpfExitFullScreen;
struct _JpfExitFullScreen
{
    gchar		session[SESSION_ID_LEN];
    gint           tw_id;
    gint           screen_id;
    tw_general_guid         guid;
};

typedef struct _JpfGetTour JpfGetTour;
struct _JpfGetTour
{
    gchar		session[SESSION_ID_LEN];
    gint            req_num;
    gint            start_num;
    gchar           username[USER_NAME_LEN];
};

typedef struct _JpfCuTourInfo JpfCuTourInfo;
struct _JpfCuTourInfo
{
    gint                tour_id;
    gchar             tour_name[TOUR_NAME_LEN];
    gint                tour_num;
    gint               auto_jump;
};


typedef struct _JpfGetTourRes JpfGetTourRes;
struct _JpfGetTourRes
{
    JpfMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    JpfCuTourInfo                  tour_info[0];
};

typedef struct _JpfGetTourStep JpfGetTourStep;
struct _JpfGetTourStep
{
    gchar		session[SESSION_ID_LEN];
    gint            req_num;
    gint            start_num;
    gint            tour_id;
};

typedef struct _JpfCuTourStepInfo JpfCuTourStepInfo;
struct _JpfCuTourStepInfo
{
    gint                step_no;
    gint               interval;
    gchar             encoder_domain[DOMAIN_ID_LEN];
    gchar             encoder_guid[MAX_ID_LEN];
};


typedef struct _JpfGetTourStepRes JpfGetTourStepRes;
struct _JpfGetTourStepRes
{
    JpfMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    JpfCuTourStepInfo                  step_info[0];
};

typedef struct _JpfGetGroup JpfGetGroup;
struct _JpfGetGroup
{
	gchar			session[SESSION_ID_LEN];
	gchar			user[USER_NAME_LEN];
	gint				req_num;
	gint				start_num;
};

typedef struct _JpfCuGroupInfo JpfCuGroupInfo;
struct _JpfCuGroupInfo
{
    gint               group_id;
    gchar             group_name[GROUP_NAME_LEN];
    gint               group_num;
    gint               tw_id;
};


typedef struct _JpfGetGroupRes JpfGetGroupRes;
struct _JpfGetGroupRes
{
    JpfMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    JpfCuGroupInfo      group_info[0];
};

////////////

typedef struct _JpfAreaList JpfAreaList;
struct _JpfAreaList
{
    gint       area_id;
    gchar    area_name[AREA_NAME_LEN];
};


typedef struct _JpfCuReqAreaResp JpfCuReqAreaResp;
struct _JpfCuReqAreaResp
{
    gchar 			username[USER_NAME_LEN];
    gchar 			domain_id[DOMAIN_ID_LEN];
    gchar 			domain_name[DOMAIN_ID_LEN];
    gint  			result_code;
    gint 			area_num;
    JpfAreaList 	       area_list[0];
};


typedef struct _JpfCuReqDeviceList JpfCuReqDeviceList;
struct _JpfCuReqDeviceList
{
    gchar         username[USER_NAME_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
    gchar         area_id[AREA_NAME_LEN];
};


typedef struct _JpfGuList JpfGuList;
struct _JpfGuList
{
    gchar         puid[MAX_ID_LEN];
    gchar         guid[MAX_ID_LEN];
    gchar         gu_name[USER_NAME_LEN];
    gint            gu_type;
    gint            dome_right;
    gint		enable_config;
    gint            enable_talk;
    gint            enable_backplay;
    gint            is_online;
};

typedef struct _JpfCuReqDeviceListResp JpfCuReqDeviceListResp;
struct _JpfCuReqDeviceListResp
{
    gchar          username[USER_NAME_LEN];
    gchar          domain_id[DOMAIN_ID_LEN];
    gchar          domain_name[DOMAIN_ID_LEN];
    gint             area_id;
    gchar          area_name[AREA_NAME_LEN];
    gint             result_code;
    gint             gu_num;
    JpfGuList      gu_list[0];
};

//////////////
typedef struct _JpfGetDevice JpfGetDevice;
struct _JpfGetDevice
{
    gchar		  session[SESSION_ID_LEN];
    gchar           username[USER_NAME_LEN];
    gint              area_id;
    gint              req_num;
    gint              start_num;
};

typedef struct _JpfGu JpfGu;
struct _JpfGu
{
    struct list_head list;
    gchar                guid[MAX_ID_LEN];
    gchar                gu_name[GU_NAME_LEN];
    gint                   gu_num;
    gint                   gu_type;
    gint                   gu_attribute;

};

typedef struct _JpfDevice JpfDevice;
struct _JpfDevice
{
    struct list_head list;

    gchar         domain_id[DOMAIN_ID_LEN];
    gchar         puid[MAX_ID_LEN];
    gchar         pu_name[PU_NAME_LEN];
    gint            pu_type;
    gint            pu_state;

    gint           gu_num;
    JpfGu       *gu_list;
};

typedef struct _JpfGetDeviceRes JpfGetDeviceRes;
struct _JpfGetDeviceRes
{
    JpfMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint	                device_count;
    gint	                gu_count;
    JpfDevice *          device_list;
    GStaticMutex        list_lock;
};


typedef struct _JpfGetAreaDevice JpfGetAreaDevice;
struct _JpfGetAreaDevice
{
    gchar		  session[SESSION_ID_LEN];
    gchar           username[USER_NAME_LEN];
    gint              area_id;
    gint              req_num;
    gint              start_num;
};

typedef struct _JpfDeviceInfo JpfDeviceInfo;
struct _JpfDeviceInfo
{
    gchar         domain_id[DOMAIN_ID_LEN];
    gchar         puid[MAX_ID_LEN];
    gchar         pu_name[PU_NAME_LEN];
    gint            pu_type;
    gint            pu_state;
    gchar         pu_ip[MAX_IP_LEN];
    gchar         pu_last_alive_time[TIME_INFO_LEN];
};


typedef struct _JpfGetAreaDeviceRes JpfGetAreaDeviceRes;
struct _JpfGetAreaDeviceRes
{
    JpfMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint	                device_count;
    gint                  req_num;
    JpfDeviceInfo       device_list[0];
};

typedef struct  _JpfGetMediaUrl JpfGetMediaUrl ;
struct _JpfGetMediaUrl
{
    gchar			session[SESSION_ID_LEN];
    gchar			domain_id[DOMAIN_ID_LEN];
    gchar			guid[MAX_ID_LEN];
    gchar			mss_id[MSS_ID_LEN];
    gchar			ip[MAX_IP_LEN];
    gint			media;     //媒体类型
    gint			connect_mode;   //0:连接mds，1:直连设备
    gchar			cu_ip[MAX_IP_LEN];
    gchar              username[USER_NAME_LEN];
};

typedef struct  _JpfGetMediaUrlRes JpfGetMediaUrlRes;
struct   _JpfGetMediaUrlRes
{
    JpfMsgErrCode      code;
    gchar			   session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                guid[MAX_ID_LEN];
    gchar                ip[MAX_IP_LEN];
    gchar                url[MAX_URL_LEN];
    gchar                decode_path[FILE_PATH_LEN];
    gchar                decode_name[DECODE_NAME_LEN];
};

typedef struct _JpfGetPlatformInfo JpfGetPlatformInfo;
struct _JpfGetPlatformInfo
{
    gchar		       session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                puid[MAX_ID_LEN];
};


typedef struct _JpfGetDeviceInfo JpfGetDeviceInfo;
struct _JpfGetDeviceInfo
{
    gchar			   session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
};

typedef struct _JpfGetDeviceInfoErr JpfGetDeviceInfoErr;
struct _JpfGetDeviceInfoErr
{
    JpfMsgErrCode      code;
    gchar			   session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
};

typedef struct _JpfGetDeviceChannelInfo JpfGetDeviceChannelInfo;
struct _JpfGetDeviceChannelInfo
{
    gchar		           session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                guid[MAX_ID_LEN];
};

typedef struct _JpfGetDeviceInfoRes JpfGetDeviceInfoRes;
struct _JpfGetDeviceInfoRes
{
    JpfMsgErrCode     code;
    gchar			         session[SESSION_ID_LEN];
    gchar                  domain_id[DOMAIN_ID_LEN];
    gchar                  puid[MAX_ID_LEN];
    gchar                  manu_info[DESCRIPTION_INFO_LEN];
    gchar                  release_date[TIME_LEN];
    gchar                  dev_version[VERSION_LEN];
    gchar                  hardware_version[VERSION_LEN];
    gint                     pu_type;
    gint                     pu_sub_type;
    gint                     di_num;
    gint                     do_num;
    gint                     channel_num;
    gint                     rs232_num;
    gint                     rs485_num;
};

typedef struct _JpfGetPlatformInfoRes JpfGetPlatformInfoRes;
struct _JpfGetPlatformInfoRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   cms_ip[MAX_IP_LEN];
    gint                      cms_port;
    gchar                   mds_ip[MAX_IP_LEN];
    gint                      mds_port;
    gint                      protocol;    //tcp,udp
    gint                      is_conn_cms;
};

typedef struct _JpfSetPlatformInfo JpfSetPlatformInfo;
struct _JpfSetPlatformInfo
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   cms_ip[MAX_IP_LEN];
    gint                      cms_port;
    gchar                   mds_ip[MAX_IP_LEN];
    gint                      mds_port;
    gint                      protocol;    //tcp,udp
    gint                      is_conn_cms;
};

typedef struct _JpfNetworkInfo JpfNetworkInfo;
struct _JpfNetworkInfo
{
    gint                      network_type;
    gchar                   ip[MAX_IP_LEN];
    gchar                   netmask[MAX_IP_LEN];
    gchar                   gateway[MAX_IP_LEN];
    gchar                   mac[MAX_IP_LEN];
    gint                      dhcp_enable;
};

typedef struct _JpfGetNetworkInfoRes JpfGetNetworkInfoRes;
struct _JpfGetNetworkInfoRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   main_dns[MAX_IP_LEN];
    gchar                   sub_dns[MAX_IP_LEN];
    gint                      auto_dns_enable;
    gint                      cmd_port;
    gint                      data_port;
    gint                      web_port;
    JpfNetworkInfo 		network[3];
};

typedef struct _JpfSetNetworkInfo JpfSetNetworkInfo;
struct _JpfSetNetworkInfo
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   main_dns[MAX_IP_LEN];
    gchar                   sub_dns[MAX_IP_LEN];
    gint                      auto_dns_enable;
    gint                      cmd_port;
    gint                      data_port;
    gint                      web_port;
    JpfNetworkInfo 		network[3];
};


typedef struct _JpfGetPppoeInfoRes JpfGetPppoeInfoRes;
struct _JpfGetPppoeInfoRes
{
    JpfMsgErrCode      code;
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   account[USER_NAME_LEN];
    gchar                   passwd[USER_PASSWD_LEN];
    gint                      interfaces;
    gint                      pppoeEnable;
    gchar                   pppoeIp[MAX_IP_LEN];
};

typedef struct _JpfSetPppoeInfo JpfSetPppoeInfo;
struct _JpfSetPppoeInfo
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   account[USER_NAME_LEN];
    gchar                   passwd[USER_PASSWD_LEN];
    gint                      interfaces;
    gint                      pppoeEnable;
    gchar                   pppoeIp[MAX_IP_LEN];
};

typedef struct _JpfGetEncodeParaRes JpfGetEncodeParaRes;
struct _JpfGetEncodeParaRes
{
    JpfMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
 //   gint                      channel;
    gint                      level;      //type of code master,slave
    gint                      video_format;    //PAL,NTSC
    gint                      frame_rate;
    gint                      i_frame_interval;
    gint                      video_type;   //video encode type :H264/MPEG4
    gint                      resolution;		//D1,CIF,HCIF
    gint                      bit_rate_type;  //CBR,VBR
    gint                      Qp_value;
    gint                      code_rate;
    gint                      frame_priority;
    gint                      audio_enable;
    gint                      audio_type;   //audio encode type :G711A/G711u
    gint                      audio_input_mode;
    gint                      encodeLevel;
};

typedef struct _JpfSetDeviceInfo JpfSetDeviceInfo;
struct _JpfSetDeviceInfo
{
    JpfMsgErrCode       code;
    gchar		               session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                     puid[MAX_ID_LEN];
};

typedef struct _JpfSetDeviceInfoRes JpfSetDeviceInfoRes;
struct _JpfSetDeviceInfoRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                    puid[MAX_ID_LEN];
};

typedef struct _JpfSetDeviceParaRes JpfSetDeviceParaRes;
struct _JpfSetDeviceParaRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
};

typedef struct _JpfSetEncodePara JpfSetEncodePara;
struct _JpfSetEncodePara
{
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
  //  gint                      channel;
    gint                      level;      //type of code master,slave
    gint                      video_format;    //PAL,NTSC
    gint                      frame_rate;
    gint                      i_frame_interval;
    gint                      video_type;   //video encode type :H264
    gint                      resolution;		//D1,CIF,HCIF
    gint                      bit_rate_type;  //CBR,VBR
    gint                      Qp_value;
    gint                      code_rate;
    gint                      frame_priority;
    gint                      audio_enable;
    gint                      audio_type;
    gint                      audio_input_mode;
    gint                      encodeLevel;
};

typedef struct _JpfGetDisplayParaRes JpfGetDisplayParaRes;
struct _JpfGetDisplayParaRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      contrast;
    gint                      bright;
    gint                      hue;
    gint                      saturation;
    gint                      sharpness;
};

typedef struct _JpfSetDisplayPara JpfSetDisplayPara;
struct _JpfSetDisplayPara
{

    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      contrast;
    gint                      bright;
    gint                      hue;
    gint                      saturation;
    gint                      sharpness;
};

typedef struct _JpfGetOSDParaRes JpfGetOSDParaRes;
struct _JpfGetOSDParaRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      time_display_enable;
    gint                      time_display_x;
    gint                      time_display_y;
    gint                      time_display_color;
    gint                      text_display_enable;
    gint                      text_display_x;
    gint                      text_display_y;
    gint                      text_display_color;
    gchar                   text_display_data[TEXT_DATA_LEN];
    gint                      max_width;
    gint                      max_height;
    gint                      stream_enable;
    gint                      time_display_w;
    gint                      time_display_h;
    gint                      text_display_w;
    gint                      text_display_h;
};

typedef struct _JpfSetOSDPara JpfSetOSDPara;
struct _JpfSetOSDPara
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      time_display_enable;
    gint                      time_display_x;
    gint                      time_display_y;
    gint                      time_display_color;
    gint                      text_display_enable;
    gint                      text_display_x;
    gint                      text_display_y;
    gint                      text_display_color;
    gchar                   text_display_data[TEXT_DATA_LEN];
    gint                      max_width;
    gint                      max_height;
    gint                      stream_enable;
    gint                      time_display_w;
    gint                      time_display_h;
    gint                      text_display_w;
    gint                      text_display_h;
};


typedef struct _JpfGetRecordParaRes JpfGetRecordParaRes;
struct _JpfGetRecordParaRes
{
    JpfMsgErrCode      code;
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      level;
    gint                      auto_cover;
    gint                      pre_record;
    gint                      all_day_enable;
    gint                      weekday_num;
    JpfWeekday          weekdays[WEEKDAYS];
};

typedef struct _JpfSetRecordPara  JpfSetRecordPara;
struct _JpfSetRecordPara
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      level;
    gint                      auto_cover;
    gint                      pre_record;
    gint                      all_day_enable;
    gint                      weekday_num;
    JpfWeekday            weekdays[WEEKDAYS];
};


typedef struct _JpfRectangle JpfRectangle;
struct _JpfRectangle
{
    gint left;
    gint top;
    gint right;
    gint bottom;
};

typedef struct _JpfGetMoveAlarmParaRes JpfGetMoveAlarmParaRes;
struct _JpfGetMoveAlarmParaRes
{
    JpfMsgErrCode     code;
    gchar		             session[SESSION_ID_LEN];
    gchar                  domain_id[DOMAIN_ID_LEN];
    gchar                  guid[MAX_ID_LEN];
    gint                     move_enable; 					//移动侦测功能是否生效
    gint                     sensitive_level; 					//侦测灵敏度
    gint                     detect_interval; 					//侦测间隔时间
    gint                     detect_num;
    gint                     weekday_num;
    gint                     max_width;
    gint                     max_height;
    JpfRectangle        detect_area[DETECT_AREA_NUM]; 						//侦测区域
    JpfWeekday         weekdays[WEEKDAYS]; 						//移动侦测时间段：结束时间
};

typedef struct _JpfSetMoveAlarmPara JpfSetMoveAlarmPara;
struct _JpfSetMoveAlarmPara
{
    gchar		             session[SESSION_ID_LEN];
    gchar                  domain_id[DOMAIN_ID_LEN];
    gchar                  guid[MAX_ID_LEN];
    gint                     move_enable; 					//移动侦测功能是否生效
    gint                     sensitive_level; 					//侦测灵敏度
    gint                     detect_interval; 					//侦测间隔时间
    gint                     detect_num;
    gint                     weekday_num;
    gint                     max_width;
    gint                     max_height;
    JpfRectangle        detect_area[DETECT_AREA_NUM]; 						//侦测区域
    JpfWeekday         weekdays[WEEKDAYS]; 					//移动侦测时间段：结束时间
};

typedef struct _JpfGetVideoLostParaRes JpfGetVideoLostParaRes;
struct _JpfGetVideoLostParaRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                     lost_enable; 					//video lost是否生效
    gint                     detect_interval; 					//侦测间隔时间
    gint                      weekday_num;
    JpfWeekday         weekdays[WEEKDAYS]; 						//移动侦测时间段：结束时间
};

typedef struct _JpfSetVideoLostPara JpfSetVideoLostPara;
struct _JpfSetVideoLostPara
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      lost_enable; 					//video lost是否生效
    gint                      detect_interval; 					//侦测间隔时间
    gint                      weekday_num;
    JpfWeekday         weekdays[WEEKDAYS]; 					//移动侦测时间段：结束时间
};

typedef struct _JpfGetHideParaRes JpfGetHideParaRes;
struct _JpfGetHideParaRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      hide_enable;
    gint                      hide_color;
    gint                      max_width;
    gint                      max_height;
    gint                      detect_num;
    JpfRectangle         detect_area[DETECT_AREA_NUM];
};

typedef struct _JpfSetHidePara JpfSetHidePara;
struct _JpfSetHidePara
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      hide_enable;
    gint                      hide_color;
    gint                      max_width;
    gint                      max_height;
    gint                      detect_num;
    JpfRectangle         detect_area[DETECT_AREA_NUM];
};

typedef struct _JpfGetHideAlarmParaRes JpfGetHideAlarmParaRes;
struct _JpfGetHideAlarmParaRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      hide_enable; 					//video lost是否生效
    gint                      detect_interval; 					//侦测间隔时间
    gint                      max_width;
    gint                      max_height;
    gint                      weekday_num;
    gint                      detect_num;
    JpfRectangle         detect_area[DETECT_AREA_NUM];
    JpfWeekday          weekdays[WEEKDAYS]; 						//移动侦测时间段：结束时间
};

typedef struct _JpfSetHideAlarmPara JpfSetHideAlarmPara;
struct _JpfSetHideAlarmPara
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      hide_enable; 					//video lost是否生效
    gint                      detect_interval; 					//侦测间隔时间
    gint                      max_width;
    gint                      max_height;
    gint                      weekday_num;
    gint                      detect_num;
    JpfRectangle         detect_area[DETECT_AREA_NUM];
    JpfWeekday          weekdays[WEEKDAYS]; 					//移动侦测时间段：结束时间
};

typedef struct _JpfGetIOAlarmParaRes JpfGetIOAlarmParaRes;
struct _JpfGetIOAlarmParaRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      io_enable; 					//video lost是否生效
    gint                      io_type;
    gint                      detect_interval; 					//侦测间隔时间
    gint                      weekday_num;
    JpfWeekday          weekdays[WEEKDAYS]; 						//移动侦测时间段：结束时间
};

typedef struct _JpfSetIOAlarmPara JpfSetIOAlarmPara;
struct _JpfSetIOAlarmPara
{
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      io_enable; 					//video lost是否生效
    gint                      io_type;
    gint                      detect_interval; 					//侦测间隔时间
    gint                      weekday_num;
    JpfWeekday          weekdays[WEEKDAYS]; 					//移动侦测时间段：结束时间
};


typedef struct _JpfGetJointPara JpfGetJointPara;
struct _JpfGetJointPara
{
    JpfMsgErrCode       code;
    gchar		              session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    guid[MAX_ID_LEN];
    gint                       alarm_type;
};


typedef struct _JpfGetJointParaRes JpfGetJointParaRes;
struct _JpfGetJointParaRes
{
    JpfMsgErrCode       code;
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    guid[MAX_ID_LEN];
    gint                       alarm_type;
    gint                       record_channel;
    gint                       record_second;
    gint                       beep_enable;
    gint                       beep_second;
    gint                       output_channel;
    gint                       output_second;
    gint                       snap_channel;
    gint                       snap_interval;
    gint                       snap_times;        //抓拍次数
    gint                       email_enable;
};

typedef struct _JpfSetJointPara JpfSetJointPara;
struct _JpfSetJointPara
{
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    guid[MAX_ID_LEN];
    gint                       alarm_type;
    gint                       record_channel;
    gint                       record_second;
    gint                       beep_enable;
    gint                       beep_second;
    gint                       output_channel;
    gint                       output_second;
    gint                       snap_channel;
    gint                       snap_interval;
    gint                       snap_times;        //抓拍次数
    gint                       email_enable;
};

typedef struct _JpfPtzPara JpfPtzPara;
struct _JpfPtzPara
{
    gint                       serial_no;
    gint                       ptz_protocol;
    gint                       ptz_address;
    gint                       baud_rate;
    gint	                  data_bit;
    gint                       stop_bit;
    gint	                  verify;
};

typedef struct _JpfGetPtzParaRes JpfGetPtzParaRes;
struct _JpfGetPtzParaRes
{
    JpfMsgErrCode       code;
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    guid[MAX_ID_LEN];
    JpfPtzPara             ptz_para;
};

typedef struct _JpfSetPtzPara JpfSetPtzPara;
struct _JpfSetPtzPara
{
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    guid[MAX_ID_LEN];
    JpfPtzPara             ptz_para;
};

typedef struct _JpfControlPtz JpfControlPtz;
struct _JpfControlPtz
{
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    guid[MAX_ID_LEN];
    gint                       direction;
    gint                       param;
    gint                       rank;
};

typedef struct _JpfPresetInfo JpfPresetInfo;
struct _JpfPresetInfo
{
    gchar                   preset_name[PRESET_NAME_LEN];
    gint                      preset_no;
};

typedef struct _JpfGetPresetPointRes JpfGetPresetPointRes;
struct _JpfGetPresetPointRes
{
    JpfMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      preset_num;
    JpfPresetInfo          preset_info[0];
};

typedef struct _JpfSetPresetPoint JpfSetPresetPoint;
struct _JpfSetPresetPoint
{
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint		preset_action;
    gchar		preset_name[PRESET_NAME_LEN];
    gint		preset_no;
};

typedef struct _Jpf3DControl Jpf3DControl;
struct _Jpf3DControl
{
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint		x_offset;
    gint		y_offset;
    gint		amplify;
};

typedef struct _JpfCruiseInfo JpfCruiseInfo;
struct _JpfCruiseInfo
{
    gchar		cruise_name[CRUISE_NAME_LEN];
    gint		cruise_no;
};

typedef struct _JpfGetCruiseWaySetRes JpfGetCruiseWaySetRes;
struct _JpfGetCruiseWaySetRes
{
    JpfMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      cruise_num;
    JpfCruiseInfo          cruise_info[0];
};

typedef struct _JpfGetCruiseWay JpfGetCruiseWay;
struct _JpfGetCruiseWay
{
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      cruise_no;
};

typedef struct _JpfCruiseWayInfo JpfCruiseWayInfo;
struct _JpfCruiseWayInfo
{
    gint		preset_no;
    gint          speed;
    gint		step;
};

typedef struct _JpfGetCruiseWayRes JpfGetCruiseWayRes;
struct _JpfGetCruiseWayRes
{
    JpfMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gchar		cruise_name[CRUISE_NAME_LEN];
    gint		cruise_no;
    gint                      cruise_num;
    JpfCruiseWayInfo    cruise_way[0];
};

typedef struct _JpfAddCruiseWay JpfAddCruiseWay;
struct _JpfAddCruiseWay
{
    gchar			session[SESSION_ID_LEN];
    gchar			domain_id[DOMAIN_ID_LEN];
    gchar			guid[MAX_ID_LEN];
    gchar			cruise_name[CRUISE_NAME_LEN];
    gint			cruise_num;
    gint                 cruise_no;
    JpfCruiseWayInfo    cruise_way[0];
};

typedef struct _JpfAddCruiseWayRes JpfAddCruiseWayRes;
struct _JpfAddCruiseWayRes
{
    JpfMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      cruise_no;
};

typedef struct _JpfModifyCruiseWay JpfModifyCruiseWay;
struct _JpfModifyCruiseWay
{
    gchar			session[SESSION_ID_LEN];
    gchar			domain_id[DOMAIN_ID_LEN];
    gchar			guid[MAX_ID_LEN];
    gchar			cruise_name[CRUISE_NAME_LEN];
    gint			cruise_no;
    gint			cruise_num;
    JpfCruiseWayInfo	cruise_way[0];
};

typedef struct _JpfSetCruiseWay JpfSetCruiseWay;
struct _JpfSetCruiseWay
{
    gchar			session[SESSION_ID_LEN];
    gchar			domain_id[DOMAIN_ID_LEN];
    gchar			guid[MAX_ID_LEN];
    gint		cruise_action;
    gint		cruise_no;
};

typedef struct _JpfGetDeviceTimeRes JpfGetDeviceTimeRes;
struct _JpfGetDeviceTimeRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      sync_enable;
    gchar                   server_time[TIME_LEN];
    gint                      time_zone;
};

typedef struct _JpfSetDeviceTime JpfSetDeviceTime;
struct _JpfSetDeviceTime
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      sync_enable;
    gchar                   server_time[TIME_LEN];
    gint                      time_zone;
    gint                      set_flag;
};

typedef struct _JpfGetNTPInfoRes JpfGetNTPInfoRes;
struct _JpfGetNTPInfoRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   ntp_server_ip[MAX_IP_LEN];
    gint	                     time_interval;
    gint                      time_zone;
    gint	                     ntp_enable;
    gint                      dst_enable;
};

typedef struct _JpfSetNTPInfo JpfSetNTPInfo;
struct _JpfSetNTPInfo
{
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   ntp_server_ip[MAX_IP_LEN];
    gint	                 time_interval;
    gint                      time_zone;
    gint	                 ntp_enable;
    gint                      dst_enable;
};

typedef struct _JpfGetSerialPara JpfGetSerialPara;
struct _JpfGetSerialPara
{
    gchar		       session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                puid[MAX_ID_LEN];
    gint                   serial_no;
};

typedef struct _JpfGetSerialParaRes JpfGetSerialParaRes;
struct _JpfGetSerialParaRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      serial_no;
    gint                      baud_rate;
    gint	                     data_bit;
    gint                      stop_bit;
    gint	                     verify;
};

typedef struct _JpfSetSerialPara JpfSetSerialPara;
struct _JpfSetSerialPara
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      serial_no;
    gint                      baud_rate;
    gint	                     data_bit;
    gint                      stop_bit;
    gint	                     verify;
};

typedef struct _JpfGetFtpParaRes JpfGetFtpParaRes;
struct _JpfGetFtpParaRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   ftp_server_ip[MAX_IP_LEN];
    gint                      ftp_server_port;
    gchar                   ftp_usr[USER_NAME_LEN];
    gchar                   ftp_pwd[USER_PASSWD_LEN];
    gchar                   ftp_path[PATH_LEN];
};

typedef struct _JpfSetFtpPara JpfSetFtpPara;
struct _JpfSetFtpPara
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   ftp_server_ip[MAX_IP_LEN];
    gint                      ftp_server_port;
    gchar                   ftp_usr[USER_NAME_LEN];
    gchar                   ftp_pwd[USER_PASSWD_LEN];
    gchar                   ftp_path[PATH_LEN];
};

typedef struct _JpfGetSmtpParaRes JpfGetSmtpParaRes;
struct _JpfGetSmtpParaRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   mail_server_ip[MAX_IP_LEN];
    gint                      mail_server_port;
    gchar                   mail_addr[MAX_IP_LEN];
    gchar                   mail_usr[USER_NAME_LEN];
    gchar                   mail_pwd[USER_PASSWD_LEN];
    gchar                   mail_rctp1[MAIL_ADDR_LEN];
    gchar                   mail_rctp2[MAIL_ADDR_LEN];
    gchar                   mail_rctp3[MAIL_ADDR_LEN];
    gint                      ssl_enable;
};

typedef struct _JpfSetSmtpPara JpfSetSmtpPara;
struct _JpfSetSmtpPara
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   mail_server_ip[MAX_IP_LEN];
    gint                      mail_server_port;
    gchar                   mail_addr[MAX_IP_LEN];
    gchar                   mail_usr[USER_NAME_LEN];
    gchar                   mail_pwd[USER_PASSWD_LEN];
    gchar                   mail_rctp1[MAIL_ADDR_LEN];
    gchar                   mail_rctp2[MAIL_ADDR_LEN];
    gchar                   mail_rctp3[MAIL_ADDR_LEN];
    gint                      ssl_enable;
};

typedef struct _JpfSetUpnpPara JpfSetUpnpPara;
struct _JpfSetUpnpPara
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   upnp_server_ip[MAX_IP_LEN];
    gint                      upnp_enable;
    gint                      eth_no;
    gint 		                  model;
    gint	                     ref_time;
    gint                      data_port;
    gint	                     web_port;
    gint                      data_port_result;
    gint	                     web_port_result;
};

typedef struct _JpfGetUpnpParaRes JpfGetUpnpParaRes;
struct _JpfGetUpnpParaRes
{
    JpfMsgErrCode      code;
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   upnp_server_ip[MAX_IP_LEN];
    gint                      upnp_enable;
    gint                      eth_no;
    gint 		          model;
    gint	                 ref_time;
    gint                      data_port;
    gint	                 web_port;
    gint                      data_port_result;
    gint	                 web_port_result;
};

typedef struct _JpfSetTransparentPara JpfSetTransparentPara;
struct _JpfSetTransparentPara
{
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      type;
    gint                      channel;
    gint 		          length;
    gchar                   data[STRING_LEN];
};

typedef struct _JpfGetTransparentPara JpfGetTransparentPara;
struct _JpfGetTransparentPara
{
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      type;
    gint                      channel;
    gint 		          length;
    gchar                   data[STRING_LEN];
};

typedef struct _JpfGetTransparentParaRes JpfGetTransparentParaRes;
struct _JpfGetTransparentParaRes
{
    JpfMsgErrCode      code;
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      type;
    gint                      channel;
    gint 		          length;
    gchar                   data[STRING_LEN];
};

typedef struct _JpfSetDdnsPara JpfSetDdnsPara;
struct _JpfSetDdnsPara
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      open;
    gint                      type;
    gint 		           port;
    gchar	                  times[TIME_LEN];
    gchar                    account[USER_NAME_LEN];;
    gchar	                  username[USER_NAME_LEN];
    gchar                    password[USER_PASSWD_LEN];
};

typedef struct _JpfGetDdnsParaRes JpfGetDdnsParaRes;
struct _JpfGetDdnsParaRes
{
    JpfMsgErrCode      code;
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      open;
    gint                      type;
    gint 		           port;
    gchar	                  times[TIME_LEN];
    gchar                    account[USER_NAME_LEN];;
    gchar	                  username[USER_NAME_LEN];
    gchar                    password[USER_PASSWD_LEN];
};

typedef struct _JpfDiskInfo JpfDiskInfo;
struct _JpfDiskInfo
{
    gint                      disk_no;
    gint                      disk_type;
    gint	                 status;
    gint                      total_size;
    gint	                 free_size;
    gint                      is_backup;
    gint	                 sys_file_type;
};


typedef struct _JpfGetDiskInfoRes JpfGetDiskInfoRes;
struct _JpfGetDiskInfoRes
{
    JpfMsgErrCode      code;
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      disk_num;
    JpfDiskInfo           disk_info[0];
};

typedef struct _JpfGetResolutionInfoRes JpfGetResolutionInfoRes;
struct _JpfGetResolutionInfoRes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      resolution;
};

typedef struct _JpfSetResolutionInfo JpfSetResolutionInfo;
struct _JpfSetResolutionInfo
{

    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      resolution;
};

typedef struct _JpfIrcutControlInfo JpfIrcutControlInfo;
struct _JpfIrcutControlInfo
{
    gint                     channel;
    gint                     switch_mode;
    gint                     auto_c2b;
    gint	                 sensitive;
    gint                     open;
    gint	                 rtc;
    IrcutTimerSwitch   timer_switch;
};

typedef struct _JpfGetIrcutControlInfoRes JpfGetIrcutControlInfoRes;
struct _JpfGetIrcutControlInfoRes
{
    JpfMsgErrCode      code;
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                     channel_count;
    JpfIrcutControlInfo     ircut_control_info[0];
};

typedef struct _JpfSetIrcutControlInfo JpfSetIrcutControlInfo;
struct _JpfSetIrcutControlInfo
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                     channel_count;
    JpfIrcutControlInfo     ircut_control_info[0];
};

typedef struct _JpfFormatDisk JpfFormatDisk;
struct _JpfFormatDisk
{
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    puid[MAX_ID_LEN];
    gint                       disk_no;
};

typedef struct _JpfGetStoreLog JpfGetStoreLog;
struct _JpfGetStoreLog
{
    gchar		       session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                guid[MAX_ID_LEN];
    gint                   store_path;   // 0:shebei 1:mds
    gint                  record_type;
    gchar                begin_time[TIME_LEN];
    gchar                end_time[TIME_LEN];
    gint                   begin_node;
    gint                   end_node;
    gint                   sessId;
};

typedef struct _JpfGetMssStoreLog JpfGetMssStoreLog;
struct _JpfGetMssStoreLog
{
    gchar		       session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                guid[MAX_ID_LEN];
    gchar                mss_id[MSS_ID_LEN];
    gchar                hd_group_id[HD_GROUP_ID_LEN];
    gint                   record_type;
    gchar                begin_time[TIME_LEN];
    gchar                end_time[TIME_LEN];
    gint                   begin_node;
    gint                   end_node;
    gint                   sessId;
};

typedef struct _JpfStoreLog JpfStoreLog;
struct _JpfStoreLog
{
    gint                   record_type;
    gchar                begin_time[TIME_LEN];
    gchar                end_time[TIME_LEN];
    gchar                property[FILE_PROPERTY_LEN];
    gint                   file_size;
};

typedef struct _JpfGetStoreLogRes JpfGetStoreLogRes;
struct _JpfGetStoreLogRes
{
    JpfMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    guid[MAX_ID_LEN];
    gint                       sessId;
    gint                       total_num;
    gint                       req_num;
    JpfStoreLog           store_list[0];
};

typedef struct _JpfPuUpgrade JpfPuUpgrade;
struct _JpfPuUpgrade
{
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    puid[MAX_ID_LEN];
    gchar                    data[FIRMWARE_HEAD_LEN];
    gint                       file_len;
};

typedef struct _JpfPuUpgradeRes JpfPuUpgradeRes;
struct _JpfPuUpgradeRes
{
    JpfMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    puid[MAX_ID_LEN];
    gchar                    ip[MAX_IP_LEN];
    gint                       port;
};

typedef struct _JpfGetAlarm JpfGetAlarm;
struct _JpfGetAlarm
{
    gchar		session[SESSION_ID_LEN];
    gchar		username[USER_NAME_LEN];
    gint            req_num;
    gint            start_num;
    gint            alarm_state;
    gint            alarm_type;
    gchar	        start_time[TIME_INFO_LEN];
    gchar          end_time[TIME_INFO_LEN];
};

typedef struct _JpfAlarm JpfAlarm;
struct _JpfAlarm
{
    gint           alarm_id;
    gchar        domain_id[DOMAIN_ID_LEN];
    gchar        puid[MAX_ID_LEN];
    gchar        guid[MAX_ID_LEN];
    gchar        pu_name[PU_NAME_LEN];
    gchar        gu_name[GU_NAME_LEN];
    gint           alarm_type;
    gint           state;
    gchar	      alarm_time[TIME_INFO_LEN];
    gchar	      alarm_info[ALARM_INFO_LEN];
};

typedef struct _JpfGetAlarmRes JpfGetAlarmRes;
struct _JpfGetAlarmRes
{
    JpfMsgErrCode      code;
    gchar		session[SESSION_ID_LEN];
    gint            req_num;
    gint            total_num;
    JpfAlarm	alarm_list[0];
};

typedef struct _JpfGetAlarmState JpfGetAlarmState;
struct _JpfGetAlarmState
{
    gchar		session[SESSION_ID_LEN];
    gint            alarm_id;;
};

typedef struct _JpfGetAlarmStateRes JpfGetAlarmStateRes;
struct _JpfGetAlarmStateRes
{
    JpfMsgErrCode      code;
    gchar		session[SESSION_ID_LEN];
    gchar		operator[USER_NAME_LEN];
    gchar		deal_time[TIME_INFO_LEN];
    gchar		description[DESCRIPTION_INFO_LEN];
};

typedef struct _JpfDealAlarm JpfDealAlarm;
struct _JpfDealAlarm
{
    gchar		session[SESSION_ID_LEN];
    gint            alarm_id;
    gchar		operator[USER_NAME_LEN];
    gchar		description[DESCRIPTION_INFO_LEN];
};

typedef struct _JpfDealAlarmRes JpfDealAlarmRes;
struct _JpfDealAlarmRes
{
    JpfMsgErrCode      code;
    gchar		session[SESSION_ID_LEN];
};

typedef struct _JpfControlDevice JpfControlDevice;
struct _JpfControlDevice
{
    gchar		       session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                puid[MAX_ID_LEN];
    gint                   command;   //
};

typedef struct _JpfGetGuMss JpfGetGuMss;
struct _JpfGetGuMss
{
    gchar		       session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                guid[MAX_ID_LEN];
};

typedef struct _JpfGuMssHdGroup JpfGuMssHdGroup;
struct _JpfGuMssHdGroup
{
     gchar              hd_group_id[HD_GROUP_ID_LEN];
};

typedef struct _JpfGuMss JpfGuMss;
struct _JpfGuMss
{
    gchar                mss_id[MSS_ID_LEN];
    gchar                mss_name[MSS_NAME_LEN];
    JpfGuMssHdGroup hd_group[HD_GROUP_NUM];
};

typedef struct _JpfGetGuMssRes JpfGetGuMssRes;
struct _JpfGetGuMssRes
{
    JpfMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    guid[MAX_ID_LEN];
    gint                       total_num;
    JpfGuMss               mss_list[0];
};

typedef struct _JpfAuthorizationExpired JpfAuthorizationExpired;
struct _JpfAuthorizationExpired
{
    gint	type;  // 0:距离过期不到5天 1:过期半个小时内 2:已过期
    gchar  expired_time[TIME_LEN];  //到期时间

};

typedef struct _JpfNotifyMessage JpfNotifyMessage;
struct _JpfNotifyMessage
{
    gint	msg_id;  //消息ID
    gchar param1[GENERAL_MSG_PARM_LEN];
    gchar param2[GENERAL_MSG_PARM_LEN];
    gchar param3[GENERAL_MSG_PARM_LEN];
    gchar content[DESCRIPTION_INFO_LEN];  //

};

typedef struct _JpfGetLicenseInfo JpfGetLicenseInfo;
struct _JpfGetLicenseInfo
{
    gchar			session[SESSION_ID_LEN];
};

typedef struct _JpfGetLicenseInfoRes JpfGetLicenseInfoRes;
struct _JpfGetLicenseInfoRes
{
    JpfMsgErrCode      code;
    gchar			session[SESSION_ID_LEN];
    JpfExpiredTime expired_time;
    gint			version;
};

typedef JpfGetLicenseInfo JpfGetTwLicenseInfo;

typedef struct _JpfGetTwLicenseInfoRes JpfGetTwLicenseInfoRes;
struct _JpfGetTwLicenseInfoRes
{
    JpfMsgErrCode      code;
    gint tw_auth_type;
};

typedef struct _JpfCuModifyUserPwd JpfCuModifyUserPwd;
struct _JpfCuModifyUserPwd
{
    gchar		session[SESSION_ID_LEN];
    gchar       username[USER_NAME_LEN];
    gchar       old_password[USER_PASSWD_LEN];
    gchar       new_password[USER_PASSWD_LEN];
};

typedef struct _JpfCuModifyUserPwdRes JpfCuModifyUserPwdRes;
struct _JpfCuModifyUserPwdRes
{
	JpfMsgErrCode      code;
    gchar		session[SESSION_ID_LEN];
    gchar       new_password[USER_PASSWD_LEN];
};


typedef struct _JpfCuQueryGuid JpfCuQueryGuid;
struct _JpfCuQueryGuid
{
	gchar			user[USER_NAME_LEN];
	gint				gu_num;
	gchar			session[SESSION_ID_LEN];
};


typedef struct _JpfCuQueryGuidRes JpfCuQueryGuidRes;
struct _JpfCuQueryGuidRes
{
	JpfMsgErrCode	code;
	gchar			session[SESSION_ID_LEN];
	gchar			guid[MAX_ID_LEN];
	gchar			domain[DOMAIN_ID_LEN];
	gchar			gu_name[GU_NAME_LEN];
	gint				level;
	gint				gu_attributes;
};


typedef struct _JpfCuQueryScreenID JpfCuQueryScreenID;
struct _JpfCuQueryScreenID
{
	gchar			user[USER_NAME_LEN];
	gint				screen_num;
	gchar			session[SESSION_ID_LEN];
};


typedef struct _JpfCuQueryScreenIDRes JpfCuQueryScreenIDRes;
struct _JpfCuQueryScreenIDRes
{
	JpfMsgErrCode	code;
	gchar			session[SESSION_ID_LEN];
	gint				screen_id;
	gint				tw_id;
};


typedef struct _JpfCuQueryUserGuids JpfCuQueryUserGuids;
struct _JpfCuQueryUserGuids
{
	gchar			user[USER_NAME_LEN];
	gint				req_num;
	gint				start_num;
	gchar			session[SESSION_ID_LEN];
};


typedef struct _JpfUserGuidInfo JpfUserGuidInfo;
struct _JpfUserGuidInfo
{
	gchar			guid[MAX_ID_LEN];
	gchar			domain[DOMAIN_ID_LEN];
	gint				level;
	gchar			gu_name[GU_NAME_LEN];
	gint				gu_num;
};

typedef struct _JpfCuQueryUserGuidsRes JpfCuQueryUserGuidsRes;
struct _JpfCuQueryUserGuidsRes
{
	JpfMsgErrCode	code;
	gchar			session[SESSION_ID_LEN];
	gint				total_count;
	gint				back_count;
	JpfUserGuidInfo	guid_info[0];
};


typedef struct _JpfCuSetUserGuids JpfCuSetUserGuids;
struct _JpfCuSetUserGuids
{
	gchar			session[SESSION_ID_LEN];
	gchar			user[USER_NAME_LEN];
	gint				first_req;
	gint				count;
	JpfUserGuidInfo	guid_info[0];
};


typedef struct _JpfCuSetScreenNum JpfCuSetScreenNum;
struct _JpfCuSetScreenNum
{
	gchar			user[USER_NAME_LEN];
	gint				screen_id;
	gint				screen_num;
	gchar			session[SESSION_ID_LEN];
};


typedef struct _JpfCuQueryTourID JpfCuQueryTourID;
struct _JpfCuQueryTourID
{
	gchar			session[SESSION_ID_LEN];
	gchar			user[USER_NAME_LEN];
	gint				tour_num;
};


typedef struct _JpfCuQueryTourIDRes JpfCuQueryTourIDRes;
struct _JpfCuQueryTourIDRes
{
	JpfMsgErrCode	code;
	gchar			session[SESSION_ID_LEN];
	gint				tour_id;
};


typedef struct _JpfCuSetTourNum JpfCuSetTourNum;
struct _JpfCuSetTourNum
{
	gchar			session[SESSION_ID_LEN];
	gchar			user[USER_NAME_LEN];
	gint				tour_id;
	gint				tour_num;
};


typedef struct _JpfCuQueryGroupID JpfCuQueryGroupID;
struct _JpfCuQueryGroupID
{
	gchar			session[SESSION_ID_LEN];
	gchar			user[USER_NAME_LEN];
	gint				group_num;
};


typedef struct _JpfCuQueryGroupIDRes JpfCuQueryGroupIDRes;
struct _JpfCuQueryGroupIDRes
{
	JpfMsgErrCode	code;
	gchar			session[SESSION_ID_LEN];
	gint				group_id;
};


typedef struct _JpfCuSetGroupNum JpfCuSetGroupNum;
struct _JpfCuSetGroupNum
{
	gchar			session[SESSION_ID_LEN];
	gchar			user[USER_NAME_LEN];
	gint				group_id;
	gint				group_num;
};


#endif	//__NMP_MOD_CU_MESSAGES_EXTERNAL_H__
