#ifndef __NMP_MOD_CU_MESSAGES_EXTERNAL_H__
#define __NMP_MOD_CU_MESSAGES_EXTERNAL_H__

#include "nmp_msg_share.h"
#include "nmp_list_head.h"
#include "nmp_sysctl.h"
#include "nmp_share_struct.h"
#include "nmp_tw_interface.h"


typedef struct _NmpCuExecuteRes NmpCuExecuteRes;
struct _NmpCuExecuteRes
{
    NmpMsgErrCode      code;
    gchar		session[SESSION_ID_LEN];
};

typedef struct _NmpCuLoginInfo NmpCuLoginInfo;
struct _NmpCuLoginInfo
{
    gchar	username[USER_NAME_LEN];
    gchar	password[USER_PASSWD_LEN];
    gint     cu_version;
};

typedef struct _NmpCuHeart NmpCuHeart;
struct _NmpCuHeart
{
    gchar			session[SESSION_ID_LEN];
};

typedef struct _NmpCuHeartResp NmpCuHeartResp;
struct _NmpCuHeartResp
{
    NmpMsgErrCode	code;
    gchar			server_time[TIME_INFO_LEN];
};


typedef struct _NmpCuLoginResp NmpCuLoginResp;
struct _NmpCuLoginResp
{
    NmpMsgErrCode	code;
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


typedef struct _NmpForceUsrOffline NmpForceUsrOffline;
struct _NmpForceUsrOffline
{
    gint	reason;
};

typedef struct _NmpCuReqArea NmpCuReqArea;
struct _NmpCuReqArea
{
    gchar    username[USER_NAME_LEN];
    gchar    domain_id[DOMAIN_ID_LEN];
};


typedef struct _NmpGetArea NmpGetArea;
struct _NmpGetArea
{
    gchar			session[SESSION_ID_LEN];
    gint            req_num;
    gint            start_num;
    gint            area_id;
};

typedef struct _NmpArea  NmpArea;
struct _NmpArea
{
    gint     area_id;
    gchar  area_name[AREA_NAME_LEN];
    gint     area_parent;
};


typedef struct _NmpGetAreaRes NmpGetAreaRes;
struct _NmpGetAreaRes
{
    NmpMsgErrCode     code;
    gchar			         session[SESSION_ID_LEN];
    gint                     req_num;
    gint                     total_num;
    NmpArea               area_info[0];
};

typedef struct _NmpGetAreaInfo NmpGetAreaInfo;
struct _NmpGetAreaInfo
{
    gchar			session[SESSION_ID_LEN];
    gchar           username[USER_NAME_LEN];
    gint            area_id;
};

typedef struct _NmpGetAreaInfoRes NmpGetAreaInfoRes;
struct _NmpGetAreaInfoRes
{
    NmpMsgErrCode     code;
    gchar			     session[SESSION_ID_LEN];
    gint    gu_count;
    gchar  area_name[AREA_NAME_LEN];
    gchar  user_name[USER_NAME_LEN];
    gchar  user_phone[PHONE_NUM_LEN];
    gchar  user_address[DESCRIPTION_INFO_LEN];
    gchar  description[DESCRIPTION_INFO_LEN];
};

typedef struct _NmpGetDefenceArea NmpGetDefenceArea;
struct _NmpGetDefenceArea
{
    gchar		session[SESSION_ID_LEN];
    gint            req_num;
    gint            start_num;
    gint            area_id;
};

typedef struct _NmpDefenceArea  NmpDefenceArea;
struct _NmpDefenceArea
{
    gint     area_id;
    gchar  area_name[AREA_NAME_LEN];
    gint     is_defence_area; //是否是防区
    gint     defence_enable;
    gchar  policy[POLICY_LEN];
};


typedef struct _NmpGetDefenceAreaRes NmpGetDefenceAreaRes;
struct _NmpGetDefenceAreaRes
{
    NmpMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    NmpDefenceArea    area_info[0];
};

typedef struct _NmpGetDefenceMap NmpGetDefenceMap;
struct _NmpGetDefenceMap
{
    gchar		session[SESSION_ID_LEN];
    gint            req_num;
    gint            start_num;
    gint            defence_area_id;
};

typedef struct _NmpDefenceMap  NmpDefenceMap;
struct _NmpDefenceMap
{
    gint     map_id;
    gchar  map_name[MAP_NAME_LEN];
    gchar  map_location[MAP_LOCATION_LEN];
};


typedef struct _NmpGetDefenceMapRes NmpGetDefenceMapRes;
struct _NmpGetDefenceMapRes
{
    NmpMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    NmpDefenceMap    map_info[0];
};

typedef struct _NmpGetDefenceGu NmpGetDefenceGu;
struct _NmpGetDefenceGu
{
    gchar		session[SESSION_ID_LEN];
    gchar		username[USER_NAME_LEN];
    gint            req_num;
    gint            start_num;
    gint            map_id;
};

typedef struct _NmpDefenceGu  NmpDefenceGu;
struct _NmpDefenceGu
{
    gchar  guid[MAX_ID_LEN];
    gchar  domain_id[DOMAIN_ID_LEN];
    gchar  gu_name[GU_NAME_LEN];
    gint     gu_type;
    gint     pu_online_state;
    double            coordinate_x;
    double            coordinate_y;
};


typedef struct _NmpGetDefenceGuRes NmpGetDefenceGuRes;
struct _NmpGetDefenceGuRes
{
    NmpMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    NmpDefenceGu      defence_gu[0];
};

typedef struct _NmpGetMapHref NmpGetMapHref;
struct _NmpGetMapHref
{
    gchar		session[SESSION_ID_LEN];
    gint            req_num;
    gint            start_num;
    gint            map_id;
};

typedef struct _NmpMapHref NmpMapHref;
struct _NmpMapHref
{
    gint                dst_map_id;
    gchar             map_name[MAP_NAME_LEN];
    gchar             map_location[MAP_LOCATION_LEN];
    double            coordinate_x;
    double            coordinate_y;
};


typedef struct _NmpGetMapHrefRes NmpGetMapHrefRes;
struct _NmpGetMapHrefRes
{
    NmpMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    NmpMapHref          map_href[0];
};

typedef struct _NmpGetGuMapLocation NmpGetGuMapLocation;
struct _NmpGetGuMapLocation
{
    gchar	session[SESSION_ID_LEN];
    gchar  domain_id[DOMAIN_ID_LEN];
    gchar  guid[MAX_ID_LEN];
};

typedef struct _NmpGetGuMapLocationRes NmpGetGuMapLocationRes;
struct _NmpGetGuMapLocationRes
{
    NmpMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                    defence_area_id;
    gint                    map_id;
    gchar                 map_name[MAP_NAME_LEN];
    gchar                 map_location[MAP_LOCATION_LEN];
    double               coordinate_x;
    double               coordinate_y;
};

typedef struct _NmpGetTw NmpGetTw;
struct _NmpGetTw
{
    gchar		session[SESSION_ID_LEN];
    gint            req_num;
    gint            start_num;
    gchar           username[USER_NAME_LEN];
};

typedef struct _NmpCuTwInfo NmpCuTwInfo;
struct _NmpCuTwInfo
{
    gint                tw_id;
    gchar             tw_name[TW_NAME_LEN];
};


typedef struct _NmpGetTwRes NmpGetTwRes;
struct _NmpGetTwRes
{
    NmpMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    NmpCuTwInfo                  tw_info[0];
};

typedef struct _NmpGetScreen NmpGetScreen;
struct _NmpGetScreen
{
	gchar			session[SESSION_ID_LEN];
	gchar			user[USER_NAME_LEN];
	gint				req_num;
	gint				start_num;
	gint				tw_id;
};

typedef struct _NmpScreen NmpScreen;
struct _NmpScreen
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


typedef struct _NmpGetScreenRes NmpGetScreenRes;
struct _NmpGetScreenRes
{
    NmpMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    NmpScreen            screen_info[0];
};

typedef struct _NmpGetScrState NmpGetScrState;
struct _NmpGetScrState
{
    gchar		session[SESSION_ID_LEN];
    gint           tw_id;
    gint           screen_id;
    tw_general_guid  guid;
};

typedef struct _NmpScrStateInfo NmpScrStateInfo;
struct _NmpScrStateInfo
{
    gint                div_num;
    gchar              enc_name[GU_NAME_LEN];
    gint                enc_channel;
    gint                level;
    gint                action_type;
    gint                 action_result;
};

typedef struct _NmpFullScrState NmpFullScrState;
struct _NmpFullScrState
{
    gint                mode;	//全屏模式 0:非全屏，1:全屏
    gchar              enc_name[GU_NAME_LEN];
    gint                enc_channel;
    gint                level;
    gint                action_type;
    gint                action_result;
};


typedef struct _NmpGetScrStateRes NmpGetScrStateRes;
struct _NmpGetScrStateRes
{
    NmpMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                    div_id;
    gint                    scr_lock_state;
    NmpFullScrState     full_scr_state;
    gint                     back_num;
    NmpScrStateInfo      scr_state_info[0];
};

typedef struct _NmpChangeDivMode NmpChangeDivMode;
struct _NmpChangeDivMode
{
    gchar		session[SESSION_ID_LEN];
    gint           tw_id;
    gint           screen_id;
    gint           div_id;
    tw_general_guid  guid;
};

typedef struct _NmpChangeDivModeRes NmpChangeDivModeRes;
struct _NmpChangeDivModeRes
{
    NmpMsgErrCode      code;
    gchar		session[SESSION_ID_LEN];
};

typedef struct _NmpRunStep NmpRunStep;
struct _NmpRunStep
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

typedef struct _NmpRunStepRes NmpRunStepRes;
struct _NmpRunStepRes
{
    NmpMsgErrCode      code;
    gchar		session[SESSION_ID_LEN];
};

typedef struct _NmpFullScreen NmpFullScreen;
struct _NmpFullScreen
{
    gchar		session[SESSION_ID_LEN];
    gint           tw_id;
    gint           screen_id;
    gint           div_id;
    gint           div_num;
    tw_general_guid         guid;
};

typedef struct _NmpExitFullScreen NmpExitFullScreen;
struct _NmpExitFullScreen
{
    gchar		session[SESSION_ID_LEN];
    gint           tw_id;
    gint           screen_id;
    tw_general_guid         guid;
};

typedef struct _NmpGetTour NmpGetTour;
struct _NmpGetTour
{
    gchar		session[SESSION_ID_LEN];
    gint            req_num;
    gint            start_num;
    gchar           username[USER_NAME_LEN];
};

typedef struct _NmpCuTourInfo NmpCuTourInfo;
struct _NmpCuTourInfo
{
    gint                tour_id;
    gchar             tour_name[TOUR_NAME_LEN];
    gint                tour_num;
    gint               auto_jump;
};


typedef struct _NmpGetTourRes NmpGetTourRes;
struct _NmpGetTourRes
{
    NmpMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    NmpCuTourInfo                  tour_info[0];
};

typedef struct _NmpGetTourStep NmpGetTourStep;
struct _NmpGetTourStep
{
    gchar		session[SESSION_ID_LEN];
    gint            req_num;
    gint            start_num;
    gint            tour_id;
};

typedef struct _NmpCuTourStepInfo NmpCuTourStepInfo;
struct _NmpCuTourStepInfo
{
    gint                step_no;
    gint               interval;
    gchar             encoder_domain[DOMAIN_ID_LEN];
    gchar             encoder_guid[MAX_ID_LEN];
};


typedef struct _NmpGetTourStepRes NmpGetTourStepRes;
struct _NmpGetTourStepRes
{
    NmpMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    NmpCuTourStepInfo                  step_info[0];
};

typedef struct _NmpGetGroup NmpGetGroup;
struct _NmpGetGroup
{
	gchar			session[SESSION_ID_LEN];
	gchar			user[USER_NAME_LEN];
	gint				req_num;
	gint				start_num;
};

typedef struct _NmpCuGroupInfo NmpCuGroupInfo;
struct _NmpCuGroupInfo
{
    gint               group_id;
    gchar             group_name[GROUP_NAME_LEN];
    gint               group_num;
    gint               tw_id;
};


typedef struct _NmpGetGroupRes NmpGetGroupRes;
struct _NmpGetGroupRes
{
    NmpMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint                     back_num;
    gint                     total_num;
    NmpCuGroupInfo      group_info[0];
};

////////////

typedef struct _NmpAreaList NmpAreaList;
struct _NmpAreaList
{
    gint       area_id;
    gchar    area_name[AREA_NAME_LEN];
};


typedef struct _NmpCuReqAreaResp NmpCuReqAreaResp;
struct _NmpCuReqAreaResp
{
    gchar 			username[USER_NAME_LEN];
    gchar 			domain_id[DOMAIN_ID_LEN];
    gchar 			domain_name[DOMAIN_ID_LEN];
    gint  			result_code;
    gint 			area_num;
    NmpAreaList 	       area_list[0];
};


typedef struct _NmpCuReqDeviceList NmpCuReqDeviceList;
struct _NmpCuReqDeviceList
{
    gchar         username[USER_NAME_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
    gchar         area_id[AREA_NAME_LEN];
};


typedef struct _NmpGuList NmpGuList;
struct _NmpGuList
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

typedef struct _NmpCuReqDeviceListResp NmpCuReqDeviceListResp;
struct _NmpCuReqDeviceListResp
{
    gchar          username[USER_NAME_LEN];
    gchar          domain_id[DOMAIN_ID_LEN];
    gchar          domain_name[DOMAIN_ID_LEN];
    gint             area_id;
    gchar          area_name[AREA_NAME_LEN];
    gint             result_code;
    gint             gu_num;
    NmpGuList      gu_list[0];
};

//////////////
typedef struct _NmpGetDevice NmpGetDevice;
struct _NmpGetDevice
{
    gchar		  session[SESSION_ID_LEN];
    gchar           username[USER_NAME_LEN];
    gint              area_id;
    gint              req_num;
    gint              start_num;
};

typedef struct _NmpGu NmpGu;
struct _NmpGu
{
    struct list_head list;
    gchar                guid[MAX_ID_LEN];
    gchar                gu_name[GU_NAME_LEN];
    gint                   gu_num;
    gint                   gu_type;
    gint                   gu_attribute;

};

typedef struct _NmpDevice NmpDevice;
struct _NmpDevice
{
    struct list_head list;

    gchar         domain_id[DOMAIN_ID_LEN];
    gchar         puid[MAX_ID_LEN];
    gchar         pu_name[PU_NAME_LEN];
    gint            pu_type;
    gint            pu_state;

    gint           gu_num;
    NmpGu       *gu_list;
};

typedef struct _NmpGetDeviceRes NmpGetDeviceRes;
struct _NmpGetDeviceRes
{
    NmpMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint	                device_count;
    gint	                gu_count;
    NmpDevice *          device_list;
    GStaticMutex        list_lock;
};


typedef struct _NmpGetAreaDevice NmpGetAreaDevice;
struct _NmpGetAreaDevice
{
    gchar		  session[SESSION_ID_LEN];
    gchar           username[USER_NAME_LEN];
    gint              area_id;
    gint              req_num;
    gint              start_num;
};

typedef struct _NmpDeviceInfo NmpDeviceInfo;
struct _NmpDeviceInfo
{
    gchar         domain_id[DOMAIN_ID_LEN];
    gchar         puid[MAX_ID_LEN];
    gchar         pu_name[PU_NAME_LEN];
    gint            pu_type;
    gint            pu_state;
    gchar         pu_ip[MAX_IP_LEN];
    gchar         pu_last_alive_time[TIME_INFO_LEN];
};


typedef struct _NmpGetAreaDeviceRes NmpGetAreaDeviceRes;
struct _NmpGetAreaDeviceRes
{
    NmpMsgErrCode     code;
    gchar			  session[SESSION_ID_LEN];
    gint	                device_count;
    gint                  req_num;
    NmpDeviceInfo       device_list[0];
};

typedef struct  _NmpGetMediaUrl NmpGetMediaUrl ;
struct _NmpGetMediaUrl
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

typedef struct  _NmpGetMediaUrlRes NmpGetMediaUrlRes;
struct   _NmpGetMediaUrlRes
{
    NmpMsgErrCode      code;
    gchar			   session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                guid[MAX_ID_LEN];
    gchar                ip[MAX_IP_LEN];
    gchar                url[MAX_URL_LEN];
    gchar                decode_path[FILE_PATH_LEN];
    gchar                decode_name[DECODE_NAME_LEN];
};

typedef struct _NmpGetPlatformInfo NmpGetPlatformInfo;
struct _NmpGetPlatformInfo
{
    gchar		       session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                puid[MAX_ID_LEN];
};


typedef struct _NmpGetDeviceInfo NmpGetDeviceInfo;
struct _NmpGetDeviceInfo
{
    gchar			   session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
};

typedef struct _NmpGetDeviceInfoErr NmpGetDeviceInfoErr;
struct _NmpGetDeviceInfoErr
{
    NmpMsgErrCode      code;
    gchar			   session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
};

typedef struct _NmpGetDeviceChannelInfo NmpGetDeviceChannelInfo;
struct _NmpGetDeviceChannelInfo
{
    gchar		           session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                guid[MAX_ID_LEN];
};

typedef struct _NmpGetDeviceInfoRes NmpGetDeviceInfoRes;
struct _NmpGetDeviceInfoRes
{
    NmpMsgErrCode     code;
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

typedef struct _NmpGetPlatformInfoRes NmpGetPlatformInfoRes;
struct _NmpGetPlatformInfoRes
{
    NmpMsgErrCode      code;
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

typedef struct _NmpSetPlatformInfo NmpSetPlatformInfo;
struct _NmpSetPlatformInfo
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

typedef struct _NmpNetworkInfo NmpNetworkInfo;
struct _NmpNetworkInfo
{
    gint                      network_type;
    gchar                   ip[MAX_IP_LEN];
    gchar                   netmask[MAX_IP_LEN];
    gchar                   gateway[MAX_IP_LEN];
    gchar                   mac[MAX_IP_LEN];
    gint                      dhcp_enable;
};

typedef struct _NmpGetNetworkInfoRes NmpGetNetworkInfoRes;
struct _NmpGetNetworkInfoRes
{
    NmpMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   main_dns[MAX_IP_LEN];
    gchar                   sub_dns[MAX_IP_LEN];
    gint                      auto_dns_enable;
    gint                      cmd_port;
    gint                      data_port;
    gint                      web_port;
    NmpNetworkInfo 		network[3];
};

typedef struct _NmpSetNetworkInfo NmpSetNetworkInfo;
struct _NmpSetNetworkInfo
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
    NmpNetworkInfo 		network[3];
};


typedef struct _NmpGetPppoeInfoRes NmpGetPppoeInfoRes;
struct _NmpGetPppoeInfoRes
{
    NmpMsgErrCode      code;
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   account[USER_NAME_LEN];
    gchar                   passwd[USER_PASSWD_LEN];
    gint                      interfaces;
    gint                      pppoeEnable;
    gchar                   pppoeIp[MAX_IP_LEN];
};

typedef struct _NmpSetPppoeInfo NmpSetPppoeInfo;
struct _NmpSetPppoeInfo
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

typedef struct _NmpGetEncodeParaRes NmpGetEncodeParaRes;
struct _NmpGetEncodeParaRes
{
    NmpMsgErrCode      code;
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

typedef struct _NmpSetDeviceInfo NmpSetDeviceInfo;
struct _NmpSetDeviceInfo
{
    NmpMsgErrCode       code;
    gchar		               session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                     puid[MAX_ID_LEN];
};

typedef struct _NmpSetDeviceInfoRes NmpSetDeviceInfoRes;
struct _NmpSetDeviceInfoRes
{
    NmpMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                    puid[MAX_ID_LEN];
};

typedef struct _NmpSetDeviceParaRes NmpSetDeviceParaRes;
struct _NmpSetDeviceParaRes
{
    NmpMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
};

typedef struct _NmpSetEncodePara NmpSetEncodePara;
struct _NmpSetEncodePara
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

typedef struct _NmpGetDisplayParaRes NmpGetDisplayParaRes;
struct _NmpGetDisplayParaRes
{
    NmpMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      contrast;
    gint                      bright;
    gint                      hue;
    gint                      saturation;
    gint                      sharpness;
};

typedef struct _NmpSetDisplayPara NmpSetDisplayPara;
struct _NmpSetDisplayPara
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

typedef struct _NmpGetOSDParaRes NmpGetOSDParaRes;
struct _NmpGetOSDParaRes
{
    NmpMsgErrCode      code;
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

typedef struct _NmpSetOSDPara NmpSetOSDPara;
struct _NmpSetOSDPara
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


typedef struct _NmpGetRecordParaRes NmpGetRecordParaRes;
struct _NmpGetRecordParaRes
{
    NmpMsgErrCode      code;
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      level;
    gint                      auto_cover;
    gint                      pre_record;
    gint                      all_day_enable;
    gint                      weekday_num;
    NmpWeekday          weekdays[WEEKDAYS];
};

typedef struct _NmpSetRecordPara  NmpSetRecordPara;
struct _NmpSetRecordPara
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      level;
    gint                      auto_cover;
    gint                      pre_record;
    gint                      all_day_enable;
    gint                      weekday_num;
    NmpWeekday            weekdays[WEEKDAYS];
};


typedef struct _NmpRectangle NmpRectangle;
struct _NmpRectangle
{
    gint left;
    gint top;
    gint right;
    gint bottom;
};

typedef struct _NmpGetMoveAlarmParaRes NmpGetMoveAlarmParaRes;
struct _NmpGetMoveAlarmParaRes
{
    NmpMsgErrCode     code;
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
    NmpRectangle        detect_area[DETECT_AREA_NUM]; 						//侦测区域
    NmpWeekday         weekdays[WEEKDAYS]; 						//移动侦测时间段：结束时间
};

typedef struct _NmpSetMoveAlarmPara NmpSetMoveAlarmPara;
struct _NmpSetMoveAlarmPara
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
    NmpRectangle        detect_area[DETECT_AREA_NUM]; 						//侦测区域
    NmpWeekday         weekdays[WEEKDAYS]; 					//移动侦测时间段：结束时间
};

typedef struct _NmpGetVideoLostParaRes NmpGetVideoLostParaRes;
struct _NmpGetVideoLostParaRes
{
    NmpMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                     lost_enable; 					//video lost是否生效
    gint                     detect_interval; 					//侦测间隔时间
    gint                      weekday_num;
    NmpWeekday         weekdays[WEEKDAYS]; 						//移动侦测时间段：结束时间
};

typedef struct _NmpSetVideoLostPara NmpSetVideoLostPara;
struct _NmpSetVideoLostPara
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      lost_enable; 					//video lost是否生效
    gint                      detect_interval; 					//侦测间隔时间
    gint                      weekday_num;
    NmpWeekday         weekdays[WEEKDAYS]; 					//移动侦测时间段：结束时间
};

typedef struct _NmpGetHideParaRes NmpGetHideParaRes;
struct _NmpGetHideParaRes
{
    NmpMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      hide_enable;
    gint                      hide_color;
    gint                      max_width;
    gint                      max_height;
    gint                      detect_num;
    NmpRectangle         detect_area[DETECT_AREA_NUM];
};

typedef struct _NmpSetHidePara NmpSetHidePara;
struct _NmpSetHidePara
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      hide_enable;
    gint                      hide_color;
    gint                      max_width;
    gint                      max_height;
    gint                      detect_num;
    NmpRectangle         detect_area[DETECT_AREA_NUM];
};

typedef struct _NmpGetHideAlarmParaRes NmpGetHideAlarmParaRes;
struct _NmpGetHideAlarmParaRes
{
    NmpMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      hide_enable; 					//video lost是否生效
    gint                      detect_interval; 					//侦测间隔时间
    gint                      max_width;
    gint                      max_height;
    gint                      weekday_num;
    gint                      detect_num;
    NmpRectangle         detect_area[DETECT_AREA_NUM];
    NmpWeekday          weekdays[WEEKDAYS]; 						//移动侦测时间段：结束时间
};

typedef struct _NmpSetHideAlarmPara NmpSetHideAlarmPara;
struct _NmpSetHideAlarmPara
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
    NmpRectangle         detect_area[DETECT_AREA_NUM];
    NmpWeekday          weekdays[WEEKDAYS]; 					//移动侦测时间段：结束时间
};

typedef struct _NmpGetIOAlarmParaRes NmpGetIOAlarmParaRes;
struct _NmpGetIOAlarmParaRes
{
    NmpMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      io_enable; 					//video lost是否生效
    gint                      io_type;
    gint                      detect_interval; 					//侦测间隔时间
    gint                      weekday_num;
    NmpWeekday          weekdays[WEEKDAYS]; 						//移动侦测时间段：结束时间
};

typedef struct _NmpSetIOAlarmPara NmpSetIOAlarmPara;
struct _NmpSetIOAlarmPara
{
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      io_enable; 					//video lost是否生效
    gint                      io_type;
    gint                      detect_interval; 					//侦测间隔时间
    gint                      weekday_num;
    NmpWeekday          weekdays[WEEKDAYS]; 					//移动侦测时间段：结束时间
};


typedef struct _NmpGetJointPara NmpGetJointPara;
struct _NmpGetJointPara
{
    NmpMsgErrCode       code;
    gchar		              session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    guid[MAX_ID_LEN];
    gint                       alarm_type;
};


typedef struct _NmpGetJointParaRes NmpGetJointParaRes;
struct _NmpGetJointParaRes
{
    NmpMsgErrCode       code;
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

typedef struct _NmpSetJointPara NmpSetJointPara;
struct _NmpSetJointPara
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

typedef struct _NmpPtzPara NmpPtzPara;
struct _NmpPtzPara
{
    gint                       serial_no;
    gint                       ptz_protocol;
    gint                       ptz_address;
    gint                       baud_rate;
    gint	                  data_bit;
    gint                       stop_bit;
    gint	                  verify;
};

typedef struct _NmpGetPtzParaRes NmpGetPtzParaRes;
struct _NmpGetPtzParaRes
{
    NmpMsgErrCode       code;
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    guid[MAX_ID_LEN];
    NmpPtzPara             ptz_para;
};

typedef struct _NmpSetPtzPara NmpSetPtzPara;
struct _NmpSetPtzPara
{
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    guid[MAX_ID_LEN];
    NmpPtzPara             ptz_para;
};

typedef struct _NmpControlPtz NmpControlPtz;
struct _NmpControlPtz
{
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    guid[MAX_ID_LEN];
    gint                       direction;
    gint                       param;
    gint                       rank;
};

typedef struct _NmpPresetInfo NmpPresetInfo;
struct _NmpPresetInfo
{
    gchar                   preset_name[PRESET_NAME_LEN];
    gint                      preset_no;
};

typedef struct _NmpGetPresetPointRes NmpGetPresetPointRes;
struct _NmpGetPresetPointRes
{
    NmpMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      preset_num;
    NmpPresetInfo          preset_info[0];
};

typedef struct _NmpSetPresetPoint NmpSetPresetPoint;
struct _NmpSetPresetPoint
{
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint		preset_action;
    gchar		preset_name[PRESET_NAME_LEN];
    gint		preset_no;
};

typedef struct _Nmp3DControl Nmp3DControl;
struct _Nmp3DControl
{
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint		x_offset;
    gint		y_offset;
    gint		amplify;
};

typedef struct _NmpCruiseInfo NmpCruiseInfo;
struct _NmpCruiseInfo
{
    gchar		cruise_name[CRUISE_NAME_LEN];
    gint		cruise_no;
};

typedef struct _NmpGetCruiseWaySetRes NmpGetCruiseWaySetRes;
struct _NmpGetCruiseWaySetRes
{
    NmpMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      cruise_num;
    NmpCruiseInfo          cruise_info[0];
};

typedef struct _NmpGetCruiseWay NmpGetCruiseWay;
struct _NmpGetCruiseWay
{
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      cruise_no;
};

typedef struct _NmpCruiseWayInfo NmpCruiseWayInfo;
struct _NmpCruiseWayInfo
{
    gint		preset_no;
    gint          speed;
    gint		step;
};

typedef struct _NmpGetCruiseWayRes NmpGetCruiseWayRes;
struct _NmpGetCruiseWayRes
{
    NmpMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gchar		cruise_name[CRUISE_NAME_LEN];
    gint		cruise_no;
    gint                      cruise_num;
    NmpCruiseWayInfo    cruise_way[0];
};

typedef struct _NmpAddCruiseWay NmpAddCruiseWay;
struct _NmpAddCruiseWay
{
    gchar			session[SESSION_ID_LEN];
    gchar			domain_id[DOMAIN_ID_LEN];
    gchar			guid[MAX_ID_LEN];
    gchar			cruise_name[CRUISE_NAME_LEN];
    gint			cruise_num;
    gint                 cruise_no;
    NmpCruiseWayInfo    cruise_way[0];
};

typedef struct _NmpAddCruiseWayRes NmpAddCruiseWayRes;
struct _NmpAddCruiseWayRes
{
    NmpMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      cruise_no;
};

typedef struct _NmpModifyCruiseWay NmpModifyCruiseWay;
struct _NmpModifyCruiseWay
{
    gchar			session[SESSION_ID_LEN];
    gchar			domain_id[DOMAIN_ID_LEN];
    gchar			guid[MAX_ID_LEN];
    gchar			cruise_name[CRUISE_NAME_LEN];
    gint			cruise_no;
    gint			cruise_num;
    NmpCruiseWayInfo	cruise_way[0];
};

typedef struct _NmpSetCruiseWay NmpSetCruiseWay;
struct _NmpSetCruiseWay
{
    gchar			session[SESSION_ID_LEN];
    gchar			domain_id[DOMAIN_ID_LEN];
    gchar			guid[MAX_ID_LEN];
    gint		cruise_action;
    gint		cruise_no;
};

typedef struct _NmpGetDeviceTimeRes NmpGetDeviceTimeRes;
struct _NmpGetDeviceTimeRes
{
    NmpMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      sync_enable;
    gchar                   server_time[TIME_LEN];
    gint                      time_zone;
};

typedef struct _NmpSetDeviceTime NmpSetDeviceTime;
struct _NmpSetDeviceTime
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      sync_enable;
    gchar                   server_time[TIME_LEN];
    gint                      time_zone;
    gint                      set_flag;
};

typedef struct _NmpGetNTPInfoRes NmpGetNTPInfoRes;
struct _NmpGetNTPInfoRes
{
    NmpMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   ntp_server_ip[MAX_IP_LEN];
    gint	                     time_interval;
    gint                      time_zone;
    gint	                     ntp_enable;
    gint                      dst_enable;
};

typedef struct _NmpSetNTPInfo NmpSetNTPInfo;
struct _NmpSetNTPInfo
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

typedef struct _NmpGetSerialPara NmpGetSerialPara;
struct _NmpGetSerialPara
{
    gchar		       session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                puid[MAX_ID_LEN];
    gint                   serial_no;
};

typedef struct _NmpGetSerialParaRes NmpGetSerialParaRes;
struct _NmpGetSerialParaRes
{
    NmpMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      serial_no;
    gint                      baud_rate;
    gint	                     data_bit;
    gint                      stop_bit;
    gint	                     verify;
};

typedef struct _NmpSetSerialPara NmpSetSerialPara;
struct _NmpSetSerialPara
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

typedef struct _NmpGetFtpParaRes NmpGetFtpParaRes;
struct _NmpGetFtpParaRes
{
    NmpMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gchar                   ftp_server_ip[MAX_IP_LEN];
    gint                      ftp_server_port;
    gchar                   ftp_usr[USER_NAME_LEN];
    gchar                   ftp_pwd[USER_PASSWD_LEN];
    gchar                   ftp_path[PATH_LEN];
};

typedef struct _NmpSetFtpPara NmpSetFtpPara;
struct _NmpSetFtpPara
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

typedef struct _NmpGetSmtpParaRes NmpGetSmtpParaRes;
struct _NmpGetSmtpParaRes
{
    NmpMsgErrCode      code;
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

typedef struct _NmpSetSmtpPara NmpSetSmtpPara;
struct _NmpSetSmtpPara
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

typedef struct _NmpSetUpnpPara NmpSetUpnpPara;
struct _NmpSetUpnpPara
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

typedef struct _NmpGetUpnpParaRes NmpGetUpnpParaRes;
struct _NmpGetUpnpParaRes
{
    NmpMsgErrCode      code;
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

typedef struct _NmpSetTransparentPara NmpSetTransparentPara;
struct _NmpSetTransparentPara
{
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      type;
    gint                      channel;
    gint 		          length;
    gchar                   data[STRING_LEN];
};

typedef struct _NmpGetTransparentPara NmpGetTransparentPara;
struct _NmpGetTransparentPara
{
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      type;
    gint                      channel;
    gint 		          length;
    gchar                   data[STRING_LEN];
};

typedef struct _NmpGetTransparentParaRes NmpGetTransparentParaRes;
struct _NmpGetTransparentParaRes
{
    NmpMsgErrCode      code;
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      type;
    gint                      channel;
    gint 		          length;
    gchar                   data[STRING_LEN];
};

typedef struct _NmpSetDdnsPara NmpSetDdnsPara;
struct _NmpSetDdnsPara
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

typedef struct _NmpGetDdnsParaRes NmpGetDdnsParaRes;
struct _NmpGetDdnsParaRes
{
    NmpMsgErrCode      code;
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

typedef struct _NmpDiskInfo NmpDiskInfo;
struct _NmpDiskInfo
{
    gint                      disk_no;
    gint                      disk_type;
    gint	                 status;
    gint                      total_size;
    gint	                 free_size;
    gint                      is_backup;
    gint	                 sys_file_type;
};


typedef struct _NmpGetDiskInfoRes NmpGetDiskInfoRes;
struct _NmpGetDiskInfoRes
{
    NmpMsgErrCode      code;
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   puid[MAX_ID_LEN];
    gint                      disk_num;
    NmpDiskInfo           disk_info[0];
};

typedef struct _NmpGetResolutionInfoRes NmpGetResolutionInfoRes;
struct _NmpGetResolutionInfoRes
{
    NmpMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      resolution;
};

typedef struct _NmpSetResolutionInfo NmpSetResolutionInfo;
struct _NmpSetResolutionInfo
{

    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                      resolution;
};

typedef struct _NmpIrcutControlInfo NmpIrcutControlInfo;
struct _NmpIrcutControlInfo
{
    gint                     channel;
    gint                     switch_mode;
    gint                     auto_c2b;
    gint	                 sensitive;
    gint                     open;
    gint	                 rtc;
    IrcutTimerSwitch   timer_switch;
};

typedef struct _NmpGetIrcutControlInfoRes NmpGetIrcutControlInfoRes;
struct _NmpGetIrcutControlInfoRes
{
    NmpMsgErrCode      code;
    gchar		          session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                     channel_count;
    NmpIrcutControlInfo     ircut_control_info[0];
};

typedef struct _NmpSetIrcutControlInfo NmpSetIrcutControlInfo;
struct _NmpSetIrcutControlInfo
{
    gchar		             session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
    gint                     channel_count;
    NmpIrcutControlInfo     ircut_control_info[0];
};

typedef struct _NmpFormatDisk NmpFormatDisk;
struct _NmpFormatDisk
{
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    puid[MAX_ID_LEN];
    gint                       disk_no;
};

typedef struct _NmpGetStoreLog NmpGetStoreLog;
struct _NmpGetStoreLog
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

typedef struct _NmpGetMssStoreLog NmpGetMssStoreLog;
struct _NmpGetMssStoreLog
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

typedef struct _NmpStoreLog NmpStoreLog;
struct _NmpStoreLog
{
    gint                   record_type;
    gchar                begin_time[TIME_LEN];
    gchar                end_time[TIME_LEN];
    gchar                property[FILE_PROPERTY_LEN];
    gint                   file_size;
};

typedef struct _NmpGetStoreLogRes NmpGetStoreLogRes;
struct _NmpGetStoreLogRes
{
    NmpMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    guid[MAX_ID_LEN];
    gint                       sessId;
    gint                       total_num;
    gint                       req_num;
    NmpStoreLog           store_list[0];
};

typedef struct _NmpPuUpgrade NmpPuUpgrade;
struct _NmpPuUpgrade
{
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    puid[MAX_ID_LEN];
    gchar                    data[FIRMWARE_HEAD_LEN];
    gint                       file_len;
};

typedef struct _NmpPuUpgradeRes NmpPuUpgradeRes;
struct _NmpPuUpgradeRes
{
    NmpMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    puid[MAX_ID_LEN];
    gchar                    ip[MAX_IP_LEN];
    gint                       port;
};

typedef struct _NmpGetAlarm NmpGetAlarm;
struct _NmpGetAlarm
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

typedef struct _NmpAlarm NmpAlarm;
struct _NmpAlarm
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

typedef struct _NmpGetAlarmRes NmpGetAlarmRes;
struct _NmpGetAlarmRes
{
    NmpMsgErrCode      code;
    gchar		session[SESSION_ID_LEN];
    gint            req_num;
    gint            total_num;
    NmpAlarm	alarm_list[0];
};

typedef struct _NmpGetAlarmState NmpGetAlarmState;
struct _NmpGetAlarmState
{
    gchar		session[SESSION_ID_LEN];
    gint            alarm_id;;
};

typedef struct _NmpGetAlarmStateRes NmpGetAlarmStateRes;
struct _NmpGetAlarmStateRes
{
    NmpMsgErrCode      code;
    gchar		session[SESSION_ID_LEN];
    gchar		operator[USER_NAME_LEN];
    gchar		deal_time[TIME_INFO_LEN];
    gchar		description[DESCRIPTION_INFO_LEN];
};

typedef struct _NmpDealAlarm NmpDealAlarm;
struct _NmpDealAlarm
{
    gchar		session[SESSION_ID_LEN];
    gint            alarm_id;
    gchar		operator[USER_NAME_LEN];
    gchar		description[DESCRIPTION_INFO_LEN];
};

typedef struct _NmpDealAlarmRes NmpDealAlarmRes;
struct _NmpDealAlarmRes
{
    NmpMsgErrCode      code;
    gchar		session[SESSION_ID_LEN];
};

typedef struct _NmpControlDevice NmpControlDevice;
struct _NmpControlDevice
{
    gchar		       session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                puid[MAX_ID_LEN];
    gint                   command;   //
};

typedef struct _NmpGetGuMss NmpGetGuMss;
struct _NmpGetGuMss
{
    gchar		       session[SESSION_ID_LEN];
    gchar                domain_id[DOMAIN_ID_LEN];
    gchar                guid[MAX_ID_LEN];
};

typedef struct _NmpGuMssHdGroup NmpGuMssHdGroup;
struct _NmpGuMssHdGroup
{
     gchar              hd_group_id[HD_GROUP_ID_LEN];
};

typedef struct _NmpGuMss NmpGuMss;
struct _NmpGuMss
{
    gchar                mss_id[MSS_ID_LEN];
    gchar                mss_name[MSS_NAME_LEN];
    NmpGuMssHdGroup hd_group[HD_GROUP_NUM];
};

typedef struct _NmpGetGuMssRes NmpGetGuMssRes;
struct _NmpGetGuMssRes
{
    NmpMsgErrCode      code;
    gchar		           session[SESSION_ID_LEN];
    gchar                    domain_id[DOMAIN_ID_LEN];
    gchar                    guid[MAX_ID_LEN];
    gint                       total_num;
    NmpGuMss               mss_list[0];
};

typedef struct _NmpAuthorizationExpired NmpAuthorizationExpired;
struct _NmpAuthorizationExpired
{
    gint	type;  // 0:距离过期不到5天 1:过期半个小时内 2:已过期
    gchar  expired_time[TIME_LEN];  //到期时间

};

typedef struct _NmpNotifyMessage NmpNotifyMessage;
struct _NmpNotifyMessage
{
    gint	msg_id;  //消息ID
    gchar param1[GENERAL_MSG_PARM_LEN];
    gchar param2[GENERAL_MSG_PARM_LEN];
    gchar param3[GENERAL_MSG_PARM_LEN];
    gchar content[DESCRIPTION_INFO_LEN];  //

};

typedef struct _NmpGetLicenseInfo NmpGetLicenseInfo;
struct _NmpGetLicenseInfo
{
    gchar			session[SESSION_ID_LEN];
};

typedef struct _NmpGetLicenseInfoRes NmpGetLicenseInfoRes;
struct _NmpGetLicenseInfoRes
{
    NmpMsgErrCode      code;
    gchar			session[SESSION_ID_LEN];
    NmpExpiredTime expired_time;
    gint			version;
};

typedef NmpGetLicenseInfo NmpGetTwLicenseInfo;

typedef struct _NmpGetTwLicenseInfoRes NmpGetTwLicenseInfoRes;
struct _NmpGetTwLicenseInfoRes
{
    NmpMsgErrCode      code;
    gint tw_auth_type;
};

typedef struct _NmpCuModifyUserPwd NmpCuModifyUserPwd;
struct _NmpCuModifyUserPwd
{
    gchar		session[SESSION_ID_LEN];
    gchar       username[USER_NAME_LEN];
    gchar       old_password[USER_PASSWD_LEN];
    gchar       new_password[USER_PASSWD_LEN];
};

typedef struct _NmpCuModifyUserPwdRes NmpCuModifyUserPwdRes;
struct _NmpCuModifyUserPwdRes
{
	NmpMsgErrCode      code;
    gchar		session[SESSION_ID_LEN];
    gchar       new_password[USER_PASSWD_LEN];
};


typedef struct _NmpCuQueryGuid NmpCuQueryGuid;
struct _NmpCuQueryGuid
{
	gchar			user[USER_NAME_LEN];
	gint				gu_num;
	gchar			session[SESSION_ID_LEN];
};


typedef struct _NmpCuQueryGuidRes NmpCuQueryGuidRes;
struct _NmpCuQueryGuidRes
{
	NmpMsgErrCode	code;
	gchar			session[SESSION_ID_LEN];
	gchar			guid[MAX_ID_LEN];
	gchar			domain[DOMAIN_ID_LEN];
	gchar			gu_name[GU_NAME_LEN];
	gint				level;
	gint				gu_attributes;
};


typedef struct _NmpCuQueryScreenID NmpCuQueryScreenID;
struct _NmpCuQueryScreenID
{
	gchar			user[USER_NAME_LEN];
	gint				screen_num;
	gchar			session[SESSION_ID_LEN];
};


typedef struct _NmpCuQueryScreenIDRes NmpCuQueryScreenIDRes;
struct _NmpCuQueryScreenIDRes
{
	NmpMsgErrCode	code;
	gchar			session[SESSION_ID_LEN];
	gint				screen_id;
	gint				tw_id;
};


typedef struct _NmpCuQueryUserGuids NmpCuQueryUserGuids;
struct _NmpCuQueryUserGuids
{
	gchar			user[USER_NAME_LEN];
	gint				req_num;
	gint				start_num;
	gchar			session[SESSION_ID_LEN];
};


typedef struct _NmpUserGuidInfo NmpUserGuidInfo;
struct _NmpUserGuidInfo
{
	gchar			guid[MAX_ID_LEN];
	gchar			domain[DOMAIN_ID_LEN];
	gint				level;
	gchar			gu_name[GU_NAME_LEN];
	gint				gu_num;
};

typedef struct _NmpCuQueryUserGuidsRes NmpCuQueryUserGuidsRes;
struct _NmpCuQueryUserGuidsRes
{
	NmpMsgErrCode	code;
	gchar			session[SESSION_ID_LEN];
	gint				total_count;
	gint				back_count;
	NmpUserGuidInfo	guid_info[0];
};


typedef struct _NmpCuSetUserGuids NmpCuSetUserGuids;
struct _NmpCuSetUserGuids
{
	gchar			session[SESSION_ID_LEN];
	gchar			user[USER_NAME_LEN];
	gint				first_req;
	gint				count;
	NmpUserGuidInfo	guid_info[0];
};


typedef struct _NmpCuSetScreenNum NmpCuSetScreenNum;
struct _NmpCuSetScreenNum
{
	gchar			user[USER_NAME_LEN];
	gint				screen_id;
	gint				screen_num;
	gchar			session[SESSION_ID_LEN];
};


typedef struct _NmpCuQueryTourID NmpCuQueryTourID;
struct _NmpCuQueryTourID
{
	gchar			session[SESSION_ID_LEN];
	gchar			user[USER_NAME_LEN];
	gint				tour_num;
};


typedef struct _NmpCuQueryTourIDRes NmpCuQueryTourIDRes;
struct _NmpCuQueryTourIDRes
{
	NmpMsgErrCode	code;
	gchar			session[SESSION_ID_LEN];
	gint				tour_id;
};


typedef struct _NmpCuSetTourNum NmpCuSetTourNum;
struct _NmpCuSetTourNum
{
	gchar			session[SESSION_ID_LEN];
	gchar			user[USER_NAME_LEN];
	gint				tour_id;
	gint				tour_num;
};


typedef struct _NmpCuQueryGroupID NmpCuQueryGroupID;
struct _NmpCuQueryGroupID
{
	gchar			session[SESSION_ID_LEN];
	gchar			user[USER_NAME_LEN];
	gint				group_num;
};


typedef struct _NmpCuQueryGroupIDRes NmpCuQueryGroupIDRes;
struct _NmpCuQueryGroupIDRes
{
	NmpMsgErrCode	code;
	gchar			session[SESSION_ID_LEN];
	gint				group_id;
};


typedef struct _NmpCuSetGroupNum NmpCuSetGroupNum;
struct _NmpCuSetGroupNum
{
	gchar			session[SESSION_ID_LEN];
	gchar			user[USER_NAME_LEN];
	gint				group_id;
	gint				group_num;
};


#endif	//__NMP_MOD_CU_MESSAGES_EXTERNAL_H__
