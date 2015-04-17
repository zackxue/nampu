#include "nmp_jny_swap.h"

#define MAX_WIDTH	704
#define MAX_HEIGHT	576

static int jny_swap_devicetype(int devicetype)
{
	int type = -1;
	switch(devicetype)
	{
		case 1:
			type = J_SDK_IPC;
			break;
		case 2:
			type = J_SDK_DVR;
			break;
		case 3:
			type = J_SDK_DVS;
			break;
		case 4:
			break;
		case 5:
			type = J_SDK_NVR;
			break;
		default:
			break;
	}
	return type;
}

void jny_swap_device_info(ST_DeviceSummaryParam *dev_cfg, JDeviceInfo *dev_info, int flag)
{
    NMP_ASSERT(dev_cfg && dev_info);
    
    switch (flag)
    {
        case SWAP_PACK:
			memcpy(dev_info->hw_version, dev_cfg->szSoftwareVer, sizeof(dev_cfg->szSoftwareVer) -1);
			memcpy(dev_info->dev_version, dev_cfg->szHardwareVer, sizeof(dev_cfg->szHardwareVer) -1);
			memcpy(dev_info->release_date, dev_cfg->szProductionTime, sizeof(dev_cfg->szProductionTime) - 1);

			dev_info->pu_type      = (JPuType)jny_swap_devicetype(dev_cfg->nDeviceType);
			dev_info->di_num       = (int)dev_cfg->nAlarmInNum;
			dev_info->do_num       = (int)dev_cfg->nAlarmOutNum;
			dev_info->channel_num  = (int)dev_cfg->nCameraNum;
			dev_info->rs485_num    = (int)dev_cfg->nRS485Num;
			dev_info->rs232_num    = (int)dev_cfg->nRS232Num;
            break;
            
        case SWAP_UNPACK:
/*
            
            dev_cfg->byDVRType         = dev_info->pu_type;
            dev_cfg->byAlarmInPortNum  = dev_info->di_num;
            dev_cfg->byAlarmOutPortNum = dev_info->do_num;
            dev_cfg->byChanNum         = dev_info->channel_num;
            dev_cfg->byRS485Num        = dev_info->rs485_num;
            dev_cfg->byRS232Num        = dev_info->rs232_num;
*/
            break;
            
        default:
            break;
    }
    
    printf("dwSoftwareVersion  : %s  |  ", dev_cfg->szSoftwareVer);
    printf("dev_version : %s\n", dev_info->dev_version);
    printf("dwHardwareVersion  : %s  |  ", dev_cfg->szHardwareVer);
    printf("hw_version  : %s\n", dev_info->hw_version);
    printf("dwSoftwareBuildDate: %s  |  ", dev_cfg->szProductionTime);
    printf("release_date: %s\n", dev_info->release_date);

    printf("byDVRType        : %02d  |  ", (int)dev_cfg->nDeviceType);
    printf("pu_type  : %d\n", dev_info->pu_type);
    printf("byAlarmInPortNum : %02d  |  ", (int)dev_cfg->nAlarmInNum);
    printf("di_num   : %d\n", dev_info->di_num);
    printf("byAlarmOutPortNum: %02d  |  ", (int)dev_cfg->nAlarmOutNum);
    printf("do_num   : %d\n", dev_info->do_num);
    printf("nCameraNum        : %02d  |  ", (int)dev_cfg->nCameraNum);
    printf("chan_num : %d\n", dev_info->channel_num);
    printf("byRS485Num       : %02d  |  ", (int)dev_cfg->nRS485Num);
    printf("rs485_num: %d\n", dev_info->rs485_num);
    printf("byRS232Num       : %02d  |  ", (int)dev_cfg->nRS232Num);
    printf("rs232_num: %d\n", dev_info->rs232_num);

    return ;
}

void jny_swap_rs485_info(ST_RS485Param *rs485_cfg, JSerialParameter *serial_info, int flag)
{
    NMP_ASSERT(rs485_cfg && serial_info);
    
    switch (flag)
    {
	    case SWAP_PACK:
			serial_info->baud_rate 	= rs485_cfg->nBaudRate;
			serial_info->data_bit 	= rs485_cfg->nDataBits;
			serial_info->stop_bit 	= rs485_cfg->nStopBits;
			serial_info->verify    	= rs485_cfg->nParity;
	        break;
	        
	    case SWAP_UNPACK:

	        break;
	        
	    default:
	        break;
    }
    
    return ;
}

void jny_swap_device_time_info(jny_device_time *dev_time, JDeviceTime *nmp_dev_time, int flag)
{
	NMP_ASSERT(dev_time && nmp_dev_time);

	switch(flag)
	{
		case SWAP_PACK:
			nmp_dev_time->time.year 	= dev_time->nYearGet - 1900;
			nmp_dev_time->time.month 	= dev_time->nMonthGet;
			nmp_dev_time->time.date 	= dev_time->nDayGet;
			nmp_dev_time->time.hour 	= dev_time->nHourGet;
			nmp_dev_time->time.minute = dev_time->nMinuteGet;
			nmp_dev_time->time.second = dev_time->nMinuteGet;
			nmp_dev_time->zone 		= DEF_PROXY_TIME_ZOE;
			break;

		case SWAP_UNPACK:
            dev_time->nYearGet   = nmp_dev_time->time.year + 1900;
            dev_time->nMonthGet  = nmp_dev_time->time.month;
            dev_time->nDayGet    = nmp_dev_time->time.date;
            dev_time->nHourGet   = nmp_dev_time->time.hour;
            dev_time->nMinuteGet = nmp_dev_time->time.minute;
            dev_time->nMinuteGet = nmp_dev_time->time.second;
			break;

		default:
			break;
	}
	return;
}


void jny_swap_ntp_info(ST_NTPParam *st_ntp_info, JDeviceNTPInfo *nmp_ntp_info, int flag)
{
	NMP_ASSERT(st_ntp_info && nmp_ntp_info);

	switch(flag)
	{
		case SWAP_PACK:
			memcpy(nmp_ntp_info->ntp_server_ip, st_ntp_info->szNTPIp, strlen(st_ntp_info->szNTPIp));
			
			nmp_ntp_info->time_zone 	= 8;
			nmp_ntp_info->ntp_enable 	= st_ntp_info->bNTPEnableFlag;
			break;
			
		case SWAP_UNPACK:
			memcpy(st_ntp_info->szNTPIp, (char *)nmp_ntp_info->ntp_server_ip, 
			  		strlen((char *)nmp_ntp_info->ntp_server_ip));
			
			st_ntp_info->nIPProtoVer 	= IPPROTO_V4;
			st_ntp_info->bNTPEnableFlag = nmp_ntp_info->ntp_enable;
			st_ntp_info->nNTPPort 		= 123;		//默认ntp端口
			break;
	}
}

void jny_swap_network_info(ST_HostNetworkParam *st_network_info, 
									JNetworkInfo *nmp_network_info, int flag)
{

	NMP_ASSERT(st_network_info && nmp_network_info);

	switch(flag)
	{
		case SWAP_PACK:
	        strncpy((char*)nmp_network_info->network[J_SDK_ETH0].ip, st_network_info->szLocalIp, 
                	strlen(st_network_info->szLocalIp));
			
            strncpy((char*)nmp_network_info->network[J_SDK_ETH0].netmask, st_network_info->szLocalSubnetMask, 
                	strlen(st_network_info->szLocalSubnetMask));
			
            strncpy((char*)nmp_network_info->network[J_SDK_ETH0].gateway, st_network_info->szGateway, 
                	strlen(st_network_info->szGateway));

            strncpy((char*)nmp_network_info->main_dns, st_network_info->szPrimaryDNSIp, 
                	strlen(st_network_info->szPrimaryDNSIp));
			
            strncpy((char*)nmp_network_info->backup_dns, st_network_info->szSpareDNSIp, 
                	strlen(st_network_info->szSpareDNSIp));

            nmp_network_info->network[J_SDK_ETH0].type = J_SDK_ETH0;
            nmp_network_info->network[J_SDK_ETH0].dhcp_enable = st_network_info->bAutoGetIpFlag;

            nmp_network_info->web_port = st_network_info->nHttpPort;
            nmp_network_info->cmd_port = st_network_info->nControlPort;
			
			break;
			
		case SWAP_UNPACK:
			strncpy(st_network_info->szLocalIp, (char*)nmp_network_info->network[J_SDK_ETH0].ip,
					strlen((char*)nmp_network_info->network[J_SDK_ETH0].ip));

			strncpy(st_network_info->szLocalSubnetMask, (char*)nmp_network_info->network[J_SDK_ETH0].netmask,
					strlen((char*)nmp_network_info->network[J_SDK_ETH0].netmask));

			strncpy(st_network_info->szGateway, (char*)nmp_network_info->network[J_SDK_ETH0].gateway,
					strlen((char*)nmp_network_info->network[J_SDK_ETH0].gateway));

			strncpy(st_network_info->szPrimaryDNSIp, (char*)nmp_network_info->main_dns,
					strlen((char*)nmp_network_info->main_dns));

			strncpy(st_network_info->szSpareDNSIp, (char*)nmp_network_info->backup_dns,
					strlen((char*)nmp_network_info->backup_dns));

			st_network_info->bAutoGetIpFlag = nmp_network_info->network[J_SDK_ETH0].dhcp_enable;
			st_network_info->nHttpPort = nmp_network_info->web_port;
			st_network_info->nIPProtoVer = IPPROTO_V4;
			st_network_info->nControlPort = nmp_network_info->cmd_port;
			break;
	}
}

void jny_swap_pppoe_info(ST_PPPoEParam *st_pppoe_info, JPPPOEInfo *nmp_pppoe_info, int flag)
{
	NMP_ASSERT(st_pppoe_info && nmp_pppoe_info);

	switch(flag)
	{
		case SWAP_PACK:
			nmp_pppoe_info->enable = st_pppoe_info->bPPPoEEnableFlag;
			strncpy((char *)nmp_pppoe_info->account, (char *)st_pppoe_info->szPPPoEUserName, 
							strlen((char *)st_pppoe_info->szPPPoEUserName));
			strncpy((char *)nmp_pppoe_info->passwd, (char *)st_pppoe_info->szPPPoEPassword, 
							strlen((char *)st_pppoe_info->szPPPoEPassword));
			break;
			
		case SWAP_UNPACK:
			st_pppoe_info->bPPPoEEnableFlag = nmp_pppoe_info->enable;
			strncpy((char *)st_pppoe_info->szPPPoEUserName, (char *)nmp_pppoe_info->account, 
							strlen((char *)nmp_pppoe_info->account));
			strncpy((char *)st_pppoe_info->szPPPoEPassword, (char *)nmp_pppoe_info->passwd, 
							strlen((char *)nmp_pppoe_info->passwd));
			break;
	}
}

void jny_swap_ftp_info(ST_FTPParam *st_ftp_info, JFTPParameter *nmp_ftp_info, int flag)
{
	NMP_ASSERT(st_ftp_info && nmp_ftp_info);

	switch(flag)
	{
		case SWAP_PACK:
			strncpy((char *)nmp_ftp_info->ftp_ip, (char *)st_ftp_info->szFTPServerAddr, 
						strlen((char *)st_ftp_info->szFTPServerAddr));
			strncpy((char *)nmp_ftp_info->ftp_usr, (char *)st_ftp_info->szFTPUserName, 
						strlen((char *)st_ftp_info->szFTPUserName));
			strncpy((char *)nmp_ftp_info->ftp_pwd, (char *)st_ftp_info->szFTPPassword, 
						strlen((char *)st_ftp_info->szFTPPassword));
			strncpy((char *)nmp_ftp_info->ftp_path, (char *)st_ftp_info->szFTPWorkPath, 
						strlen((char *)st_ftp_info->szFTPWorkPath));
			break;
			
		case SWAP_UNPACK:
			strncpy((char *)st_ftp_info->szFTPServerAddr, (char *)nmp_ftp_info->ftp_ip, 
							strlen((char *)nmp_ftp_info->ftp_ip));
			strncpy((char *)st_ftp_info->szFTPUserName, (char *)nmp_ftp_info->ftp_usr, 
							strlen((char *)nmp_ftp_info->ftp_usr));
			strncpy((char *)st_ftp_info->szFTPPassword, (char *)nmp_ftp_info->ftp_pwd, 
							strlen((char *)nmp_ftp_info->ftp_pwd));
			strncpy((char *)st_ftp_info->szFTPWorkPath, (char *)nmp_ftp_info->ftp_path, 
							strlen((char *)nmp_ftp_info->ftp_path));
			
			st_ftp_info->nIPProtoVer = IPPROTO_V4;
			break;
	}
}

void jny_swap_ddns_info(ST_DDNSParam *st_ddns_info, JDdnsConfig *nmp_ddns_info, int flag)
{
	NMP_ASSERT(st_ddns_info && nmp_ddns_info);

	switch(flag)
	{
		case SWAP_PACK:
			strncpy((char *)nmp_ddns_info->ddns_account, (char *)st_ddns_info->szDDNSDomainName, 
					strlen((char *)st_ddns_info->szDDNSAccounts));
			strncpy((char *)nmp_ddns_info->ddns_usr, (char *)st_ddns_info->szDDNSAccounts,
						strlen((char *)st_ddns_info->szDDNSAccounts));

			strncpy((char *)nmp_ddns_info->ddns_pwd, (char *)st_ddns_info->szDDNSPassword, 
						strlen((char *)st_ddns_info->szDDNSPassword));
			break;
			
		case SWAP_UNPACK:
			strncpy((char *)st_ddns_info->szDDNSDomainName, (char *)nmp_ddns_info->ddns_account,  
						strlen((char *)nmp_ddns_info->ddns_account));
			strncpy((char *)st_ddns_info->szDDNSAccounts, (char *)nmp_ddns_info->ddns_usr, 
						strlen((char *)nmp_ddns_info->ddns_usr));

			strncpy((char *)st_ddns_info->szDDNSPassword, (char *)nmp_ddns_info->ddns_pwd,
						strlen((char *)nmp_ddns_info->ddns_pwd));

			st_ddns_info->bDDNSEnableFlag = 1;
			break;
	}
}

void jny_swap_smtp_info(ST_SMTPParam *st_smtp_info, JSMTPParameter *nmp_smtp_info, int flag)
{
	NMP_ASSERT(st_smtp_info && nmp_smtp_info);

	switch(flag)
	{
		case SWAP_PACK:
			strncpy((char *)nmp_smtp_info->mail_ip, (char *)st_smtp_info->szSMTPServerAddr, 
						strlen((char *)st_smtp_info->szSMTPServerAddr));

			strncpy((char *)nmp_smtp_info->mail_addr, (char *)st_smtp_info->szSenderEmailAddress, 
						strlen((char *)st_smtp_info->szSenderEmailAddress));

			strncpy((char *)nmp_smtp_info->mail_usr, (char *)st_smtp_info->szSMTPUserName,
						strlen((char *)st_smtp_info->szSMTPUserName));

			strncpy((char *)nmp_smtp_info->mail_pwd, (char *)st_smtp_info->szSMTPPassword, 
						strlen((char *)st_smtp_info->szSMTPPassword));

			strncpy((char *)nmp_smtp_info->mail_rctp1, (char *)st_smtp_info->szRecipientEmailAddressList[0],
						strlen((char *)st_smtp_info->szRecipientEmailAddressList[0]));
			strncpy((char *)nmp_smtp_info->mail_rctp2, (char *)st_smtp_info->szRecipientEmailAddressList[1],
						strlen((char *)st_smtp_info->szRecipientEmailAddressList[1]));
			strncpy((char *)nmp_smtp_info->mail_rctp3, (char *)st_smtp_info->szRecipientEmailAddressList[2],
						strlen((char *)st_smtp_info->szRecipientEmailAddressList[2]));

			nmp_smtp_info->mail_port = st_smtp_info->nSMTPServerPort;
			
			break;
			
		case SWAP_UNPACK:
			strncpy((char *)st_smtp_info->szSMTPServerAddr, (char *)nmp_smtp_info->mail_ip,  
					strlen((char *)nmp_smtp_info->mail_ip));

			strncpy((char *)st_smtp_info->szSenderEmailAddress, (char *)nmp_smtp_info->mail_addr,  
					strlen((char *)nmp_smtp_info->mail_addr));

			strncpy((char *)st_smtp_info->szSMTPUserName, (char *)nmp_smtp_info->mail_usr, 
					strlen((char *)nmp_smtp_info->mail_usr));

			strncpy((char *)st_smtp_info->szSMTPPassword, (char *)nmp_smtp_info->mail_pwd, 
					strlen((char *)nmp_smtp_info->mail_pwd));

			strncpy((char *)st_smtp_info->szRecipientEmailAddressList[0], (char *)nmp_smtp_info->mail_rctp1, 
					strlen((char *)nmp_smtp_info->mail_rctp1));
			strncpy((char *)st_smtp_info->szRecipientEmailAddressList[1], (char *)nmp_smtp_info->mail_rctp2,
					strlen((char *)nmp_smtp_info->mail_rctp2));
			strncpy((char *)st_smtp_info->szRecipientEmailAddressList[2], (char *)nmp_smtp_info->mail_rctp3, 
					strlen((char *)nmp_smtp_info->mail_rctp3));

			st_smtp_info->nSMTPServerPort = nmp_smtp_info->mail_port;
			st_smtp_info->bSMTPEnableFlag = 1;

			break;
	}
}

void jny_swap_disk_info(ST_AllDiskStatistic *st_disk_info, JDeviceDiskInfo *nmp_disk_info, int flag)
{
	int i;
	NMP_ASSERT(st_disk_info && nmp_disk_info);

	switch(flag)
	{
		case SWAP_PACK:
			nmp_disk_info->disk_num = st_disk_info->nDiskNum;
			for(i = 0; i < st_disk_info->nDiskNum; i++)
			{
				nmp_disk_info->disk[i].disk_no = st_disk_info->stDiskStatisticList[i].nDiskId;
				nmp_disk_info->disk[i].total_size = st_disk_info->stDiskStatisticList[i].nDiskTotalSize;
				nmp_disk_info->disk[i].free_size = st_disk_info->stDiskStatisticList[i].nDiskFreeSize;
				switch(st_disk_info->stDiskStatisticList[i].btDiskStatus)
				{
					case DISKSTATUS_DORMANCY:
					case DISKSTATUS_WRITE_PROTECT:
						nmp_disk_info->disk[i].status = J_SDK_USING;
						break;
						
					case DISKSTATUS_FORMATTING:
					case DISKSTATUS_NOT_FORMAT:
						nmp_disk_info->disk[i].status = J_SDK_MOUNTED;
						break;
					case DISKSTATUS_NOT_EXISTENT:
					case DISKSTATUS_ABNORMAL:
						nmp_disk_info->disk[i].status = J_SDK_UNMOUNT;
						break;		
				}
			}
			break;
			
		case SWAP_UNPACK:
			break;
	}
}


void jny_swap_osd_info(ST_VideoOSDInfoParam *st_osd_info, JOSDParameter *nmp_osd_info, int flag)
{
	int i;
	NMP_ASSERT(st_osd_info && nmp_osd_info);

	switch(flag)
	{
		case SWAP_PACK:
			
			for(i = 0; i < st_osd_info->nOSDInfoNum; i++)
			{
				printf("-------st_osd_info->nOSDInfoNum = %d, st_osd_info->stOSDInfoParam[i].nOSDType = %d\n", 
							(int)st_osd_info->nOSDInfoNum,
							(int)st_osd_info->stOSDInfoParam[i].nOSDType);
				switch(st_osd_info->stOSDInfoParam[i].nOSDType)
				{
					//case OSDTYPE_DEVICENAME:
					//case OSDTYPE_CAMERAID:
					//case OSDTYPE_CAMERANAME:
					case OSDTYPE_TEXT:
						nmp_osd_info->text_enable    = st_osd_info->stOSDInfoParam[i].bOSDEnableFlag;
						nmp_osd_info->text_display_x = st_osd_info->stOSDInfoParam[i].nTopX;
						nmp_osd_info->text_display_y = st_osd_info->stOSDInfoParam[i].nTopY;

						strncpy((char *)nmp_osd_info->text_data, (char *)st_osd_info->stOSDInfoParam[i].szInfo,
									strlen((char *)st_osd_info->stOSDInfoParam[i].szInfo));
						break;
						
					case OSDTYPE_TIME:
						nmp_osd_info->time_enable    = st_osd_info->stOSDInfoParam[i].bOSDEnableFlag;
		            	nmp_osd_info->time_display_x = st_osd_info->stOSDInfoParam[i].nTopX;
		            	nmp_osd_info->time_display_y = st_osd_info->stOSDInfoParam[i].nTopY;
						break;
				}
			}
			nmp_osd_info->max_width      = 704;
            nmp_osd_info->max_height     = 576;
            
            //out_buf = (char*)osd_para->text_data;
            //out_len = sizeof(osd_para->text_data)-1;
			/*
            		code_convert((char*)"gb2312", (char*)"utf-8", 
                    (char*)pic_cfg->sChanName, 
                    strlen((const char*)pic_cfg->sChanName), 
                    out_buf, out_len);
                    */
			break;		
			
		case SWAP_UNPACK:
			st_osd_info->stOSDInfoParam[0].bOSDEnableFlag 	= nmp_osd_info->text_enable;						
			st_osd_info->stOSDInfoParam[0].nTopX 			= nmp_osd_info->text_display_x*100/704;		
			st_osd_info->stOSDInfoParam[0].nTopY 			= nmp_osd_info->text_display_y*100/576;
			st_osd_info->stOSDInfoParam[0].nOSDType 		= OSDTYPE_TEXT;
			
			strncpy((char *)st_osd_info->stOSDInfoParam[0].szInfo, (char *)nmp_osd_info->text_data,
												strlen((char *)nmp_osd_info->text_data));

			st_osd_info->stOSDInfoParam[1].bOSDEnableFlag 	= nmp_osd_info->time_enable;
        	st_osd_info->stOSDInfoParam[1].nTopX			= nmp_osd_info->time_display_x*100/704; 
        	st_osd_info->stOSDInfoParam[1].nTopY			= nmp_osd_info->time_display_y*100/576;
			st_osd_info->stOSDInfoParam[1].nOSDType 		= OSDTYPE_TIME;
			st_osd_info->nOSDInfoNum = 2;
			break;
	}
}

void jny_swap_encode_info(ST_AVStreamParam *st_encode_info, JEncodeParameter *nmp_encode_info, int flag)
{
	NMP_ASSERT(st_encode_info && nmp_encode_info);

	switch(flag)
	{
		case SWAP_PACK:
            nmp_encode_info->frame_rate = st_encode_info->nFrameRate;          
            nmp_encode_info->i_frame_interval = st_encode_info->nIFrameInterval;


			switch (st_encode_info->nVideoEncoderType)
            {
                case 0xff:			//无效
                    break;
					
                case H264:			//标准h264
                    nmp_encode_info->video_type = J_SDK_AV_VIDEO_H264;
                    break;
					
                case MPEG4:			//标准mpeg4
                    nmp_encode_info->video_type = J_SDK_AV_VIDEO_MPEG4;
                    break;
				
                case MJPEG://M-JPEG
                    nmp_encode_info->video_type = J_SDK_AV_VIDEO_MJPEG;
                    break;
            }

            switch (st_encode_info->nAudioEncoderType)
            {
                case 0xff:				//无效
                    nmp_encode_info->audio_enble = 0;
                    break;
					
                case G711_ULAW:			//G711_U
                    nmp_encode_info->audio_enble = 1;
                    nmp_encode_info->audio_type = J_SDK_AV_AUDIO_G711U;
                    break;
					
                case G711_ALAW:			//G711_A
                    nmp_encode_info->audio_enble = 1;
                    nmp_encode_info->audio_type = J_SDK_AV_AUDIO_G711A;
                    break;
            }

		/*
            switch (st_encode_info->stVideoVideoEncodeQuality.nImageFormatId)
            {
				case IMAGEFORMAT_D1:
					nmp_encode_info->resolution = J_SDK_VIDEO_D1;
					break;
					
				case IMAGEFORMAT_LOW_BITRATE_D1:

					break;

				case IMAGEFORMAT_CIF:
					nmp_encode_info->resolution = J_SDK_VIDEO_CIF;
					break;
					
				case IMAGEFORMAT_QCIF:
					nmp_encode_info->resolution = J_SDK_VIDEO_QCIF;
					break;
					
				case IMAGEFORMAT_SXGA:
					break;
				case IMAGEFORMAT_QVGA:
					nmp_encode_info->resolution = J_SDK_VIDEO_QVGA;
					break;
					
				case IMAGEFORMAT_1280_720:
					nmp_encode_info->resolution = J_SDK_VIDEO_720P;
					break;
					
				case IMAGEFORMAT_360_160:
					break;
					
				case IMAGEFORMAT_640_360:
					break;
					
				case IMAGEFORMAT_VGA:
					nmp_encode_info->resolution = J_SDK_VIDEO_VGA;
					break;
					
				case IMAGEFORMAT_UXGA:
					nmp_encode_info->resolution = J_SDK_VIDEO_UXGA;
					break;
					
				case IMAGEFORMAT_1920_1080:
					nmp_encode_info->resolution = J_SDK_VIDEO_1080P;
					break;
					
				case IMAGEFORMAT_640_360_EX:
					break;
				default :
					break;
			}
            */
            nmp_encode_info->qp_value   = st_encode_info->nQuality;
            
            nmp_encode_info->code_rate = st_encode_info->nBitRate;
			
			if(st_encode_info->nBitRateType == CBR_TYPE)
				nmp_encode_info->bit_rate = J_SDK_CBR;
			else if(st_encode_info->nBitRateType == VBR_TYPE)
				nmp_encode_info->bit_rate = J_SDK_VBR;
			
			break;
		
		case SWAP_UNPACK:
            st_encode_info->nFrameRate = nmp_encode_info->frame_rate;          
            st_encode_info->nIFrameInterval = nmp_encode_info->i_frame_interval;
            
            switch (nmp_encode_info->video_type)
            {
                case 0xff:			//无效
                    break;

                case J_SDK_AV_VIDEO_H264:		//标准h264
                    st_encode_info->nVideoEncoderType = H264;
                    break;
					
                case J_SDK_AV_VIDEO_MPEG4:		//标准mpeg4
                    st_encode_info->nVideoEncoderType = MPEG4;
                    break;
					
                case J_SDK_AV_VIDEO_MJPEG:			//M-JPEG
                    st_encode_info->nVideoEncoderType = MJPEG;
                    break;
            }

            switch (nmp_encode_info->audio_enble)
            {
                case 0xff://无效
                    break;
					
                case J_SDK_AV_AUDIO_G711U://G711_U
                    //nmp_encode_info->audio_enble = 1;
                    st_encode_info->nAudioEncoderType = G711_ULAW;
                    break;
					
                case J_SDK_AV_AUDIO_G711A://G711_A
                    //nmp_encode_info->audio_enble = 1;
                    st_encode_info->nAudioEncoderType = G711_ALAW;
                    break;
            }
            
            st_encode_info->nQuality = nmp_encode_info->qp_value;
            
            st_encode_info->nBitRate = nmp_encode_info->code_rate;
			
			if(nmp_encode_info->bit_rate == J_SDK_CBR)
				st_encode_info->nBitRateType = CBR_TYPE;
			else if(nmp_encode_info->bit_rate == J_SDK_VBR)
				st_encode_info->nBitRateType = VBR_TYPE;
	/*			
			switch (nmp_encode_info->resolution)
            {
				case J_SDK_VIDEO_D1:
					st_encode_info->stVideoVideoEncodeQuality.nImageFormatId = IMAGEFORMAT_D1;
					break;

				case J_SDK_VIDEO_CIF:
					st_encode_info->stVideoVideoEncodeQuality.nImageFormatId = IMAGEFORMAT_CIF;
					break;
					
				case J_SDK_VIDEO_QCIF:
					st_encode_info->stVideoVideoEncodeQuality.nImageFormatId =IMAGEFORMAT_QCIF;
					break;
					
				case J_SDK_VIDEO_QVGA:
					st_encode_info->stVideoVideoEncodeQuality.nImageFormatId = IMAGEFORMAT_QVGA;
					break;
					
				case J_SDK_VIDEO_720P:
					st_encode_info->stVideoVideoEncodeQuality.nImageFormatId = IMAGEFORMAT_1280_720;
					break;
					
				case J_SDK_VIDEO_VGA:
					st_encode_info->stVideoVideoEncodeQuality.nImageFormatId = IMAGEFORMAT_VGA;
					break;
					
				case J_SDK_VIDEO_UXGA:
					st_encode_info->stVideoVideoEncodeQuality.nImageFormatId = IMAGEFORMAT_UXGA;
					break;
					
				case J_SDK_VIDEO_1080P:
					st_encode_info->stVideoVideoEncodeQuality.nImageFormatId = IMAGEFORMAT_1920_1080;
					break;
				default :
					break;
			}
	*/		
			break;
	}
}

void jny_swap_ptz_info(ST_PTZParam *st_ptz_info, JPTZParameter *nmp_ptz_info, int flag)
{
	NMP_ASSERT(st_ptz_info && nmp_ptz_info);

	switch(flag)
	{
		case SWAP_PACK:
			nmp_ptz_info->serial_no 	= st_ptz_info->nComId;
			nmp_ptz_info->ptz_addr 	= st_ptz_info->nPTZDeviceId;
			nmp_ptz_info->protocol 	= st_ptz_info->nPTZProtocol;
			nmp_ptz_info->baud_rate 	= st_ptz_info->nBaudRate;
			nmp_ptz_info->data_bit 	= st_ptz_info->nDataBits;
			nmp_ptz_info->stop_bit 	= st_ptz_info->nStopBits;
			nmp_ptz_info->verify 		= st_ptz_info->nParity;
			printf("serial_no = %d, ptz_addr = %d, protocol = %d, baud_rate = %d, data_bit = %d, stop_bit = %d, verify = %d\n",
				nmp_ptz_info->serial_no, nmp_ptz_info->ptz_addr, nmp_ptz_info->protocol,nmp_ptz_info->baud_rate,
				nmp_ptz_info->data_bit, nmp_ptz_info->stop_bit,nmp_ptz_info->verify);
			break;
			
		case SWAP_UNPACK:
			st_ptz_info->bPTZEnableFlag = 1;
			st_ptz_info->nComId 		= nmp_ptz_info->serial_no;
			st_ptz_info->nPTZDeviceId 	= nmp_ptz_info->ptz_addr;
			st_ptz_info->nPTZProtocol 	= nmp_ptz_info->protocol;
			st_ptz_info->nBaudRate 		= nmp_ptz_info->baud_rate;
			st_ptz_info->nDataBits 		= nmp_ptz_info->data_bit;
			st_ptz_info->nStopBits 		= nmp_ptz_info->stop_bit;
			st_ptz_info->nParity 		= nmp_ptz_info->verify;
			break;
	}
}

void jny_swap_record_info(ST_RecordPolicyParam *st_record_info, JRecordParameter *nmp_record_info, int flag)
{
	int days, seg_time_num, seg_time_count;
	time_t start_time, end_time;
	struct tm *timep;
	struct tm st_timep;
	NMP_ASSERT(st_record_info && nmp_record_info);

	switch(flag)
	{
		case SWAP_PACK:
			for(days = 0; days < 7; days++)
			{
				seg_time_count = st_record_info->stScheduleRecordPolicy.stScheduleTimeParam.stScheduleWeekList[days].nScheduleTimeNum;
				for(seg_time_num = 0; seg_time_num < seg_time_count; seg_time_num++)
				{
					nmp_record_info->week.days[days].seg[seg_time_num].enable = 1;
					start_time = (time_t)st_record_info->stScheduleRecordPolicy.stScheduleTimeParam.stScheduleWeekList[days].stScheduleTime[seg_time_num].nStartTime;
					end_time = (time_t)st_record_info->stScheduleRecordPolicy.stScheduleTimeParam.stScheduleWeekList[days].stScheduleTime[seg_time_num].nEndTime;

					timep = gmtime(&start_time);
					
					nmp_record_info->week.days[days].seg[seg_time_num].time_start.year = timep->tm_year;
					nmp_record_info->week.days[days].seg[seg_time_num].time_start.month = timep->tm_mon;
					nmp_record_info->week.days[days].seg[seg_time_num].time_start.date = timep->tm_mday ;
					nmp_record_info->week.days[days].seg[seg_time_num].time_start.hour = timep->tm_hour;
					nmp_record_info->week.days[days].seg[seg_time_num].time_start.minute = timep->tm_min; 
					nmp_record_info->week.days[days].seg[seg_time_num].time_start.second = timep->tm_sec;

					timep = gmtime(&end_time);
					nmp_record_info->week.days[days].seg[seg_time_num].time_end.year = timep->tm_year;
					nmp_record_info->week.days[days].seg[seg_time_num].time_end.month = timep->tm_mon;
					nmp_record_info->week.days[days].seg[seg_time_num].time_end.date = timep->tm_mday ;
					nmp_record_info->week.days[days].seg[seg_time_num].time_end.hour = timep->tm_hour;
					nmp_record_info->week.days[days].seg[seg_time_num].time_end.minute = timep->tm_min; 
					nmp_record_info->week.days[days].seg[seg_time_num].time_end.second = timep->tm_sec;
				}
				nmp_record_info->week.days[days].day_id = days;
				nmp_record_info->week.days[days].count = seg_time_num;
			}
			nmp_record_info->week.count = days;

			break;
			
		case SWAP_UNPACK:
			//st_record_info->stScheduleRecordPolicy.bIsScheduleRecordOpened = 1;
			st_record_info->bIsRecordAudioOpened = 1;
			st_record_info->stScheduleRecordPolicy.nScheduleRecordType = 2;
			for(days = 0; (unsigned int)days < nmp_record_info->week.count; days++)
			{
				seg_time_count = (nmp_record_info->week.days[days].count < (unsigned int)CONST_MAX_PERIOD_SCHEDULE_RECORD_TIME_NUM) ?
										(nmp_record_info->week.days[days].count) : CONST_MAX_PERIOD_SCHEDULE_RECORD_TIME_NUM;
				printf("====seg_time_count %d, %d\n", seg_time_count, nmp_record_info->week.days[days].count);
				for(seg_time_num = 0; seg_time_num < seg_time_count; seg_time_num++)
				{
					st_timep.tm_year = nmp_record_info->week.days[days].seg[seg_time_num].time_start.year-1900;
					st_timep.tm_mon = nmp_record_info->week.days[days].seg[seg_time_num].time_start.month-1;
					st_timep.tm_mday = nmp_record_info->week.days[days].seg[seg_time_num].time_start.date;
					st_timep.tm_hour = nmp_record_info->week.days[days].seg[seg_time_num].time_start.hour;
					st_timep.tm_min = nmp_record_info->week.days[days].seg[seg_time_num].time_start.minute; 
					st_timep.tm_sec = nmp_record_info->week.days[days].seg[seg_time_num].time_start.second;
					st_record_info->stScheduleRecordPolicy.stScheduleTimeParam.stScheduleWeekList[days].stScheduleTime[seg_time_num].nStartTime = (long)mktime(&st_timep);

					st_timep.tm_year = nmp_record_info->week.days[days].seg[seg_time_num].time_end.year-1900;
					st_timep.tm_mon = nmp_record_info->week.days[days].seg[seg_time_num].time_end.month-1;
					st_timep.tm_mday = nmp_record_info->week.days[days].seg[seg_time_num].time_end.date;
					st_timep.tm_hour = nmp_record_info->week.days[days].seg[seg_time_num].time_end.hour;
					st_timep.tm_min = nmp_record_info->week.days[days].seg[seg_time_num].time_end.minute; 
					st_timep.tm_sec = nmp_record_info->week.days[days].seg[seg_time_num].time_end.second;
					st_record_info->stScheduleRecordPolicy.stScheduleTimeParam.stScheduleWeekList[days].stScheduleTime[seg_time_num].nEndTime = (long)mktime(&st_timep);
				}
				st_record_info->stScheduleRecordPolicy.stScheduleTimeParam.stScheduleWeekList[days].nWeekDay = days;
				st_record_info->stScheduleRecordPolicy.stScheduleTimeParam.stScheduleWeekList[days].nScheduleTimeNum = seg_time_count;
			}

			break;
	}
}



void jny_swap_hide_info(ST_BlindAreaParam *st_hide_info, JHideParameter *nmp_hide_info, int flag)
{
	int i;
	NMP_ASSERT(st_hide_info && nmp_hide_info);

	switch(flag)
	{
		case SWAP_PACK:
			nmp_hide_info->hide_enable = st_hide_info->bEnableFlag;
			nmp_hide_info->hide_color = RGB(st_hide_info->stColorParam.nRed, st_hide_info->stColorParam.nGreen,
											st_hide_info->stColorParam.nBlue);
			for(i = 0; i < st_hide_info->nAreaParamNum; i++)
			{
				nmp_hide_info->hide_area.rect[i].left 	= (st_hide_info->stAreaParamList[i].nTopX*704)/100;
				nmp_hide_info->hide_area.rect[i].top		= (st_hide_info->stAreaParamList[i].nTopY*576)/100;
				nmp_hide_info->hide_area.rect[i].right 	= nmp_hide_info->hide_area.rect[i].left + (st_hide_info->stAreaParamList[i].nWidth*704)/100;
				nmp_hide_info->hide_area.rect[i].bottom 	= nmp_hide_info->hide_area.rect[i].top + (st_hide_info->stAreaParamList[i].nHeight*576)/100;
			}
			nmp_hide_info->hide_area.count 		= st_hide_info->nAreaParamNum;
			nmp_hide_info->max_width	= 704;
			nmp_hide_info->max_height	= 576;

			break;
			
		case SWAP_UNPACK:
			st_hide_info->bEnableFlag 			= 1;//nmp_hide_info->hide_enable;
			st_hide_info->stColorParam.nRed 	= (unsigned char)nmp_hide_info->hide_color;
			st_hide_info->stColorParam.nGreen	= (unsigned char)(nmp_hide_info->hide_color >> 16);
			st_hide_info->stColorParam.nBlue	= (unsigned char)(nmp_hide_info->hide_color >> 24);
			st_hide_info->nAreaParamNum			= nmp_hide_info->hide_area.count;
			for(i = 0; (unsigned int)i < nmp_hide_info->hide_area.count; i++)
			{
				st_hide_info->stAreaParamList[i].nTopX 		= (nmp_hide_info->hide_area.rect[i].left*100)/704;
				st_hide_info->stAreaParamList[i].nTopY 		= (nmp_hide_info->hide_area.rect[i].top*100)/576;
				st_hide_info->stAreaParamList[i].nWidth 		= ((nmp_hide_info->hide_area.rect[i].right - nmp_hide_info->hide_area.rect[i].left)*100)/704;
				st_hide_info->stAreaParamList[i].nHeight	= ((nmp_hide_info->hide_area.rect[i].bottom - nmp_hide_info->hide_area.rect[i].top)*100)/576;
			}
			break;
	}
}


void jny_swap_move_alarm_info(ST_MotionDetectionEventParam *st_move_alarm_info, JMoveAlarm *nmp_move_alarm_info, int flag)
{
	int i;
	int days, seg_time_num;
	time_t	start_time, end_time;
	struct tm *timep;
	struct tm st_timep;
	int seg_time_count;
	NMP_ASSERT(st_move_alarm_info && nmp_move_alarm_info);

	switch(flag)
	{
		case SWAP_PACK:
			nmp_move_alarm_info->max_width = 704;
			nmp_move_alarm_info->max_height = 576;
			nmp_move_alarm_info->move_enable = st_move_alarm_info->bMotionDetectionEnableFlag;
			for(i = 0; i < st_move_alarm_info->stMotionDetectionParam.nDetectionAreaNum; i++)
			{
				nmp_move_alarm_info->detect_area.rect[i].left = st_move_alarm_info->stMotionDetectionParam.stDetectionAreaList[i].nTopX;
				nmp_move_alarm_info->detect_area.rect[i].top	= st_move_alarm_info->stMotionDetectionParam.stDetectionAreaList[i].nTopY;
				nmp_move_alarm_info->detect_area.rect[i].right = st_move_alarm_info->stMotionDetectionParam.stDetectionAreaList[i].nTopX + 
																st_move_alarm_info->stMotionDetectionParam.stDetectionAreaList[i].nWidth;
				nmp_move_alarm_info->detect_area.rect[i].bottom = st_move_alarm_info->stMotionDetectionParam.stDetectionAreaList[i].nTopY +
																st_move_alarm_info->stMotionDetectionParam.stDetectionAreaList[i].nHeight;
			}
			nmp_move_alarm_info->detect_area.count = st_move_alarm_info->stMotionDetectionParam.nDetectionAreaNum;

			for(days = 0; days < 7; days++)
			{
				seg_time_count = st_move_alarm_info->stScheduleTimeParam.stScheduleWeekList[days].nScheduleTimeNum;
				printf("--------seg_time_count = %d\n", seg_time_count);
				for(seg_time_num = 0; seg_time_num < seg_time_count; seg_time_num++)
				{
					nmp_move_alarm_info->week.days[days].seg[seg_time_num].enable = 1;
					start_time = (time_t)st_move_alarm_info->stScheduleTimeParam.stScheduleWeekList[days].stScheduleTime[seg_time_num].nStartTime;
					end_time = (time_t)st_move_alarm_info->stScheduleTimeParam.stScheduleWeekList[days].stScheduleTime[seg_time_num].nEndTime;

					timep = gmtime(&start_time);
					
					nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_start.year = timep->tm_year+1900;
					nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_start.month = timep->tm_mon+1;
					nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_start.date = timep->tm_mday ;
					nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_start.hour = timep->tm_hour;
					nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_start.minute = timep->tm_min; 
					nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_start.second = timep->tm_sec;
					printf("start ------day %d, hour %d\n", timep->tm_mday, timep->tm_hour);
					timep = gmtime(&end_time);
					nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_end.year = timep->tm_year+1900;
					nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_end.month = timep->tm_mon+1;
					nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_end.date = timep->tm_mday ;
					nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_end.hour = timep->tm_hour;
					nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_end.minute = timep->tm_min; 
					nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_end.second = timep->tm_sec;
					printf("end ------day %d, hour %d\n", timep->tm_mday, timep->tm_hour);
				}
				nmp_move_alarm_info->week.days[days].day_id = days;
				nmp_move_alarm_info->week.days[days].count = seg_time_num;
			}
			nmp_move_alarm_info->week.count = days;
			break;
			
		case SWAP_UNPACK:
			st_move_alarm_info->bMotionDetectionEnableFlag = nmp_move_alarm_info->move_enable;
			st_move_alarm_info->stMotionDetectionParam.bAreaMaskFlag = 0;
			st_move_alarm_info->stMotionDetectionParam.bToDownCheckFlag = 1;
			st_move_alarm_info->stMotionDetectionParam.bToLeftCheckFlag = 1;
			st_move_alarm_info->stMotionDetectionParam.bToRightCheckFlag = 1;
			st_move_alarm_info->stMotionDetectionParam.bToUpCheckFlag = 1;
			st_move_alarm_info->stMotionDetectionParam.nCheckBlockNum = 4;
			st_move_alarm_info->stMotionDetectionParam.nDetectionAreaNum = nmp_move_alarm_info->detect_area.count;
			st_move_alarm_info->stMotionDetectionParam.nSensitivity = 1;
			for(i = 0; (unsigned int)i < nmp_move_alarm_info->detect_area.count; i++)
			{
				st_move_alarm_info->stMotionDetectionParam.stDetectionAreaList[i].nHeight = nmp_move_alarm_info->detect_area.rect[i].bottom -
																		nmp_move_alarm_info->detect_area.rect[i].top;
				st_move_alarm_info->stMotionDetectionParam.stDetectionAreaList[i].nWidth = nmp_move_alarm_info->detect_area.rect[i].right - 
																		nmp_move_alarm_info->detect_area.rect[i].left;
				st_move_alarm_info->stMotionDetectionParam.stDetectionAreaList[i].nTopX = nmp_move_alarm_info->detect_area.rect[i].left;
				st_move_alarm_info->stMotionDetectionParam.stDetectionAreaList[i].nTopY = nmp_move_alarm_info->detect_area.rect[i].top;
			}

			for(days = 0; (unsigned int)days < nmp_move_alarm_info->week.count; days++)
			{
				seg_time_count = (nmp_move_alarm_info->week.days[days].count < (unsigned int)CONST_MAX_PERIOD_SCHEDULE_RECORD_TIME_NUM) ?
										(nmp_move_alarm_info->week.days[days].count) : CONST_MAX_PERIOD_SCHEDULE_RECORD_TIME_NUM;
				printf("====seg_time_count %d, %d\n", seg_time_count, nmp_move_alarm_info->week.days[days].count);
				for(seg_time_num = 0; seg_time_num < seg_time_count; seg_time_num++)
				{
					st_timep.tm_year = nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_start.year-1900;
					st_timep.tm_mon = nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_start.month-1;
					st_timep.tm_mday = nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_start.date;
					st_timep.tm_hour = nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_start.hour;
					st_timep.tm_min = nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_start.minute; 
					st_timep.tm_sec = nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_start.second;
					st_move_alarm_info->stScheduleTimeParam.stScheduleWeekList[days].stScheduleTime[seg_time_num].nStartTime = (long)mktime(&st_timep);

					st_timep.tm_year = nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_end.year-1900;
					st_timep.tm_mon = nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_end.month-1;
					st_timep.tm_mday = nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_end.date;
					st_timep.tm_hour = nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_end.hour;
					st_timep.tm_min = nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_end.minute; 
					st_timep.tm_sec = nmp_move_alarm_info->week.days[days].seg[seg_time_num].time_end.second;
					st_move_alarm_info->stScheduleTimeParam.stScheduleWeekList[days].stScheduleTime[seg_time_num].nEndTime = (long)mktime(&st_timep);
				}
				st_move_alarm_info->stScheduleTimeParam.stScheduleWeekList[days].nWeekDay = days;
				st_move_alarm_info->stScheduleTimeParam.stScheduleWeekList[days].nScheduleTimeNum = seg_time_count;
			}
			break;
	}
}

void jny_swap_hide_alarm_info(ST_OcclusionDetectionEventParam *st_hide_alarm_info, JHideAlarm *nmp_hide_alarm_info, int flag)
{
	int i, count;
	int days, seg_time_num;
	time_t	start_time, end_time;
	struct tm *timep;
	struct tm st_timep;
	int seg_time_count;
	NMP_ASSERT(st_hide_alarm_info && nmp_hide_alarm_info);

	switch(flag)
	{
		case SWAP_PACK:
			nmp_hide_alarm_info->hide_enable = st_hide_alarm_info->bOcclusionDetectionEnableFlag;
			nmp_hide_alarm_info->sensitive_level = st_hide_alarm_info->stOcclusionDetectionParam.nSensitivity;
			nmp_hide_alarm_info->max_width = MAX_WIDTH;
			nmp_hide_alarm_info->max_height = MAX_HEIGHT;

			count = st_hide_alarm_info->stOcclusionDetectionParam.nDetectionAreaNum;
			for(i = 0; i < count; i++)
			{
				nmp_hide_alarm_info->detect_area.rect[i].left = st_hide_alarm_info->stOcclusionDetectionParam.stDetectionAreaList[i].nTopX;
				nmp_hide_alarm_info->detect_area.rect[i].top	= st_hide_alarm_info->stOcclusionDetectionParam.stDetectionAreaList[i].nTopY;
				nmp_hide_alarm_info->detect_area.rect[i].right= st_hide_alarm_info->stOcclusionDetectionParam.stDetectionAreaList[i].nTopX + 
																st_hide_alarm_info->stOcclusionDetectionParam.stDetectionAreaList[i].nWidth;
				nmp_hide_alarm_info->detect_area.rect[i].bottom= st_hide_alarm_info->stOcclusionDetectionParam.stDetectionAreaList[i].nTopY +
																st_hide_alarm_info->stOcclusionDetectionParam.stDetectionAreaList[i].nHeight;
			}
			nmp_hide_alarm_info->detect_area.count = count;

			//nmp_hide_alarm_info->week.count = st_hide_alarm_info->stScheduleTimeParam.stScheduleWeekList[0].nScheduleTimeNum;
			for(days = 0; days < 7; days++)
			{
				seg_time_count = st_hide_alarm_info->stScheduleTimeParam.stScheduleWeekList[days].nScheduleTimeNum;
				for(seg_time_num = 0; seg_time_num < seg_time_count; seg_time_num++)
				{
					nmp_hide_alarm_info->week.days[days].seg[seg_time_num].enable = 1;
					start_time = (time_t)st_hide_alarm_info->stScheduleTimeParam.stScheduleWeekList[days].stScheduleTime[seg_time_num].nStartTime;
					end_time = (time_t)st_hide_alarm_info->stScheduleTimeParam.stScheduleWeekList[days].stScheduleTime[seg_time_num].nEndTime;

					timep = gmtime(&start_time);
					
					nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_start.year = timep->tm_year;
					nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_start.month = timep->tm_mon;
					nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_start.date = timep->tm_mday ;
					nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_start.hour = timep->tm_hour;
					nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_start.minute = timep->tm_min; 
					nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_start.second = timep->tm_sec;

					timep = gmtime(&end_time);
					nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_end.year = timep->tm_year;
					nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_end.month = timep->tm_mon;
					nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_end.date = timep->tm_mday ;
					nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_end.hour = timep->tm_hour;
					nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_end.minute = timep->tm_min; 
					nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_end.second = timep->tm_sec;
				}
				nmp_hide_alarm_info->week.days[days].day_id = days;
				nmp_hide_alarm_info->week.days[days].count = seg_time_num;
			}
			nmp_hide_alarm_info->week.count = days;
			break;
			
		case SWAP_UNPACK:
			st_hide_alarm_info->bOcclusionDetectionEnableFlag = nmp_hide_alarm_info->hide_enable;
			st_hide_alarm_info->stOcclusionDetectionParam.nSensitivity = nmp_hide_alarm_info->sensitive_level;
			st_hide_alarm_info->stOcclusionDetectionParam.nDetectionAreaNum = nmp_hide_alarm_info->detect_area.count;

			for(i = 0; (unsigned int)i < nmp_hide_alarm_info->detect_area.count; i++)
			{
				st_hide_alarm_info->stOcclusionDetectionParam.stDetectionAreaList[i].nTopX 		= (nmp_hide_alarm_info->detect_area.rect[i].left*100)/704;
				st_hide_alarm_info->stOcclusionDetectionParam.stDetectionAreaList[i].nTopY 		= (nmp_hide_alarm_info->detect_area.rect[i].top*100)/576;
				st_hide_alarm_info->stOcclusionDetectionParam.stDetectionAreaList[i].nWidth 	= ((nmp_hide_alarm_info->detect_area.rect[i].right - nmp_hide_alarm_info->detect_area.rect[i].left)*100)/704;
				st_hide_alarm_info->stOcclusionDetectionParam.stDetectionAreaList[i].nHeight	= ((nmp_hide_alarm_info->detect_area.rect[i].bottom - nmp_hide_alarm_info->detect_area.rect[i].top)*100)/576;
			}

			for(days = 0; days < 7; days++)
			{
				seg_time_count = (nmp_hide_alarm_info->week.days[days].count < (unsigned int)CONST_MAX_PERIOD_SCHEDULE_RECORD_TIME_NUM) ?
										(nmp_hide_alarm_info->week.days[days].count) : CONST_MAX_PERIOD_SCHEDULE_RECORD_TIME_NUM;
				for(seg_time_num = 0; seg_time_num < seg_time_count; seg_time_num++)
				{
					st_timep.tm_year = nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_start.year;
					st_timep.tm_mon = nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_start.month;
					st_timep.tm_mday = nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_start.date;
					st_timep.tm_hour = nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_start.hour;
					st_timep.tm_min = nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_start.minute; 
					st_timep.tm_sec = nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_start.second;
					st_hide_alarm_info->stScheduleTimeParam.stScheduleWeekList[days].stScheduleTime[seg_time_num].nStartTime = (long)mktime(&st_timep);

					st_timep.tm_year = nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_end.year;
					st_timep.tm_mon = nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_end.month;
					st_timep.tm_mday = nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_end.date;
					st_timep.tm_hour = nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_end.hour;
					st_timep.tm_min = nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_end.minute; 
					st_timep.tm_sec = nmp_hide_alarm_info->week.days[days].seg[seg_time_num].time_end.second;
					st_hide_alarm_info->stScheduleTimeParam.stScheduleWeekList[days].stScheduleTime[seg_time_num].nEndTime = (long)mktime(&st_timep);
				}
				st_hide_alarm_info->stScheduleTimeParam.stScheduleWeekList[days].nScheduleTimeNum =
																					nmp_hide_alarm_info->week.days[days].count;
			}
			break;
	}
}

void jny_video_lost_info(ST_VideoLoseDetectionEventParam *st_video_lost_info, JLostAlarm *nmp_video_lost_info, int flag)
{
	NMP_ASSERT(st_video_lost_info && nmp_video_lost_info);

	switch(flag)
	{
		case SWAP_PACK:
			nmp_video_lost_info->lost_enable = st_video_lost_info->bVideoLoseDetectionEnableFlag;
			break;
			
		case SWAP_UNPACK:
			st_video_lost_info->bVideoLoseDetectionEnableFlag = nmp_video_lost_info->lost_enable;
			break;
	}
}

