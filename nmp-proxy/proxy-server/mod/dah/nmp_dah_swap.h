#ifndef __DAH_SWAP_H__
#define __DAH_SWAP_H__

#include "dhnetsdk.h"
#include "dhconfigsdk.h"

#include "nmp_sdk.h"

#include "nmp_proxy_server.h"
#include "nmp_dah_sdkinfo.h"


#ifdef __cplusplus
extern "C" {
#endif

void dah_swap_system_info(DHDEV_SYSTEM_ATTR_CFG *sys_cfg, JDeviceInfo *dev_info, int flag);
void dah_swap_device_info(NET_DEVICEINFO *dev_cfg, JDeviceInfo *dev_info, int flag);
void dah_swap_serial_info(DH_RS232_CFG *rs232_cfg, JSerialParameter *serial_info, int flag);
void dah_swap_time_info(NET_TIME *dah_time, JDeviceTime *dev_time, int flag);
void dah_swap_network_info(DHDEV_NET_CFG *net_cfg, JNetworkInfo *net_info, int flag);
void dah_swap_pppoe_info(DH_REMOTE_HOST *pppoe_cfg, JPPPOEInfo *pppoe_info, int flag);
void dah_swap_ntp_info(DHDEV_NTP_CFG *ntp_cfg, JDeviceNTPInfo *ntp_info, int flag);
void dah_swap_ftp_info(DHDEV_FTP_PROTO_CFG *ftp_cfg, JFTPParameter *ftp_info, int flag);
void dah_swap_smtp_info(DH_MAIL_CFG *email_cfg, JSMTPParameter *smtp_info, int flag);
void dah_swap_ddns_info(DH_DDNS_SERVER_CFG *ddns_cfg, JDdnsConfig *ddns_info, int flag);
void dah_swap_disk_list(DH_HARDDISK_STATE *dh_disk, JDeviceDiskInfo *dev_disk, int flag);
void dah_swap_format_info(DISKCTRL_PARAM *dh_format, JFormatDisk *fmt_disk, int flag);
void dah_swap_encode_info(DH_VIDEOENC_OPT *enc_cfg, JEncodeParameter *enc_info, int flag);
void dah_swap_display_info(DH_COLOR_CFG *color, JDisplayParameter *disp_info, int flag);
void dah_swap_osd_info(DHDEV_CHANNEL_CFG *chn_cfg, JOSDParameter *osd_info, int flag);
void dah_swap_record_info(DHDEV_RECORD_CFG *rec_cfg, JRecordParameter *rec_info, int flag);
void dah_swap_move_alarm_info(DH_MOTION_DETECT_CFG *dah_motion, JMoveAlarm *move_alarm, int flag);
void dah_swap_video_lost_info(DH_VIDEO_LOST_CFG *dah_video_lost, JLostAlarm *lost_alarm, int flag);
void dah_swap_hide_alarm_info(DH_BLIND_CFG *dah_hide_alarm, JHideAlarm *hide_alarm, int flag);
void dah_swap_alarm_in_info(DH_ALARMIN_CFG *dah_alarm_in, JIoAlarm *io_alarm, int flag);
void dah_swap_hide_info(DHDEV_VIDEOCOVER_CFG *vc_cfg, JHideParameter *hide_info, int flag);
void dah_swap_ptz_info(CFG_PTZ_INFO *ptz_cfg, JPTZParameter *ptz_info, int flag);
void dah_swap_store_log_info(dah_query_t *query_record, JStoreLog *store_log, int flag);
void dah_swap_ptz_control(dah_ptz_ctrl_t *dah_ptz_ctrl, JPTZControl *ptz_ctrl, int flag);



#ifdef __cplusplus
    }
#endif


#endif  //__DAH_SWAP_H__

