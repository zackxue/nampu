
#ifndef __J_TRANSFORM_H__
#define __J_TRANSFORM_H__

#include "nmp_packet.h"

/*extern void process_register_request(RegisterRequestPacket *g_reg_request, 
												RegisterRequest *reg_request, JAction action);
extern void process_register_response(RegisterResponsePacket *g_reg_response, 
												RegisterResponse *reg_response, JAction action);
extern void process_heart_beat_request(HeartBeatRequestPacket *g_heart_beat, 
													HeartBeatRequest *heart_beat, JAction action);
extern void process_heart_beat_response(HeartBeatResponsePacket *g_heart_beat, 
													HeartBeatResponse *heart_beat, JAction action);*/
//extern void process_change_dispatch(ChangeDispatchPacket *g_change_disp, 
	//											ChangeDispatch *change_disp, JAction action);
extern void process_device_info(DeviceInfoPacket *g_dev_info, 
				JDeviceInfo *dev_info, JAction action);
extern void process_device_ntp_info(DeviceNTPInfoPacket *g_dev_ntp_info, 
				JDeviceNTPInfo *dev_ntp_info, JAction action);
extern void process_device_time(DeviceTimePacket *g_dev_time, 
				JDeviceTime *dev_time, JAction action);
extern void process_platform_info(PlatformInfoPacket *g_pltf_info, 
				JPlatformInfo *pltf_info, JAction action);
extern void process_network_info(NetworkInfoPacket *g_net_info, 
				JNetworkInfo *net_info, JAction action);
extern void process_pppoe_info(PPPOEInfoPacket *g_pppoe_info,
				JPPPOEInfo *pppoe_info, JAction action);
extern void process_encode_parameter(EncodeParameterPacket *g_encode_para, 
				JEncodeParameter *encode_para, JAction action);
extern void process_display_parameter(DisplayParameterPacket *g_display_para, 
				JDisplayParameter *display_para, JAction action);
extern void process_record_parameter(RecordParameterPacket *g_record_para, 
				JRecordParameter *record_para, JAction action);
extern void process_hide_parameter(HideParameterPacket *g_hide_para, 
				JHideParameter *hide_para, JAction action);
extern void process_serial_parameter(SerialParameterPacket *g_serial_para, 
				JSerialParameter *serial_para, JAction action);
extern void process_osd_parameter(OSDParameterPacket *g_osd_para, 
				JOSDParameter *osd_para, JAction action);
extern void process_ptz_parameter(PTZParameterPacket *g_ptz_para, 
				JPTZParameter *ptz_para, JAction action);
extern void process_ftp_parameter(FTPParameterPacket *g_ftp_para, 
				JFTPParameter *ftp_para, JAction action);
extern void process_smtp_parameter(SMTPParameterPacket *g_smtp_para, 
				JSMTPParameter *smtp_para, JAction action);
extern void process_upnp_parameter(UPNPParameterPacket *g_upnp_para, 
				JUPNPParameter *upnp_para, JAction action);
extern void process_device_disk_info(DeviceDiskInfoPacket *g_disk_info, 
				JDeviceDiskInfo *disk_info, JAction action);
extern void process_format_disk(FormatDiskPacket *g_format_disk, 
				JFormatDisk *format_disk, JAction action);
extern void process_move_alarm(MoveAlarmPacket *g_move_alarm, 
				JMoveAlarm *move_alarm, JAction action);
extern void process_lost_alarm(LostAlarmPacket *g_lost_alarm, 
				JLostAlarm *lost_alarm, JAction action);
extern void process_hide_alarm(HideAlarmPacket *g_hide_alarm, 
				JHideAlarm *hide_alarm, JAction action);
extern void process_io_alarm(IoAlarmPacket *g_io_alarm, 
				JIoAlarm *io_alarm, JAction action);
extern void process_joint_action(JointActionPacket *g_joint_action, 
				JJointAction *joint_action, JAction action);
extern void process_control_ptz_cmd(PTZControlPacket *g_ptz_ctrl, 
				JPTZControl *ptz_ctrl, JAction action);
extern void process_store_log(StoreLogPacket *g_store, 
				JStoreLog *store, JAction action);
extern void process_firmware_upgrade(FirmwareUpgradePacket *g_upgrade, 
				JFirmwareUpgrade *upgrade, JAction action);

extern void process_channel_info(ChannelInfoPacket *g_ch_info, 
				JChannelInfo *ch_info, JAction action);
extern void process_picture_info(PictureInfoPacket *g_pic_info, 
				JPictureInfo *pic_info, JAction action);
extern void process_wifi_config(WifiConfigPacket *g_wifi_cfg, 
				JWifiConfig *wifi_cfg, JAction action);
extern void process_wifi_search(WifiSearchResPacket *g_wifi_search, 
				JWifiSearchRes *wifi_search, JAction action);
extern void process_network_status(NetworkStatusPacket *g_net_status, 
				JNetworkStatus *net_status, JAction action);
extern void process_control_device(ControlDevicePacket *g_cntrl_dev, 
				JControlDevice *cntrl_dev, JAction action);
extern void process_ddns_config(DdnsConfigPacket *g_ddns_cfg, 
				JDdnsConfig *ddns_cfg, JAction action);
extern void process_avd_config(AvdConfigPacket *g_avd_cfg, 
				JAvdConfig *avd_cfg, JAction action);


#endif //__J_TRANSFORM_H__

