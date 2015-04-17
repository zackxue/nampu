#ifndef __XMT_SWAP_H__
#define __XMT_SWAP_H__

#include "xmtnetsdk.h"

#include "nmp_sdk.h"

#include "nmp_proxy_server.h"
#include "nmp_xmt_sdkinfo.h"


#ifdef __cplusplus
extern "C" {
#endif

void xmt_swap_device_info(H264_DVR_DEVICEINFO *dev_cfg, JDeviceInfo *dev_info, int flag);
void xmt_swap_serial_info(SDK_STR_CONFIG_PTZ *ptz_cfg, JSerialParameter *serial_info, int flag);
void xmt_swap_sys_time(SDK_SYSTEM_TIME *sys_time, JDeviceTime *dev_time, int flag);
void xmt_swap_ntp_info(SDK_NetNTPConfig *ntp_cfg, JDeviceNTPInfo *ntp_info, int flag);
void xmt_swap_network_info(SDK_CONFIG_NET_COMMON *net_cfg, JNetworkInfo *net_info, int flag);
void xmt_swap_dns_info(SDK_NetDNSConfig *dns_cfg, JNetworkInfo *net_info, int flag);
void xmt_swap_pppoe_info(SDK_NetPPPoEConfig *pppoe_cfg, JPPPOEInfo *pppoe_info, int flag);
void xmt_swap_ftp_info(SDK_FtpServerConfig *ftp_cfg, JFTPParameter *ftp_info, int flag);
void xmt_swap_smtp_info(SDK_NetEmailConfig *email_cfg, JSMTPParameter *smtp_info, int flag);
void xmt_swap_ddns_info(SDK_NetDDNSConfigALL *ddns_cfg, JDdnsConfig *ddns_info, int flag);
void xmt_swap_upnp_info(SDK_NetUPNPConfig *upnp_cfg, JUPNPParameter *upnp_info, int flag);
void xmt_swap_disk_list(SDK_StorageDeviceInformationAll *xmt_disk, JDeviceDiskInfo *dev_disk, int flag);
void xmt_swap_format_info(SDK_StorageDeviceControl *storage_ctl, SDK_StorageDeviceInformationAll *dh_disk, JFormatDisk *fmt_disk);


void xmt_swap_encode_info(SDK_CONFIG_ENCODE *enc_cfg, JEncodeParameter *enc_info, int flag);
void xmt_swap_display_info(SDK_VIDEOCOLOR_PARAM *color, JDisplayParameter *disp_info, int flag);
void xmt_swap_osd_info(SDK_CONFIG_VIDEOWIDGET *osd_cfg, JOSDParameter *osd_info, int flag);

void xmt_swap_record_info(SDK_RECORDCONFIG *rec_cfg, JRecordParameter *rec_info, int flag);

void xmt_swap_move_alarm_info(SDK_MOTIONCONFIG *xmt_motion, JMoveAlarm *move_alarm, int flag);
void xmt_swap_video_lost_info(SDK_VIDEOLOSSCONFIG *xmt_video_lost, JLostAlarm *lost_alarm, int flag);
void xmt_swap_hide_alarm_info(SDK_BLINDDETECTCONFIG *xmt_hide_alarm, JHideAlarm *hide_alarm, int flag);
void xmt_swap_alarm_in_info(SDK_ALARM_INPUTCONFIG *xmt_alarm_in, JIoAlarm *io_alarm, int flag);
void xmt_swap_alarm_out_info(SDK_AlarmOutConfig *xmt_alarm_out, JIoAlarm *io_alarm, int flag);

void xmt_swap_ptz_info(SDK_STR_CONFIG_PTZ *ptz_cfg, JPTZParameter *ptz_info, int flag);
void xmt_swap_ptz_cmd(xmt_ptz_ctrl_t *xmt_ptz_ctrl, JPTZControl *ptz_ctrl, int flag);

void xmt_swap_store_log_info(xmt_query_t *query_record, JStoreLog *store_log, int flag);

/*void xmt_swap_system_info(DHDEV_SYSTEM_ATTR_CFG *sys_cfg, JDeviceInfo *dev_info, int flag);
void xmt_swap_hide_info(DHDEV_VIDEOCOVER_CFG *vc_cfg, JHideParameter *hide_info, int flag);*/



#ifdef __cplusplus
    }
#endif


#endif  //__XMT_SWAP_H__

