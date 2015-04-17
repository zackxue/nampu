
#ifndef __PROTO_MEDIUM_H__
#define __PROTO_MEDIUM_H__

#include "nmp_xmlmsg.h"
#include "nmp_packet.h"
#include "nmp_msg_impl.h"

typedef union __JSDKType JSDKType;

union __JSDKType
{
    JDeviceInfo dev_info;
    JDeviceNTPInfo ntp_info;
    JDeviceTime dev_time;
    JPlatformInfo pltf_info;
    JNetworkInfo net_info;
    JPPPOEInfo pppoe_info;
    JEncodeParameter enc_info;
    JDisplayParameter dis_info;
    JRecordParameter rec_info;
    JHideParameter hide_info;
    JSerialParameter ser_info;
    JOSDParameter osd_info;
    JPTZParameter ptz_info;
    JFTPParameter ftp_info;
    JSMTPParameter smtp_info;
    JUPNPParameter upnp_info;
    JDeviceDiskInfo disk_info;
    JFormatDisk fmt_info;
    JMoveAlarm move_alarm;
    JLostAlarm lost_alarm;
    JHideAlarm hide_alarm;
    JIoAlarm io_alarm;
    JJointAction joint_info;
    JPTZControl ptz_ctl;
    JSubmitAlarm smt_alarm;
    JMediaUrl medial_url;
    JStoreLog store_log;
    JUserInfo user_info;
    JUserHeart user_heart;
    JFirmwareUpgrade frm_upgrade;
    JUpgradeProgress upgrade_prg;
    JDevCap dev_cap;
    JChannelInfo chn_info;
    JPictureInfo pit_info;
    JWifiConfig wifi_info;
    JWifiApInfo ap_info;
    JWifiSearchRes wifi_search;
    JNetworkStatus net_status;
    JControlDevice ctrl_dev;
    JSubmitDeviceStatus smt_dev_status;
    JUserConfig user_cfg;
    JRegionConfig region_cfg;
    JDdnsConfig ddns_cfg;
    JHxhtPfConfig hxht_cfg;
    JUserModConfig mod_user;
};


#ifdef __cplusplus
extern "C" {
#endif

int get_device_info(void *init_data, msg_t *msg, int parm_id);
int get_platform_info(void *init_data, msg_t *msg, int parm_id);
int set_platform_info(void *init_data, msg_t *msg, int parm_id);
int get_serial_info(void *init_data, msg_t *msg, int parm_id);
int set_serial_info(void *init_data, msg_t *msg, int parm_id);
int get_device_time(void *init_data, msg_t *msg, int parm_id);
int set_device_time(void *init_data, msg_t *msg, int parm_id);
int get_ntp_info(void *init_data, msg_t *msg, int parm_id);
int set_ntp_info(void *init_data, msg_t *msg, int parm_id);
int get_network_info(void *init_data, msg_t *msg, int parm_id);
int set_network_info(void *init_data, msg_t *msg, int parm_id);
int get_pppoe_info(void *init_data, msg_t *msg, int parm_id);
int set_pppoe_info(void *init_data, msg_t *msg, int parm_id);
int get_ftp_info(void *init_data, msg_t *msg, int parm_id);
int set_ftp_info(void *init_data, msg_t *msg, int parm_id);
int get_smtp_info(void *init_data, msg_t *msg, int parm_id);
int set_smtp_info(void *init_data, msg_t *msg, int parm_id);
int get_ddns_info(void *init_data, msg_t *msg, int parm_id);
int set_ddns_info(void *init_data, msg_t *msg, int parm_id);
int get_upnp_info(void *init_data, msg_t *msg, int parm_id);
int set_upnp_info(void *init_data, msg_t *msg, int parm_id);

int get_disk_list(void *init_data, msg_t *msg, int parm_id);
int format_disk(void *init_data, msg_t *msg, int parm_id);
int control_device(void *init_data, msg_t *msg, int parm_id);

int get_encode_info(void *init_data, msg_t *msg, int parm_id);
int set_encode_info(void *init_data, msg_t *msg, int parm_id);
int get_display_info(void *init_data, msg_t *msg, int parm_id);
int set_display_info(void *init_data, msg_t *msg, int parm_id);
int get_osd_info(void *init_data, msg_t *msg, int parm_id);
int set_osd_info(void *init_data, msg_t *msg, int parm_id);
int get_ptz_info(void *init_data, msg_t *msg, int parm_id);
int set_ptz_info(void *init_data, msg_t *msg, int parm_id);
int get_record_info(void *init_data, msg_t *msg, int parm_id);
int set_record_info(void *init_data, msg_t *msg, int parm_id);
int get_hide_info(void *init_data, msg_t *msg, int parm_id);
int set_hide_info(void *init_data, msg_t *msg, int parm_id);

int get_move_alarm_info(void *init_data, msg_t *msg, int parm_id);
int set_move_alarm_info(void *init_data, msg_t *msg, int parm_id);
int get_video_lost_info(void *init_data, msg_t *msg, int parm_id);
int set_video_lost_info(void *init_data, msg_t *msg, int parm_id);
int get_hide_alarm_info(void *init_data, msg_t *msg, int parm_id);
int set_hide_alarm_info(void *init_data, msg_t *msg, int parm_id);
int get_io_alarm_info(void *init_data, msg_t *msg, int parm_id);
int set_io_alarm_info(void *init_data, msg_t *msg, int parm_id);

int get_media_url(void *init_data, msg_t *msg, int parm_id);
int get_store_log(void *init_data, msg_t *msg, int parm_id);

int ptz_control(void *init_data, msg_t *msg, int parm_id);

int get_preset_set(void *init_data, msg_t *msg, int parm_id);
int set_preset_point(void *init_data, msg_t *msg, int parm_id);
int get_cruise_set(void *init_data, msg_t *msg, int parm_id);
int get_cruise_way(void *init_data, msg_t *msg, int parm_id);
int set_cruise_way(void *init_data, msg_t *msg, int parm_id);
int add_cruise_way(void *init_data, msg_t *msg, int parm_id);
int modify_cruise_way(void *init_data, msg_t *msg, int parm_id);

int transparent_get_param(void *init_data, msg_t *msg, int parm_id);
int transparent_set_param(void *init_data, msg_t *msg, int parm_id);


#ifdef __cplusplus
    }
#endif

#endif  //__PROTO_MEDIUM_H__

