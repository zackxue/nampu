#ifndef __J_JNY_SWAP_H__
#define __J_JNY_SWAP_H__

#include "NvdcDll.h"


#include "nmp_sdk.h"
#include "nmp_jny_sdkinfo.h"
#include "nmp_jny_handler.h"

#define RGB(r,g,b)		((unsigned int)(((unsigned char)(r)|((unsigned short)((unsigned char)(g))<<8)) \
									|(((unsigned short)(unsigned char)(b))<<16)))


#ifndef __cplusplus
	extern "C" {
#endif
void jny_swap_device_info(ST_DeviceSummaryParam *dev_cfg, JDeviceInfo *dev_info, int flag);

void jny_swap_rs485_info(ST_RS485Param *rs485_cfg, JSerialParameter *serial_info, int flag);

void jny_swap_device_time_info(jny_device_time *dev_time, JDeviceTime *nmp_dev_time, int flag);

void jny_swap_ntp_info(ST_NTPParam *st_ntp_info, JDeviceNTPInfo *nmp_ntp_info, int flag);
void jny_swap_network_info(ST_HostNetworkParam *st_network_info, JNetworkInfo *nmp_network_info, int flag);
void jny_swap_pppoe_info(ST_PPPoEParam *st_pppoe_info, JPPPOEInfo *nmp_pppoe_info, int flag);

void jny_swap_ftp_info(ST_FTPParam *st_ftp_info, JFTPParameter *nmp_ftp_info, int flag);
void jny_swap_ddns_info(ST_DDNSParam *st_ddns_info, JDdnsConfig *nmp_ddns_info, int flag);

void jny_swap_smtp_info(ST_SMTPParam *st_smtp_info, JSMTPParameter *nmp_smtp_info, int flag);
void jny_swap_disk_info(ST_AllDiskStatistic *st_disk_info, JDeviceDiskInfo *nmp_disk_info, int flag);
void jny_swap_osd_info(ST_VideoOSDInfoParam *st_osd_info, JOSDParameter *nmp_osd_info, int flag);
void jny_swap_record_info(ST_RecordPolicyParam *st_record_info, JRecordParameter *nmp_record_info, int flag);

void jny_swap_ptz_info(ST_PTZParam *st_ptz_info, JPTZParameter *nmp_ptz_info, int flag);
void jny_swap_hide_info( ST_BlindAreaParam*st_hide_info, JHideParameter *nmp_hide_info, int flag);
void jny_swap_move_alarm_info(ST_MotionDetectionEventParam *st_move_alarm_info, JMoveAlarm *nmp_move_alarm_info, int flag);
void jny_swap_encode_info(ST_AVStreamParam *st_encode_info, JEncodeParameter *nmp_encode_info, int flag);

void jny_swap_hide_alarm_info(ST_OcclusionDetectionEventParam *st_hide_alarm_info, JHideAlarm *nmp_hide_alarm_info, int flag);
void jny_video_lost_info(ST_VideoLoseDetectionEventParam *st_video_lost_info, JLostAlarm *nmp_video_lost_info, int flag);




#ifndef __cplusplus
	}
#endif

#endif

