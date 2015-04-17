#include <stdio.h>
#include <string.h>

#include "nmp_jny_proc_cfg.h"



int jny_proc_get_device_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{printf("---------------jny_proc_get_device_cfg-----enter\n");
	int ret;
    char addr[MAX_IP_LEN];
	ST_InetAddr p_stAddr;
	ST_DeviceSummaryParam p_pstDeviceSummaryInfo;
	
	NMP_ASSERT(parm && sdk_info);

    memset(addr, 0, sizeof(addr));
    if (!proxy_resolve_host_immediate((const char*)sdk_info->dev_host, addr, sizeof(addr)))
	{
		show_debug("Proxy device ipaddr is not found!!!!!!!!!!!!!!\n");
		return -1;
	}

	memset(&p_stAddr, 0, sizeof(ST_InetAddr));
	memcpy(p_stAddr.szHostIP, addr, strlen(addr));
	p_stAddr.nPORT = sdk_info->dev_port;
	p_stAddr.nIPProtoVer = IPPROTO_V4;

	memset(&p_pstDeviceSummaryInfo, 0, sizeof(ST_DeviceSummaryParam));
	ret = Remote_System_GetSystemInfo(user_id, p_stAddr, &p_pstDeviceSummaryInfo, UDP);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}

	memcpy(parm, (void *)&p_pstDeviceSummaryInfo, sizeof(ST_DeviceSummaryParam));
	return 0;
}

int jny_proc_get_serial_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	ST_RS485Param get_rs485_param;
	int com_id;
	
	NMP_ASSERT(parm && sdk_info);

	com_id = ((ST_RS485Param *)parm)->nComId;
	ret = Remote_System_GetRS485Device(user_id, com_id, &get_rs485_param);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	memcpy(parm, (void *)&get_rs485_param, sizeof(ST_RS485Param));
	return 0;
}

int jny_proc_set_serial_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	return 0;
}


int jny_proc_get_dev_time_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_device_time dev_time;

	NMP_ASSERT(parm && sdk_info);

	ret = Remote_System_GetDeviceTime(user_id, &dev_time.nYearGet, &dev_time.nMonthGet, 
										&dev_time.nDayGet, &dev_time.nHourGet, 
										&dev_time.nMinuteGet, &dev_time.nSecondGet);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	memcpy(parm, (void *)&dev_time, sizeof(jny_device_time));
	return 0;
}

int jny_proc_set_dev_time_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_device_time *dev_time;

	NMP_ASSERT(parm && sdk_info);

	dev_time = (jny_device_time *)parm;
	ret = Remote_System_SetDeviceTime(user_id, dev_time->nYearGet, dev_time->nMonthGet, 
										dev_time->nDayGet, dev_time->nHourGet, 
										dev_time->nMinuteGet, dev_time->nSecondGet);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return 0;
}

int jny_proc_get_ntp_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	ST_NTPParam jny_ntp_info;

	NMP_ASSERT(parm && sdk_info);

	ret = Remote_System_GetNTPParam(user_id, IPPROTO_V4, &jny_ntp_info);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	memcpy(parm, (void *)&jny_ntp_info, sizeof(ST_NTPParam));
	return 0;
}

int jny_proc_set_ntp_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	ST_NTPParam *st_ntp_info = NULL;

	NMP_ASSERT(parm && sdk_info);
	
	st_ntp_info = (ST_NTPParam *)parm;
	ret = Remote_System_SetNTPParam(user_id, IPPROTO_V4, st_ntp_info);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return 0;
}


int jny_proc_get_network_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	ST_HostNetworkParam st_network_info;

	NMP_ASSERT(parm && sdk_info);
	
	ret = Remote_System_GetHostNetwork(user_id, IPPROTO_V4, &st_network_info);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	memcpy(parm, (void *)&st_network_info, sizeof(ST_HostNetworkParam));
	return ret;
}

//DEBUG: nmp_jny_proc_cfg.cpp(170) Receiving data timeout., -213
int jny_proc_set_network_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	ST_HostNetworkParam *st_network_info = NULL;

	NMP_ASSERT(parm && sdk_info);

	st_network_info = (ST_HostNetworkParam *)parm;
	ret = Remote_System_SetHostNetwork(user_id, IPPROTO_V4, st_network_info);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_get_pppoe_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	ST_PPPoEParam st_pppoe_info;

	NMP_ASSERT(parm && sdk_info);
	
	ret = Remote_System_GetPPPoEParam(user_id, &st_pppoe_info);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	memcpy(parm, (void *)&st_pppoe_info, sizeof(ST_PPPoEParam));
	return ret;
}

int jny_proc_set_pppoe_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	ST_PPPoEParam *st_pppoe_info;

	NMP_ASSERT(parm && sdk_info);

	st_pppoe_info = (ST_PPPoEParam *)parm;
	ret = Remote_System_GetPPPoEParam(user_id, st_pppoe_info);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_get_ftp_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	ST_FTPParam *st_ftp_info;

	NMP_ASSERT(parm && sdk_info);

	st_ftp_info = (ST_FTPParam *)parm;
	ret = Remote_System_GetFTPParam(user_id, IPPROTO_V4, st_ftp_info);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_set_ftp_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	ST_FTPParam *st_ftp_info;

	NMP_ASSERT(parm && sdk_info);

	st_ftp_info = (ST_FTPParam *)parm;
	ret = Remote_System_SetFTPParam(user_id, IPPROTO_V4, st_ftp_info);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;

}


int jny_proc_get_ddns_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	ST_DDNSParam *st_ddns_info;

	NMP_ASSERT(parm && sdk_info);

	st_ddns_info = (ST_DDNSParam *)parm;
	ret = Remote_System_GetDDNSParam(user_id, st_ddns_info);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_set_ddns_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	ST_DDNSParam *st_ddns_info;

	NMP_ASSERT(parm && sdk_info);

	st_ddns_info = (ST_DDNSParam *)parm;
	ret = Remote_System_SetDDNSParam(user_id, st_ddns_info);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;

}

int jny_proc_get_smtp_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	ST_SMTPParam *st_smtp_info;

	NMP_ASSERT(parm && sdk_info);

	st_smtp_info = (ST_SMTPParam *)parm;
	ret = Remote_System_GetSMTPParam(user_id, st_smtp_info);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_set_smtp_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	ST_SMTPParam *st_smtp_info;

	NMP_ASSERT(parm && sdk_info);

	st_smtp_info = (ST_SMTPParam *)parm;
	ret = Remote_System_SetSMTPParam(user_id, st_smtp_info);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_get_disk_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	ST_AllDiskStatistic *st_disk_info;

	NMP_ASSERT(parm && sdk_info);

	st_disk_info = (ST_AllDiskStatistic *)parm;
	ret = Remote_System_GetDeviceDiskInfo(user_id, st_disk_info);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_ctrl_device(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	JControlDevice *nmp_ctrl;

	NMP_ASSERT(parm && sdk_info);

	nmp_ctrl = (JControlDevice *)parm;

	switch(nmp_ctrl->command)
	{
		case SHUTDOWN_DEVICE:	    
		    Remote_System_ShutDown(user_id);
			ret = 0;
		    break;
		case RESTART_DEVICE:
			Remote_System_Restart(user_id);
		    ret = 0;
		    break;
		case RESTORE_DEFAULT:
			Remote_System_Reset(user_id);
		    ret = 0;
		    break;

		case DETECT_DEAL_PIX:
		    ret = -1;
		    break;
		case DETECT_IRIS:
		    ret = -1;
		    break;

		default:
		    ret = -1;
		    break;
	}
	return ret;
}


int jny_proc_get_osd_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *osd_info;

	NMP_ASSERT(parm && sdk_info);

	osd_info = (jny_parm_info *)parm;
	ret = Remote_System_GetVideoOSD_V2(user_id, osd_info->channel, (ST_VideoOSDInfoParam *)osd_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_set_osd_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *osd_info;

	NMP_ASSERT(parm && sdk_info);

	osd_info = (jny_parm_info *)parm;
	ret = Remote_System_SetVideoOSD_V2(user_id, osd_info->channel, (ST_VideoOSDInfoParam *)osd_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

//DEBUG: nmp_jny_proc_cfg.cpp(418) Device param was not configured., -513
int jny_proc_get_ptz_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	//ST_PTZParam *st_ptz_info;
	jny_parm_info *ptz_info;
	NMP_ASSERT(parm && sdk_info);

	ptz_info = (jny_parm_info *)parm;
	//st_ptz_info = (ST_PTZParam *)ptz_info->parm_ptr;
	ret = Remote_System_GetPTZParam(user_id, ptz_info->channel, (ST_PTZParam *)ptz_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}


int jny_proc_set_ptz_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *ptz_info;
	NMP_ASSERT(parm && sdk_info);

	ptz_info = (jny_parm_info *)parm;

	ret = Remote_System_SetPTZParam(user_id, ptz_info->channel, (ST_PTZParam *)ptz_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_control_ptz(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *control_ptz_info;
	JPTZControl *ptz_ctrl;

	NMP_ASSERT(parm && sdk_info);

	control_ptz_info = (jny_parm_info *)parm;
	ptz_ctrl = (JPTZControl*)control_ptz_info->parm_ptr;
	ret = Remote_PTZEx_Open(user_id, control_ptz_info->channel);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}


	switch((int)ptz_ctrl->action)
	{
		case PTZ_AUTO:
			ret = -1;
			break;
			
		case PTZ_LEFT:
			ret = Remote_PTZEx_RotateLeft(user_id, 60);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_RIGHT:
			ret = Remote_PTZEx_RotateRight(user_id, 60);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_UP:
			ret = Remote_PTZEx_RotateUp(user_id, 60);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_DOWN:
			ret = Remote_PTZEx_RotateDown(user_id, 60);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_LEFT_UP:
			ret = Remote_PTZEx_RotateLeftUp(user_id, 60);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_LEFT_DOWN:
			ret = Remote_PTZEx_RotateLeftDown(user_id, 60);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_RIGHT_UP:
			ret = Remote_PTZEx_RotateRightUp(user_id, 60);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_RIGHT_DOWN:
			ret = Remote_PTZEx_RotateRightDown(user_id, 60);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_ADD_ZOOM:
			ret = Remote_PTZEx_ZoomIn(user_id);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_SUB_ZOOM:
			ret = Remote_PTZEx_ZoomOut(user_id);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_ADD_FOCUS:
			ret = Remote_PTZEx_FocusFar(user_id);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_SUB_FOCUS:
			ret = Remote_PTZEx_FocusNear(user_id);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_TURN_ON:
			ret = Remote_PTZEx_OpenLight(user_id);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_TURN_OFF:
			ret = Remote_PTZEx_CloseLight(user_id);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_WIPERS_ON:
			ret = Remote_PTZEx_RunBrush(user_id);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				ret = -1;
			}
			break;
			
		case PTZ_WIPERS_OFF:
			ret = -1;
			break;
			
		default:
			ret = -1;
			break;
			
	}

	ret = Remote_PTZEx_Close(user_id);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;

}

int jny_proc_set_preset_point_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *preset_point_info;
	
	NMP_ASSERT(parm && sdk_info);
	preset_point_info = (jny_parm_info *)parm;
	JPPConfig *pp_cfg = (JPPConfig *)preset_point_info->parm_ptr;
    switch (pp_cfg->action)
    {
        case PTZ_SET_PP:
			ret = Remote_PTZEx_PresetSet(user_id, pp_cfg->pp.preset);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				return -1;
			}

            break;
        case PTZ_USE_PP:
			ret = Remote_PTZEx_PresetInvoke(user_id, pp_cfg->pp.preset);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				return -1;
			}
            break;
			
        case PTZ_DEL_PP:
			ret = Remote_PTZEx_PresetRemove(user_id, pp_cfg->pp.preset);
			if(ret != SN_SUCCESS)
			{
				jny_print_error(ret);
				return -1;
			}
            break;
			
        default:
            ret = -1;
            break;
    }
	return ret;
}

int jny_proc_add_cruise_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	int i;
	JCruiseWay *cruise_parm;
	jny_parm_info *cruise_info;

	NMP_ASSERT(parm && sdk_info);

	cruise_info = (jny_parm_info *)parm;
	cruise_parm = (JCruiseWay *)cruise_info->parm_ptr;

	for(i  = 0; (unsigned int)i < cruise_parm->pp_count; i++)
	{
		ret = Remote_PTZEx_AddTourPoint(user_id, cruise_parm->crz_pp[i].preset, cruise_parm->crz_pp[i].speed, cruise_parm->crz_pp[i].dwell);
		if(ret != SN_SUCCESS)
		{
			jny_print_error(ret);
			return -1;
		}
	}
	return ret;
}


int jny_proc_get_record_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret = 0;

	jny_parm_info *record_info;

	NMP_ASSERT(parm && sdk_info);

	record_info = (jny_parm_info *)parm;

	ret = Remote_System_GetRecordPolicy(user_id, record_info->channel, (ST_RecordPolicyParam *)record_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_set_record_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret = 0;

	jny_parm_info *record_info;

	NMP_ASSERT(parm && sdk_info);

	record_info = (jny_parm_info *)parm;

	ret = Remote_System_SetRecordPolicy(user_id, record_info->channel, (ST_RecordPolicyParam *)record_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}


int jny_proc_get_hide_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *hide_info;

	NMP_ASSERT(parm && sdk_info);

	hide_info = (jny_parm_info *)parm;
	ret = Remote_System_GetBlindAreaParam(user_id, hide_info->channel, (ST_BlindAreaParam *)hide_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_set_hide_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *hide_info;

	NMP_ASSERT(parm && sdk_info);

	hide_info = (jny_parm_info *)parm;
	ret = Remote_System_SetBlindAreaParam(user_id, hide_info->channel, (ST_BlindAreaParam *)hide_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}


int jny_proc_get_move_alarm_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *move_alarm_info;

	NMP_ASSERT(parm && sdk_info);

	move_alarm_info = (jny_parm_info *)parm;
	ret = Remote_System_GetMotionDetectionEvent(user_id, move_alarm_info->channel, 
											(ST_MotionDetectionEventParam *)move_alarm_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}



int jny_proc_set_move_alarm_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *move_alarm_info;

	NMP_ASSERT(parm && sdk_info);

	move_alarm_info = (jny_parm_info *)parm;
	ret = Remote_System_SetMotionDetectionEvent(user_id, move_alarm_info->channel, 
											(ST_MotionDetectionEventParam *)move_alarm_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_get_capability_list(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret = 0;
	JDevCap *dev_cap;
	ST_DeviceAbility stDeviceAbility;
	
	NMP_ASSERT(parm && sdk_info);

	memset(&stDeviceAbility, 0, sizeof(stDeviceAbility));
	ret = Remote_System_GetDeviceAbility(user_id, &stDeviceAbility);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}

	dev_cap = (JDevCap*)parm;
	
	dev_cap->cap0 = DEV_CAP_VIDEO_IN | DEV_CAP_AUDIO | DEV_CAP_IRIS | DEV_CAP_PTZ | 
		DEV_CAP_ALARM_IN | DEV_CAP_ALARM_OUT | DEV_CAP_STORAGE | DEV_CAP_WEB | DEV_CAP_PLATFORM | 
		DEV_CAP_INTELLIGENT_ANALYSIS | DEV_CAP_UPDATE | DEV_CAP_VIDEO_OUT;// | DEV_CAP_IR;
		
	dev_cap->ftp_enable = 0;
	dev_cap->upnp_enable = 0;
	dev_cap->chn_cap.size = sizeof(JChnCap);
	dev_cap->chn_cap.encode = VIDEO_ENCODE_H264_E | VIDEO_ENCODE_MJPEG_E | 
								VIDEO_ENCODE_JPEG_E | VIDEO_ENCODE_MPEG4_E;
	
	dev_cap->chn_cap.supp_mask = 1;
	dev_cap->chn_cap.mask_count = 1;
	dev_cap->chn_cap.supp_hide_alarm = 1;
	dev_cap->chn_cap.hide_alarm_count = 1;
	dev_cap->chn_cap.supp_move_alarm = 1;
	dev_cap->chn_cap.move_alarm_count = 1;
	dev_cap->chn_cap.supp_video_lost_alarm = 1;
	dev_cap->chn_cap.osd_count = 1;
	dev_cap->chn_cap.stream_count = 2;
	dev_cap->chn_cap.stream_supp_resolution[0] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | CAP_VIDEO_HD1 | 
		CAP_VIDEO_D1 | CAP_VIDEO_QQVGA | CAP_VIDEO_QVGA | CAP_VIDEO_VGA | CAP_VIDEO_SVGA | 
		CAP_VIDEO_UXGA | CAP_VIDEO_720P | CAP_VIDEO_960 | CAP_VIDEO_1080P;
	
	dev_cap->chn_cap.stream_supp_resolution[1] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | CAP_VIDEO_HD1 | 
		CAP_VIDEO_D1 | CAP_VIDEO_QQVGA | CAP_VIDEO_QVGA | CAP_VIDEO_VGA | CAP_VIDEO_SVGA | 
		CAP_VIDEO_UXGA | CAP_VIDEO_720P | CAP_VIDEO_960 | CAP_VIDEO_1080P;
	
	dev_cap->chn_cap.stream_supp_resolution[2] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | CAP_VIDEO_HD1 | 
		CAP_VIDEO_D1 | CAP_VIDEO_QQVGA | CAP_VIDEO_QVGA | CAP_VIDEO_VGA | CAP_VIDEO_SVGA | 
		CAP_VIDEO_UXGA | CAP_VIDEO_720P | CAP_VIDEO_960 | CAP_VIDEO_1080P;
	
	dev_cap->chn_cap.stream_supp_resolution[3] = CAP_VIDEO_QCIF | CAP_VIDEO_CIF | CAP_VIDEO_HD1 | 
		CAP_VIDEO_D1 | CAP_VIDEO_QQVGA | CAP_VIDEO_QVGA | CAP_VIDEO_VGA | CAP_VIDEO_SVGA | 
		CAP_VIDEO_UXGA | CAP_VIDEO_720P | CAP_VIDEO_960 | CAP_VIDEO_1080P;
	
	dev_cap->chn_cap.stream_max_frate_rate[0] = 30;
	
	dev_cap->chn_cap.img_cap = IMA_BRIGHTNESS | IMA_CONTRAST | IMA_SATURATION | IMA_HUE | IMA_GAMMA |
								IMA_SHARPNESS;
	return ret;
	
}

int jny_proc_get_encode_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *encode_info;

	NMP_ASSERT(parm && sdk_info);

	encode_info = (jny_parm_info *)parm;
	ret = Remote_System_GetAVStreamParam(user_id, encode_info->channel, 0,
											(ST_AVStreamParam *)encode_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return 0;
}

int jny_proc_set_encode_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *encode_info;

	NMP_ASSERT(parm && sdk_info);
	encode_info = (jny_parm_info *)parm;
	ret = Remote_System_SetAVStreamParam(user_id, encode_info->channel, ((ST_AVStreamParam *)encode_info->parm_ptr)->nStreamId,
											(ST_AVStreamParam *)encode_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_get_hide_alarm_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *hide_alarm_info;

	NMP_ASSERT(parm && sdk_info);
	hide_alarm_info = (jny_parm_info *)parm;
	ret = Remote_System_GetOcclusionDetectionEvent(user_id, hide_alarm_info->channel,
								(ST_OcclusionDetectionEventParam *)hide_alarm_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_set_hide_alarm_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *hide_alarm_info;

	NMP_ASSERT(parm && sdk_info);
	hide_alarm_info = (jny_parm_info *)parm;
	ret = Remote_System_SetOcclusionDetectionEvent(user_id, hide_alarm_info->channel,
								(ST_OcclusionDetectionEventParam *)hide_alarm_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_get_video_lost_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *video_lost_info;

	NMP_ASSERT(parm && sdk_info);
	video_lost_info = (jny_parm_info *)parm;
	ret = Remote_System_GetVideoLoseDetectionEvent(user_id, video_lost_info->channel,
								(ST_VideoLoseDetectionEventParam *)video_lost_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

int jny_proc_set_video_lost_cfg(int user_id, void *parm, proxy_sdk_t *sdk_info)
{
	int ret;
	jny_parm_info *video_lost_info;

	NMP_ASSERT(parm && sdk_info);
	video_lost_info = (jny_parm_info *)parm;
	ret = Remote_System_SetVideoLoseDetectionEvent(user_id, video_lost_info->channel,
								(ST_VideoLoseDetectionEventParam *)video_lost_info->parm_ptr);
	if(ret != SN_SUCCESS)
	{
		jny_print_error(ret);
		return -1;
	}
	return ret;
}

