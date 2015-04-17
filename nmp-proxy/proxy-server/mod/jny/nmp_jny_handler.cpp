#include "nmp_jny_swap.h"
#include "nmp_jny_service.h"
#include "nmp_jny_handler.h"

#define DEF_JNY_FACTORY_INFO   "JNY" 


int jny_get_device_info(struct service *srv, int channel, int parm_id, void *pvalue)
{printf("---------jny_get_device_info------------enter\n");
	int ret;
	JDeviceInfo *dev_info;
	jny_service_basic *jny_basic;
	ST_DeviceSummaryParam jny_device_info;

	dev_info = (JDeviceInfo*)pvalue;
    jny_basic = (jny_service_basic *)srv->tm;

	memset(&jny_device_info, 0, sizeof(ST_DeviceSummaryParam));
    ret = (*jny_basic->ro.get_config)(srv, parm_id, (void*)&jny_device_info);
    if (!ret)
    {
        strncpy((char*)dev_info->manu_info, DEF_JNY_FACTORY_INFO, strlen(DEF_JNY_FACTORY_INFO));
        jny_swap_device_info(&jny_device_info, dev_info, SWAP_PACK);
    }
	printf("-----------------ret = %d\n", ret);
	return ret;
}

int jny_get_serial_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JSerialParameter *serial_info;
    jny_service_basic *jny_basic;
	ST_RS485Param rs485_param;

    NMP_ASSERT(srv && pvalue);

    serial_info = (JSerialParameter *)pvalue;
    jny_basic = (jny_service_basic *)srv->tm;
printf("--jny_get_serial_info---------serial_info->serial_no= %d\n", serial_info->serial_no);
	memset(&rs485_param, 0, sizeof(ST_RS485Param));
	rs485_param.nComId = serial_info->serial_no + 1;
    ret = (*jny_basic->ro.get_config)(srv, parm_id, &rs485_param);
    if (!ret)
    {
		jny_swap_rs485_info(&rs485_param, serial_info, SWAP_PACK);
    }

    return ret;
}


int jny_set_serial_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	return 0;
}

int jny_get_device_time(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceTime *device_time;
    jny_service_basic *jny_basic;
	jny_device_time jny_device_time;

    NMP_ASSERT(srv && pvalue);

    device_time = (JDeviceTime*)pvalue;
    jny_basic = (jny_service_basic*)srv->tm;

	memset(&jny_device_time, 0, sizeof(jny_device_time));
    ret = (*jny_basic->ro.get_config)(srv, parm_id, &jny_device_time);
    if (!ret)
    {
		jny_swap_device_time_info(&jny_device_time, device_time, SWAP_PACK);
    }

    return ret;

}

int jny_set_device_time(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceTime *device_time;
    jny_service_basic *jny_basic;
	jny_device_time jny_device_time;

    NMP_ASSERT(srv && pvalue);

    device_time = (JDeviceTime*)pvalue;
    jny_basic = (jny_service_basic*)srv->tm;

	memset(&jny_device_time, 0, sizeof(jny_device_time));
    ret = (*jny_basic->ro.get_config)(srv, parm_id-1, &jny_device_time);
    if (!ret)
    {
		jny_swap_device_time_info(&jny_device_time, device_time, SWAP_UNPACK);
    	ret = (*jny_basic->ro.set_config)(srv, parm_id, &jny_device_time);

	}

    return ret;
}

int jny_get_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceNTPInfo *ntp_info;
    jny_service_basic *jny_basic;
	ST_NTPParam jny_ntp_info;

    NMP_ASSERT(srv && pvalue);

    ntp_info = (JDeviceNTPInfo*)pvalue;
    jny_basic = (jny_service_basic*)srv->tm;

	memset(&jny_ntp_info, 0, sizeof(ST_NTPParam));
    ret = (*jny_basic->ro.get_config)(srv, parm_id, &jny_ntp_info);
    if (!ret)
    {
		jny_swap_ntp_info(&jny_ntp_info, ntp_info, SWAP_PACK);
    }

    return ret;

}


int jny_set_ntp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JDeviceNTPInfo *ntp_info;
    jny_service_basic *jny_basic;	
	ST_NTPParam jny_ntp_info;
	

    NMP_ASSERT(srv && pvalue);

    ntp_info = (JDeviceNTPInfo*)pvalue;
    jny_basic = (jny_service_basic *)srv->tm;

	memset(&jny_ntp_info, 0, sizeof(ST_NTPParam));
    ret = (*jny_basic->ro.get_config)(srv, parm_id-1, &jny_ntp_info);
    if (!ret)
    {
		jny_swap_ntp_info(&jny_ntp_info, ntp_info, SWAP_UNPACK);
    	ret = (*jny_basic->ro.set_config)(srv, parm_id, &jny_ntp_info);

	}

    return ret;
}

int jny_get_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JNetworkInfo *network_info;
    jny_service_basic *jny_basic;
	ST_HostNetworkParam st_network_info;

    NMP_ASSERT(srv && pvalue);

    network_info = (JNetworkInfo*)pvalue;
    jny_basic = (jny_service_basic *)srv->tm;

	memset(&st_network_info, 0, sizeof(ST_HostNetworkParam));
    ret = (*jny_basic->ro.get_config)(srv, parm_id, &st_network_info);
    if (!ret)
    {
		jny_swap_network_info(&st_network_info, network_info, SWAP_PACK);
    }

    return ret;
}

int jny_set_network_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret;
    JNetworkInfo *nmp_network_info;
    jny_service_basic *jny_basic;	
	ST_HostNetworkParam st_network_info;
	

    NMP_ASSERT(srv && pvalue);

    nmp_network_info = (JNetworkInfo *)pvalue;
    jny_basic = (jny_service_basic *)srv->tm;

	memset(&st_network_info, 0, sizeof(ST_HostNetworkParam));
    ret = (*jny_basic->ro.get_config)(srv, parm_id-1, &st_network_info);
    if (!ret)
    {
		jny_swap_network_info(&st_network_info, nmp_network_info, SWAP_UNPACK);
    	ret = (*jny_basic->ro.set_config)(srv, parm_id, &st_network_info);
	}

    return ret;
}

int jny_get_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JPPPOEInfo *nmp_pppoe_info;
	jny_service_basic *jny_basic;
	ST_PPPoEParam st_pppoe_info;

	NMP_ASSERT(srv && pvalue);

	nmp_pppoe_info = (JPPPOEInfo *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;

	memset(&st_pppoe_info, 0, sizeof(ST_PPPoEParam));
	ret = (*jny_basic->ro.get_config)(srv, parm_id, &st_pppoe_info);
	if(!ret)
	{
		jny_swap_pppoe_info(&st_pppoe_info, nmp_pppoe_info, SWAP_PACK);
	}
	return ret;
}

int jny_set_pppoe_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JPPPOEInfo *nmp_pppoe_info;
	jny_service_basic *jny_basic;
	ST_PPPoEParam st_pppoe_info;

	NMP_ASSERT(srv && pvalue);

	nmp_pppoe_info = (JPPPOEInfo *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;

	memset(&st_pppoe_info, 0, sizeof(ST_PPPoEParam));
    ret = (*jny_basic->ro.get_config)(srv, parm_id-1, &st_pppoe_info);
    if (!ret)
    {
		jny_swap_pppoe_info(&st_pppoe_info, nmp_pppoe_info, SWAP_UNPACK);
    	ret = (*jny_basic->ro.set_config)(srv, parm_id, &st_pppoe_info);
	}
	return ret;
}

int jny_get_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JFTPParameter *nmp_ftp_info;
	jny_service_basic *jny_basic;
	ST_FTPParam st_ftp_info;

	NMP_ASSERT(srv && pvalue);

	nmp_ftp_info = (JFTPParameter *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;

	memset(&st_ftp_info, 0, sizeof(ST_FTPParam));
	ret = (*jny_basic->ro.get_config)(srv, parm_id, &st_ftp_info);
	if(!ret)
	{
		jny_swap_ftp_info(&st_ftp_info, nmp_ftp_info, SWAP_PACK);
	}
	return ret;
}

int jny_set_ftp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JFTPParameter *nmp_ftp_info;
	jny_service_basic *jny_basic;
	ST_FTPParam st_ftp_info;

	NMP_ASSERT(srv && pvalue);

	nmp_ftp_info = (JFTPParameter *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;

	memset(&st_ftp_info, 0, sizeof(ST_FTPParam));
    ret = (*jny_basic->ro.get_config)(srv, parm_id-1, &st_ftp_info);
    if (!ret)
    {
		jny_swap_ftp_info(&st_ftp_info, nmp_ftp_info, SWAP_UNPACK);
    	ret = (*jny_basic->ro.set_config)(srv, parm_id, &st_ftp_info);
	}
	return ret;
}


int jny_get_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JSMTPParameter *nmp_smtp_info;
	jny_service_basic *jny_basic;
	ST_SMTPParam st_smtp_info;

	NMP_ASSERT(srv && pvalue);

	nmp_smtp_info = (JSMTPParameter *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;

	memset(&st_smtp_info, 0, sizeof(ST_SMTPParam));
    ret = (*jny_basic->ro.get_config)(srv, parm_id, &st_smtp_info);
    if (!ret)
    {
		jny_swap_smtp_info(&st_smtp_info, nmp_smtp_info, SWAP_PACK);
	}
	return 0;

}

int jny_set_smtp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JSMTPParameter *nmp_smtp_info;
	jny_service_basic *jny_basic;
	ST_SMTPParam st_smtp_info;

	NMP_ASSERT(srv && pvalue);

	nmp_smtp_info = (JSMTPParameter *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;

	memset(&st_smtp_info, 0, sizeof(ST_SMTPParam));
    ret = (*jny_basic->ro.get_config)(srv, parm_id - 1, &st_smtp_info);
    if (!ret)
    {
		jny_swap_smtp_info(&st_smtp_info, nmp_smtp_info, SWAP_UNPACK);
		ret = (*jny_basic->ro.set_config)(srv, parm_id, &st_smtp_info);
	}
	return 0;

}


int jny_get_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JDdnsConfig *nmp_ddns_info;
	jny_service_basic *jny_basic;
	ST_DDNSParam st_ddns_info;

	NMP_ASSERT(srv && pvalue);

	nmp_ddns_info = (JDdnsConfig *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;

	memset(&st_ddns_info, 0, sizeof(ST_DDNSParam));
	ret = (*jny_basic->ro.get_config)(srv, parm_id, &st_ddns_info);
	if(!ret)
	{
		jny_swap_ddns_info(&st_ddns_info, nmp_ddns_info, SWAP_PACK);
	}
	return ret;
}

int jny_set_ddns_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JDdnsConfig *nmp_ddns_info;
	jny_service_basic *jny_basic;
	ST_DDNSParam st_ddns_info;

	NMP_ASSERT(srv && pvalue);

	nmp_ddns_info = (JDdnsConfig *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;

	memset(&st_ddns_info, 0, sizeof(ST_DDNSParam));
	ret = (*jny_basic->ro.get_config)(srv, parm_id - 1, &st_ddns_info);
	if(!ret)
	{
		jny_swap_ddns_info(&st_ddns_info, nmp_ddns_info, SWAP_UNPACK);
		ret = (*jny_basic->ro.set_config)(srv, parm_id, &st_ddns_info);
	}
	return ret;
}

int jny_get_upnp_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	return 0;
}


int jny_get_disk_list(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JDeviceDiskInfo *nmp_disk_info;
	jny_service_basic *jny_basic;
	ST_AllDiskStatistic st_disk_info;

	NMP_ASSERT(srv && pvalue);

	nmp_disk_info = (JDeviceDiskInfo *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;

	memset(&st_disk_info, 0, sizeof(ST_AllDiskStatistic));
	ret = (*jny_basic->ro.get_config)(srv, parm_id, &st_disk_info);
	if(!ret)
	{
		jny_swap_disk_info(&st_disk_info, nmp_disk_info, SWAP_PACK);
	}
	return ret;
}

int jny_format_disk(struct service *srv, int channel, int parm_id, void *pvalue)
{
	return 0;
}


int jny_control_device(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	//JControlDevice *nmp_ctrl;
	jny_service_basic *jny_basic;

	NMP_ASSERT(srv && pvalue);

	jny_basic = (jny_service_basic *)srv->tm;

	ret = (*jny_basic->ro.set_config)(srv, parm_id, pvalue);
	return ret;
}


int jny_get_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret = 0;
	JEncodeParameter *nmp_encode_info;
	jny_service_basic *jny_basic;
	//ST_AVStreamParam st_encode_info;
	ST_AVStreamParam st_encode_info;
	jny_parm_info encode_parm;
	
	NMP_ASSERT(srv && pvalue);


	nmp_encode_info = (JEncodeParameter *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;

	memset(&st_encode_info, 0, sizeof(ST_AVStreamParam));
	encode_parm.channel = channel + 1;
	encode_parm.parm_ptr = (void *)&st_encode_info;

	ret = (*jny_basic->ro.get_config)(srv, parm_id, &encode_parm);
	if(!ret)
	{
		jny_swap_encode_info(&st_encode_info, nmp_encode_info, SWAP_PACK);
	}
	return ret;
}

int jny_set_encode_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JEncodeParameter *nmp_encode_info;
	jny_service_basic *jny_basic;
	ST_AVStreamParam st_encode_info;
	jny_parm_info encode_parm;
	
	NMP_ASSERT(srv && pvalue);

	nmp_encode_info = (JEncodeParameter *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;

	memset(&st_encode_info, 0, sizeof(ST_AVStreamParam));
	encode_parm.channel = channel + 1;
	encode_parm.parm_ptr = (void *)&st_encode_info;
	ret = (*jny_basic->ro.get_config)(srv, parm_id-1, &encode_parm);
	if(!ret)
	{
		jny_swap_encode_info(&st_encode_info, nmp_encode_info, SWAP_UNPACK);
		ret = (*jny_basic->ro.set_config)(srv, parm_id, &encode_parm);
	}
	return ret;
}



int jny_set_display_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	return 0;
}


int jny_get_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JOSDParameter *nmp_osd_info;
	jny_service_basic *jny_basic;
	ST_VideoOSDInfoParam st_osd_info;
	jny_parm_info osd_parm;
	NMP_ASSERT(srv && pvalue);

	nmp_osd_info = (JOSDParameter *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;

	memset(&st_osd_info, 0, sizeof(ST_VideoOSDInfoParam));
	osd_parm.channel = channel + 1;
	osd_parm.parm_ptr = (void *)&st_osd_info;
	ret = (*jny_basic->ro.get_config)(srv, parm_id, &osd_parm);
	if(!ret)
	{
		jny_swap_osd_info(&st_osd_info, nmp_osd_info, SWAP_PACK);
	}
	return ret;
}

int jny_set_osd_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JOSDParameter *nmp_osd_info;
	jny_service_basic *jny_basic;
	ST_VideoOSDInfoParam st_osd_info;
	jny_parm_info osd_parm;

	NMP_ASSERT(srv && pvalue);

	nmp_osd_info = (JOSDParameter *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;
	
	memset(&st_osd_info, 0, sizeof(ST_VideoOSDInfoParam));
	osd_parm.channel = channel + 1;
	osd_parm.parm_ptr = (void *)&st_osd_info;	
	ret = (*jny_basic->ro.get_config)(srv, parm_id-1, &osd_parm);
	if(!ret)
	{
		jny_swap_osd_info(&st_osd_info, nmp_osd_info, SWAP_UNPACK);
		ret = (*jny_basic->ro.set_config)(srv, parm_id, &osd_parm);
	}
	return ret;
}

int jny_get_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JPTZParameter *nmp_ptz_info;
	jny_service_basic *jny_basic;
	ST_PTZParam st_ptz_info;
	jny_parm_info ptz_parm;
	NMP_ASSERT(srv && pvalue);

	nmp_ptz_info = (JPTZParameter *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;
	
	memset(&st_ptz_info, 0, sizeof(ST_PTZParam));
	ptz_parm.channel = channel + 1;
	ptz_parm.parm_ptr = &st_ptz_info;
	ret = (*jny_basic->ro.get_config)(srv, parm_id, &ptz_parm);
	if(!ret)
	{
		jny_swap_ptz_info(&st_ptz_info, nmp_ptz_info, SWAP_PACK);
	}
	return ret;

}

int jny_set_ptz_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JPTZParameter *nmp_ptz_info;
	jny_service_basic *jny_basic;
	ST_PTZParam st_ptz_info;
	jny_parm_info ptz_parm;
	NMP_ASSERT(srv && pvalue);

	nmp_ptz_info = (JPTZParameter *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;
	
	memset(&st_ptz_info, 0, sizeof(ST_PTZParam));
	ptz_parm.channel = channel + 1;
	ptz_parm.parm_ptr = &st_ptz_info;
	ret = (*jny_basic->ro.get_config)(srv, parm_id - 1, &ptz_parm);
	if(!ret)
	{
		jny_swap_ptz_info(&st_ptz_info, nmp_ptz_info, SWAP_UNPACK);
		ret = (*jny_basic->ro.set_config)(srv, parm_id, &ptz_parm);
	}
	return ret;
}

int jny_get_record_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JRecordParameter *nmp_record_info;
	jny_service_basic *jny_basic;
	ST_RecordPolicyParam st_record_info;
	jny_parm_info record_parm;
	NMP_ASSERT(srv && pvalue);

	nmp_record_info = (JRecordParameter *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;
	
	memset(&st_record_info, 0, sizeof(ST_RecordPolicyParam));
	record_parm.channel = channel + 1;
	record_parm.parm_ptr = &st_record_info;
	ret = (*jny_basic->ro.get_config)(srv, parm_id, &record_parm);
	if(!ret)
	{
		jny_swap_record_info(&st_record_info, nmp_record_info, SWAP_PACK);
	}
	return ret;
}

int jny_set_record_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JRecordParameter *nmp_record_info;
	jny_service_basic *jny_basic;
	ST_RecordPolicyParam st_record_info;
	jny_parm_info record_parm;
	NMP_ASSERT(srv && pvalue);

	nmp_record_info = (JRecordParameter *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;
	
	memset(&st_record_info, 0, sizeof(ST_RecordPolicyParam));
	record_parm.channel = channel + 1;
	record_parm.parm_ptr = &st_record_info;
	ret = (*jny_basic->ro.get_config)(srv, parm_id-1, &record_parm);
	if(!ret)
	{
		jny_swap_record_info(&st_record_info, nmp_record_info, SWAP_UNPACK);
		ret = (*jny_basic->ro.set_config)(srv, parm_id-1, &record_parm);
	}
	return ret;
}


int jny_get_hide_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JHideParameter *nmp_hide_info;
	jny_service_basic *jny_basic;
	ST_BlindAreaParam st_hide_info;
	jny_parm_info hide_parm;
	NMP_ASSERT(srv && pvalue);

	nmp_hide_info = (JHideParameter *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;
	
	memset(&st_hide_info, 0, sizeof(ST_BlindAreaParam));
	hide_parm.channel = channel + 1;
	hide_parm.parm_ptr = &st_hide_info;
	ret = (*jny_basic->ro.get_config)(srv, parm_id, &hide_parm);
	if(!ret)
	{
		jny_swap_hide_info(&st_hide_info, nmp_hide_info, SWAP_PACK);
	}
	return ret;
}

int jny_set_hide_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JHideParameter *nmp_hide_info;
	jny_service_basic *jny_basic;
	ST_BlindAreaParam st_hide_info;
	jny_parm_info hide_parm;
	NMP_ASSERT(srv && pvalue);

	nmp_hide_info = (JHideParameter *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;
	
	memset(&st_hide_info, 0, sizeof(ST_BlindAreaParam));
	hide_parm.channel = channel + 1;
	hide_parm.parm_ptr = &st_hide_info;
	ret = (*jny_basic->ro.get_config)(srv, parm_id - 1, &hide_parm);
	if(!ret)
	{
		jny_swap_hide_info(&st_hide_info, nmp_hide_info, SWAP_UNPACK);
		ret = (*jny_basic->ro.set_config)(srv, parm_id, &hide_parm);
	}
	return ret;
}

int jny_get_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	
	int ret;
	JMoveAlarm *nmp_move_alarm_info;
	jny_service_basic *jny_basic;
	ST_MotionDetectionEventParam st_move_alarm_info;
	jny_parm_info move_alarm_parm;
	NMP_ASSERT(srv && pvalue);
	
	nmp_move_alarm_info = (JMoveAlarm *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;
	
	memset(&st_move_alarm_info, 0, sizeof(ST_MotionDetectionEventParam));
	move_alarm_parm.channel = channel + 1;
	move_alarm_parm.parm_ptr = &st_move_alarm_info;
	ret = (*jny_basic->ro.get_config)(srv, parm_id, &move_alarm_parm);
	if(!ret)
	{
		jny_swap_move_alarm_info(&st_move_alarm_info, nmp_move_alarm_info, SWAP_PACK);
	}
	return ret;

}

int jny_set_move_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	
	int ret;
	JMoveAlarm *nmp_move_alarm_info;
	jny_service_basic *jny_basic;
	ST_MotionDetectionEventParam st_move_alarm_info;
	jny_parm_info move_alarm_parm;
	NMP_ASSERT(srv && pvalue);
	
	nmp_move_alarm_info = (JMoveAlarm *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;
	
	memset(&st_move_alarm_info, 0, sizeof(ST_MotionDetectionEventParam));
	move_alarm_parm.channel = channel + 1;
	move_alarm_parm.parm_ptr = &st_move_alarm_info;
	ret = (*jny_basic->ro.get_config)(srv, parm_id-1, &move_alarm_parm);
	if(!ret)
	{
		jny_swap_move_alarm_info(&st_move_alarm_info, nmp_move_alarm_info, SWAP_UNPACK);
		ret = (*jny_basic->ro.set_config)(srv, parm_id, &move_alarm_parm);
	}
	return ret;
}


int jny_get_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue)
{

	int ret;
	JLostAlarm *nmp_video_lost_info;
	jny_service_basic *jny_basic;
	ST_VideoLoseDetectionEventParam st_video_lost_info;
	jny_parm_info video_lost_parm;
	NMP_ASSERT(srv && pvalue);
	
	nmp_video_lost_info = (JLostAlarm *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;
	
	memset(&st_video_lost_info, 0, sizeof(ST_VideoLoseDetectionEventParam));
	video_lost_parm.channel = channel + 1;
	video_lost_parm.parm_ptr = &st_video_lost_info;
	ret = (*jny_basic->ro.get_config)(srv, parm_id, &video_lost_parm);
	if(!ret)
	{
		//jny_swap_hide_info(&st_video_lost_info, nmp_video_lost_info, SWAP_PACK);
	}
	return ret;
}

int jny_set_video_lost_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JLostAlarm *nmp_video_lost_info;
	jny_service_basic *jny_basic;
	ST_VideoLoseDetectionEventParam st_video_lost_info;
	jny_parm_info video_lost_parm;
	NMP_ASSERT(srv && pvalue);
	
	nmp_video_lost_info = (JLostAlarm *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;
	
	memset(&st_video_lost_info, 0, sizeof(ST_VideoLoseDetectionEventParam));
	video_lost_parm.channel = channel + 1;
	video_lost_parm.parm_ptr = &st_video_lost_info;
	ret = (*jny_basic->ro.get_config)(srv, parm_id-1, &video_lost_parm);
	if(!ret)
	{
		//jny_swap_hide_info(&st_video_lost_info, nmp_video_lost_info, SWAP_UNPACK);
		ret = (*jny_basic->ro.set_config)(srv, parm_id, &video_lost_parm);
	}
	return ret;
}


int jny_get_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JHideAlarm *nmp_hide_alarm_info;
	jny_service_basic *jny_basic;
	ST_OcclusionDetectionEventParam st_hide_alarm_info;
	jny_parm_info hide_alarm_parm;
	NMP_ASSERT(srv && pvalue);
	
	nmp_hide_alarm_info = (JHideAlarm *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;
	
	memset(&st_hide_alarm_info, 0, sizeof(ST_OcclusionDetectionEventParam));
	hide_alarm_parm.channel = channel + 1;
	hide_alarm_parm.parm_ptr = &st_hide_alarm_info;
	ret = (*jny_basic->ro.get_config)(srv, parm_id, &hide_alarm_parm);
	if(!ret)
	{
		jny_swap_hide_alarm_info(&st_hide_alarm_info, nmp_hide_alarm_info, SWAP_PACK);
	}
	return ret;
}

int jny_set_hide_alarm_info(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	JHideAlarm *nmp_hide_alarm_info;
	jny_service_basic *jny_basic;
	ST_OcclusionDetectionEventParam st_hide_alarm_info;
	jny_parm_info hide_alarm_parm;
	NMP_ASSERT(srv && pvalue);
	
	nmp_hide_alarm_info = (JHideAlarm *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;
	
	memset(&st_hide_alarm_info, 0, sizeof(ST_OcclusionDetectionEventParam));
	hide_alarm_parm.channel = channel + 1;
	hide_alarm_parm.parm_ptr = &st_hide_alarm_info;
	ret = (*jny_basic->ro.get_config)(srv, parm_id - 1, &hide_alarm_parm);
	if(!ret)
	{
		jny_swap_hide_alarm_info(&st_hide_alarm_info, nmp_hide_alarm_info, SWAP_UNPACK);
		ret = (*jny_basic->ro.set_config)(srv, parm_id, &hide_alarm_parm);
	}
	return ret;
}


int jny_get_io_alarm_info(struct service *srv, int channel, int type, void *pvalue)
{
#if 0
	int ret;
	JIoAlarm *nmp_io_alarm_info;
	jny_service_basic *jny_basic;
	ST_AlarmIOEventParam st_io_alarm_info;
	jny_parm_info io_alarm_parm;
	NMP_ASSERT(srv && pvalue);
	
	nmp_io_alarm_info = (JIoAlarm *)pvalue;
	jny_basic = (jny_service_basic *)srv->tm;
	
	memset(&st_io_alarm_info, 0, sizeof(ST_AlarmIOEventParam));
	switch(type)
	{
		case ALARM_IN:
			io_alarm_parm.channel = channel + 1;
			io_alarm_parm.parm_ptr = &st_io_alarm_info;
			ret = (*jny_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &io_alarm_parm);
			if(!ret)
			{
				//jny_swap_move_alarm_info(&st_io_alarm_info, nmp_io_alarm_info, SWAP_PACK);
			}
			break;
			
		case ALARM_OUT:
			io_alarm_parm.channel = channel + 1;
			io_alarm_parm.parm_ptr = &st_io_alarm_info;
			ret = (*jny_basic->ro.get_config)(srv, GET_IO_ALARM_CONFIG, &io_alarm_parm);
			if(!ret)
			{
				//jny_swap_io_alarm_info(&st_io_alarm_info, nmp_io_alarm_info, SWAP_PACK);
			}
			break;
			
	}
	return ret;
#endif
	return 0;
}

int jny_set_io_alarm_info(struct service *srv, int channel, int type, void *pvalue)
{
	return 0;
}

int jny_get_store_log(struct service *srv, int channel, int parm_id, void *pvalue)
{
	return 0;
}


int jny_control_ptz(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret;
	jny_service_basic *jny_basic;
	jny_parm_info control_ptz_parm;

	NMP_ASSERT(srv && pvalue);

	jny_basic = (jny_service_basic *)srv->tm;
	
	control_ptz_parm.channel = channel+1;
	control_ptz_parm.parm_ptr = pvalue;

	ret = (*jny_basic->ro.set_config)(srv, parm_id, &control_ptz_parm);

	return 0;
}

int jny_set_preset_point(struct service *srv, int channel, int parm_id, void *pvalue)
{
    int ret = 0;
    //JPPConfig *jny_pp_cfg;
    jny_parm_info set_preset_point_parm;
    jny_service_basic *jny_basic;

    NMP_ASSERT(srv && pvalue);

    //jny_pp_cfg= (JPPConfig*)pvalue;
    jny_basic = (jny_service_basic*)srv->tm;
	
	set_preset_point_parm.channel = channel+1;
	set_preset_point_parm.parm_ptr = pvalue;

    ret = (*jny_basic->ro.set_config)(srv, parm_id, &set_preset_point_parm);

    return ret;
}

int jny_get_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
/*
	int ret;
	JCruiseWay *nmp_cruise_info;

*/	return 0;
}

int jny_set_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
	return 0;
}

int jny_add_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
	int ret = 0;

    jny_parm_info cruise_parm;
    jny_service_basic *jny_basic;

    NMP_ASSERT(srv && pvalue);

    jny_basic = (jny_service_basic*)srv->tm;
	
	cruise_parm.channel = channel+1;
	cruise_parm.parm_ptr = pvalue;

    ret = (*jny_basic->ro.set_config)(srv, parm_id, &cruise_parm);

    return ret;
}

int jny_modify_cruise_way(struct service *srv, int channel, int parm_id, void *pvalue)
{
	return 0;
}



int jny_get_capability_list(struct service *srv, int channel, int parm_id, void *pvalue)
{
    //JDevCap *dev_cap;
    jny_service_basic *jny_basic;

    NMP_ASSERT(srv && pvalue);

    //dev_cap = (JDevCap*)pvalue;
    jny_basic = (jny_service_basic*)srv->tm;

    return (*jny_basic->ro.get_config)(srv, parm_id, pvalue);
}




