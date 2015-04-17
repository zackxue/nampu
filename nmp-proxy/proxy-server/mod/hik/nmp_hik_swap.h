#ifndef __HIK_SWAP_H__
#define __HIK_SWAP_H__


#include "HCNetSDK.h"

#include "nmp_sdk.h"

#include "nmp_proxy_server.h"
#include "nmp_hik_sdkinfo.h"


#ifdef __cplusplus
extern "C" {
#endif

void hik_swap_device_info(NET_DVR_DEVICECFG_V40 *device_cfg, 
        JDeviceInfo *device_info, int flag);

void hik_swap_encode_info(NET_DVR_COMPRESSION_INFO_V30 *compress_info, 
        JEncodeParameter *encode_para, int flag);

void hik_swap_display_info(NET_DVR_VIDEOEFFECT *video_effect, 
        JDisplayParameter *display, int flag);

void hik_swap_video_effect(hik_video_effect_t *video_effect, 
        JDisplayParameter *display, int flag);

void hik_swap_osd_info(NET_DVR_PICCFG_V30 *pic_cfg, 
        JOSDParameter *osd_para, int flag);

void hik_swap_record_info(NET_DVR_RECORD_V30 *record_cfg, 
        JRecordParameter *record_para, int flag);

void hik_swap_move_alarm_info(NET_DVR_MOTION_V30 *hik_motion, 
        JMoveAlarm *move_alarm, int flag);

void hik_swap_video_lost_info(NET_DVR_VILOST_V30 *hik_video_lost, 
        JLostAlarm *lost_alarm, int flag);

void hik_swap_hide_alarm_info(NET_DVR_HIDEALARM_V30 *hik_hide_alarm, 
        JHideAlarm *hide_alarm, int flag);

void hik_swap_alarm_in_info(NET_DVR_ALARMINCFG_V30 *hik_alarm_in, 
        JIoAlarm *io_alarm, int flag);

void hik_swap_alarm_out_info(NET_DVR_ALARMOUTCFG_V30 *hik_hide_out, 
        JIoAlarm *io_alarm, int flag);

void hik_swap_disk_list(NET_DVR_HDCFG *hd_cfg, 
        JDeviceDiskInfo *device_disk, int flag);

void hik_swap_network_info(NET_DVR_NETCFG_V30 *net_cfg, 
        JNetworkInfo *net_info, int flag);

void hik_swap_pppoe_info(NET_DVR_PPPOECFG *pppoe_cfg, 
        JPPPOEInfo *pppoe_info, int flag);

void hik_swap_time_info(NET_DVR_TIME *hik_time, 
        JDeviceTime *device_time, int flag);

void hik_swap_zone_info(NET_DVR_ZONEANDDST *hik_zone, 
        JDeviceTime *device_time, int flag);

void hik_swap_ntp_info(NET_DVR_NTPPARA *ntp_para, 
        JDeviceNTPInfo *ntp_info, int flag);

void hik_swap_ftp_info(NET_DVR_FTPCFG *ftp_cfg, 
        JFTPParameter *ftp_param, int flag);

void hik_swap_store_log_info(JStoreLog *store_log_cfg, 
        JStoreLog *store_log_param, int flag);

void hik_swap_hide_info(NET_DVR_PICCFG_V30 *pic_cfg, 
        JHideParameter *hide_info, int flag);

void hik_swap_rs232_info(NET_DVR_SINGLE_RS232 *rs232_cfg, 
        JSerialParameter *serial_info, int flag);

void hik_swap_rs485_info(NET_DVR_ALARM_RS485CFG *rs485_cfg, 
        JSerialParameter *serial_info, int flag);

void hik_swap_smtp_info(NET_DVR_EMAILCFG_V30 *email_cfg, 
        JSMTPParameter *smtp_info, int flag);

void hik_swap_ddns_info(NET_DVR_DDNSPARA_EX *ddns_cfg, 
        JDdnsConfig *ddns_info, int flag);

void hik_swap_ptz_info(NET_DVR_PTZCFG *ptz_cfg, 
        JPTZParameter *ptz_info, int flag);

void hik_swap_cruise_way(NET_DVR_CRUISE_RET *hik_crz, 
        JCruiseWay *crz_way, int flag);

void hik_swap_decoder_info(NET_DVR_DECODERCFG_V30 *dec_cfg, 
        JPTZParameter *ptz_info, int flag);



#ifdef __cplusplus
    }
#endif


#endif  //__HIK_SWAP_H__

