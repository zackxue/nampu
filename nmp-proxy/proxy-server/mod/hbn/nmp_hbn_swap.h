#ifndef __HBN_SWAP_H__
#define __HBN_SWAP_H__

#include "HBNetSDK.h"

#include "nmp_sdk.h"
#include "nmp_hbn_sdkinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

void hbn_swap_device_info(HB_NET_DEVICECFG *dev_cfg, 
        JDeviceInfo *dev_info, int flag);

void hbn_swap_serial_info(HB_NET_SERIALCFG *serial_cfg, 
        JSerialParameter *serial_info, int flag);

void hbn_swap_time_info(HB_NET_TIME *hbn_time, 
        JDeviceTime *dev_time, int flag);

void hbn_swap_ntp_info(HB_NET_NTPCFG *ntp_cfg, 
        JDeviceNTPInfo *ntp_info, int flag);

void hbn_swap_net_info(HB_NET_NETCFG *net_cfg, 
        JNetworkInfo *net_info, int flag);

void hbn_swap_pppoe_info(HB_NET_NETCFG *pppoe_cfg, 
        JPPPOEInfo *pppoe_info, int flag);

void hbn_swap_ftp_info(HB_NET_FTPRECORDCFG *ftp_cfg, 
        JFTPParameter *ftp_info, int flag);

void hbn_swap_smtp_info(HB_NET_SMTPCFG *smtp_cfg, 
        JSMTPParameter *smtp_info, int flag);

void hbn_swap_ddns_info(HB_NET_DDNSCFG *ddns_cfg, 
        JDdnsConfig *ddns_info, int flag);

void hbn_swap_encode_info(HB_NET_COMPRESSINFO *cmp_info, 
        JEncodeParameter *encode_para, int flag);

void hbn_swap_display_info(HB_NET_VIDEOPARAM *vid_cfg, 
        JDisplayParameter *display, int flag);

void hbn_swap_osd_info(HB_NET_PICCFG *pic_cfg, 
        JOSDParameter *osd_para, int flag);

void hbn_swap_decoder_info(HB_NET_DECODERCFG *dec_cfg, 
        JPTZParameter *ptz_info, int flag);

void hbn_swap_record_info(HB_NET_RECORDCFG *rec_cfg, 
        JRecordParameter *rec_para, int flag);

void hbn_swap_hide_info(HB_NET_SHELTER *shelter, 
        JHideParameter *hide_info, int flag);

void hbn_swap_move_alarm_info(HB_NET_MOTION *motion, 
        JMoveAlarm *move_alarm, int flag);

void hbn_swap_video_lost_info(HB_NET_VILOST *video_lost, 
        JLostAlarm *lost_alarm, int flag);

void hbn_swap_hide_alarm_info(HB_NET_HIDEALARM *hbn_hide_alarm, 
        JHideAlarm *hide_alarm, int flag);

void hbn_swap_alarm_in_info(HB_NET_ALARMINCFG *alarm_in, 
        JIoAlarm *io_alarm, int flag);

void hbn_swap_alarm_out_info(HB_NET_ALARMOUTCFG *alarm_out, 
        JIoAlarm *io_alarm, int flag);

void hbn_swap_disk_list(HB_NET_DISKSTAT *hbn_disk, 
        JDeviceDiskInfo *dev_disk, int flag);

void hbn_swap_store_log_info(JStoreLog *store_log_cfg, 
        JStoreLog *store_log_param, int flag);

void hbn_swap_cruise_way(HB_NET_PRESETPOLLCFG *pp_cfg, 
        JCruiseWay *crz_way, int flag);

#ifdef __cplusplus
    }
#endif

#endif  //__HBN_SWAP_H__

