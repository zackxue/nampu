#include <arpa/inet.h>
#include "nmp_hie_swap.h"

void hie_swap_device_info(HY_DVR_DEVICE_INFO *dev_cfg, JDeviceInfo *dev_info, int flag)
{
	NMP_ASSERT(dev_cfg && dev_info);

    switch (flag)
    {
        case SWAP_PACK:
            snprintf((char*)dev_info->dev_version, sizeof(dev_info->dev_version), 
                "%s", dev_cfg->szSoftwareVersion);
            snprintf((char*)dev_info->hw_version, sizeof(dev_info->hw_version), 
                "%s", dev_cfg->szHardwareVersion);
            //printf("dev_cfg->sPanelVer: %s\n", dev_cfg->sPanelVer);
            //snprintf((char*)dev_info->release_date, sizeof(dev_info->release_date), 
              //  "%s", dev_cfg->sPanelVer);

            //dev_info->pu_type     = (JPuType)dev_cfg->dwDevType;//DVR类型, 1:DVR 2:ATM DVR 3:DVS
            dev_info->di_num      = (int)(dev_cfg->byVideoInChannels + dev_cfg->byAlarmInChannels + 
            							dev_cfg->byAudioInChannels + dev_cfg->byVoiceInChannels);
            dev_info->do_num      = (int)(dev_cfg->byAlarmOutChannels + dev_cfg->bySpotOutChannles);
            //dev_info->channel_num = (int)dev_cfg->byChanNum;
            //dev_info->rs485_num   = (int)dev_cfg->byRS485Num;
            //dev_info->rs232_num   = (int)dev_cfg->byRS232Num;
            break;

        case SWAP_UNPACK:
            break;

        default:
            break;
    }

    return ;
}


void hie_swap_serial_info(HY_DVR_SERIAL *dev_cfg, JSerialParameter *dev_info, int flag)
{
    NMP_ASSERT(dev_cfg && dev_info);

    switch (flag)
    {
    case SWAP_PACK:
        dev_info->serial_no = dev_cfg->nComType;

		dev_info->baud_rate = dev_cfg->nBaudRate;

        dev_info->data_bit = dev_cfg->nDataBit;
        dev_info->stop_bit = dev_cfg->nStopBit;
        dev_info->verify   = dev_cfg->nParity;		//0-NONE, 1-ODD, 2-EVEN, 3-SPACE
        break;

    case SWAP_UNPACK:
		dev_cfg->nComType = dev_info->serial_no;
		dev_cfg->nBaudRate = dev_info->baud_rate;
		dev_cfg->nDataBit = dev_info->data_bit;
		dev_cfg->nStopBit = dev_info->stop_bit;
		dev_cfg->nParity = dev_info->verify;
        break;

    default:
        break;
    }
    return ;
}

void hie_swap_time_info(HY_DVR_TIME *dev_cfg, JDeviceTime *dev_info, int flag)
{
    NMP_ASSERT(dev_cfg && dev_info);

    switch (flag)
    {
    case SWAP_PACK:
		dev_info->time.year = dev_cfg->nYear;
		dev_info->time.month = dev_cfg->nMonth;
		dev_info->time.date = dev_cfg->nDay;
		dev_info->time.hour = dev_cfg->nHour;
		dev_info->time.minute = dev_cfg->nMinute;
		dev_info->time.second = dev_cfg->nSecond;
		
		//dev_info->time.zone =;
		//dev_info->sync_enable = 1;
        break;

    case SWAP_UNPACK:
		dev_cfg->nYear = dev_info->time.year;
		dev_cfg->nMonth = dev_info->time.month;
		dev_cfg->nDay = dev_info->time.date;
		dev_cfg->nHour = dev_info->time.hour;
		dev_cfg->nMinute = dev_info->time.minute;
		dev_cfg->nSecond = dev_info->time.second;
        break;

    default:
        break;
    }

    return ;
}

void hie_swap_net_info(HY_DVR_NET_CFG *dev_cfg, JNetworkInfo *dev_info, int flag)
{
    NMP_ASSERT(dev_cfg && dev_info);

    switch (flag)
    {
    case SWAP_PACK:
		strncpy((char *)dev_info->network[J_SDK_ETH0].ip, dev_cfg->struEth.szIPAddress, strlen(dev_cfg->struEth.szIPAddress));
		strncpy((char *)dev_info->network[J_SDK_ETH0].netmask, dev_cfg->struEth.szSubnetMask, strlen(dev_cfg->struEth.szSubnetMask));
		strncpy((char *)dev_info->network[J_SDK_ETH0].gateway, dev_cfg->struEth.szGateway, strlen(dev_cfg->struEth.szGateway));
		strncpy((char *)dev_info->main_dns, dev_cfg->struDNS.szPrimaryDNS, strlen(dev_cfg->struDNS.szPrimaryDNS));
		strncpy((char *)dev_info->backup_dns, dev_cfg->struDNS.szSecondaryDNS, strlen(dev_cfg->struDNS.szSecondaryDNS));
		break;

    case SWAP_UNPACK:
		strncpy(dev_cfg->struEth.szIPAddress, (char *)dev_info->network[J_SDK_ETH0].ip, strlen((char *)dev_info->network[J_SDK_ETH0].ip));
		strncpy(dev_cfg->struEth.szSubnetMask, (char *)dev_info->network[J_SDK_ETH0].netmask, strlen((char *)dev_info->network[J_SDK_ETH0].netmask));
		strncpy(dev_cfg->struEth.szGateway, (char *)dev_info->network[J_SDK_ETH0].gateway, strlen((char *)dev_info->network[J_SDK_ETH0].gateway));
		strncpy(dev_cfg->struDNS.szPrimaryDNS, (char *)dev_info->main_dns, strlen((char *)dev_info->main_dns));
		strncpy(dev_cfg->struDNS.szSecondaryDNS, (char *)dev_info->backup_dns, strlen((char *)dev_info->backup_dns));
		break;

    default:
        break;
    }

    return ;
}

void hie_swap_pppoe_info(HY_DVR_PPPOE_CONF  *dev_cfg, JPPPOEInfo *dev_info, int flag)
{
    NMP_ASSERT(dev_cfg && dev_info);

    switch (flag)
    {
    case SWAP_PACK:
		dev_info->enable = dev_cfg->bPPPoEEnable;
		strncpy((char *)dev_info->account, dev_cfg->szPPPoEUser, strlen(dev_cfg->szPPPoEUser));
		strncpy((char *)dev_info->passwd, dev_cfg->szPPPoEPassword, strlen(dev_cfg->szPPPoEPassword));
		break;

    case SWAP_UNPACK:
		dev_cfg->bPPPoEEnable = dev_info->enable;
		strncpy(dev_cfg->szPPPoEUser, (char *)dev_info->account, strlen((char *)dev_info->account));
		strncpy(dev_cfg->szPPPoEPassword, (char *)dev_info->passwd, strlen((char *)dev_info->passwd));
		break;

    default:
        break;
    }

    return ;
}


void hie_swap_ddns_info(HY_DVR_DDNS_CONF  *dev_cfg, JDdnsConfig *dev_info, int flag)
{
    NMP_ASSERT(dev_cfg && dev_info);

    switch (flag)
    {
    case SWAP_PACK:
		if(dev_cfg->nDdnsProtocolCount > 0)
		{
			strncpy((char *)dev_info->ddns_account, dev_cfg->struDdnsProtocol[0].szDomainName, strlen(dev_cfg->struDdnsProtocol[0].szDomainName));
			strncpy((char *)dev_info->ddns_usr, dev_cfg->struDdnsProtocol[0].szUserName, strlen(dev_cfg->struDdnsProtocol[0].szUserName));
			strncpy((char *)dev_info->ddns_pwd, dev_cfg->struDdnsProtocol[0].szUserPassword, strlen(dev_cfg->struDdnsProtocol[0].szUserPassword));
			dev_info->ddns_open = dev_cfg->struDdnsProtocol[0].bDdnsEnable;
			dev_info->ddns_open = dev_cfg->struDdnsProtocol[0].wRegisterPort;
		}
		break;

    case SWAP_UNPACK:
		dev_cfg->nDdnsProtocolCount = 1;
		strncpy(dev_cfg->struDdnsProtocol[0].szDomainName, (char *)dev_info->ddns_account, strlen((char *)dev_info->ddns_account));
		strncpy(dev_cfg->struDdnsProtocol[0].szUserName, (char *)dev_info->ddns_usr, strlen((char *)dev_info->ddns_usr));
		strncpy(dev_cfg->struDdnsProtocol[0].szUserPassword, (char *)dev_info->ddns_pwd, strlen((char *)dev_info->ddns_pwd));
		dev_cfg->struDdnsProtocol[0].bDdnsEnable = dev_info->ddns_open;
		dev_cfg->struDdnsProtocol[0].wRegisterPort = dev_info->ddns_open;
		break;

    default:
        break;
    }

    return ;
}

void hie_swap_disk_list(HY_DVR_STORAGE_CFG *dev_cfg, JDeviceDiskInfo *dev_info, int flag)
{
	int i, j;
    NMP_ASSERT(dev_cfg && dev_info);

    switch (flag)
    {
    case SWAP_PACK:
		for(i = 0, j = 0; i < DVR_MAX_HARDDISK_NUM; i++)
		{
			if(dev_cfg->struDisk[i].bValid)
			{
				dev_info->disk[j].disk_no = dev_cfg->struDisk[i].nBusNum;
				dev_info->disk[j].total_size = dev_cfg->struDisk[i].dwCapacity;
				//dev_info->disk[i].
				j++;
			}
		}
		dev_info->disk_num = j;
		
		break;

    case SWAP_UNPACK:

		break;

    default:
        break;
    }

    return ;
}

void hie_swap_encode_info(HY_DVR_COMPRESSION_CFG *dev_cfg, JEncodeParameter *dev_info, int flag, int channel)
{
    NMP_ASSERT(dev_cfg && dev_info);

    switch (flag)
    {
    case SWAP_PACK:
		dev_info->frame_rate = dev_cfg->struCompressChannel[channel].struRecordPara[0].nVideoFrameRate;
		switch(dev_cfg->struCompressChannel[channel].struRecordPara[0].nResolution)
		{
			case 0:
				dev_info->resolution = J_SDK_VIDEO_CIF;
				break;
			case 1:
				dev_info->resolution = J_SDK_VIDEO_D1;
				break;
			/*case DVR_CAP_RESOLUTION_QCIF:
				dev_info->resolution = J_SDK_VIDEO_D1;
				break;
			case DVR_CAP_RESOLUTION_QVGA:
				dev_info->resolution = J_SDK_VIDEO_QVGA;
				break;
			case DVR_CAP_RESOLUTION_VGA:
				dev_info->resolution = J_SDK_VIDEO_VGA;
				break;

			case DVR_CAP_RESOLUTION_SVGA:
				dev_info->resolution = J_SDK_VIDEO_SVGA;
				break;

			case DVR_CAP_RESOLUTION_UXGA:
				dev_info->resolution = J_SDK_VIDEO_UXGA;
				break;

			case DVR_CAP_RESOLUTION_HD720:
				dev_info->resolution = J_SDK_VIDEO_720P;
				break;

			case DVR_CAP_RESOLUTION_HD1080:
				dev_info->resolution = J_SDK_VIDEO_1080P;
				break;
			*/
			default :
				break;
		}
		dev_info->qp_value = dev_cfg->struCompressChannel[channel].struRecordPara[0].nQuotiety;

		dev_info->video_type = J_SDK_AV_VIDEO_H264;
		
		//dev_info->code_rate = dev_cfg->struCompressChannel[channel].struRecordPara[0].
		break;

    case SWAP_UNPACK:
		dev_cfg->struCompressChannel[channel].struRecordPara[0].nVideoFrameRate = dev_info->frame_rate;
		dev_cfg->struCompressChannel[channel].struRecordPara[0].nQuotiety = dev_info->qp_value;
		switch(dev_info->resolution)
		{
			case J_SDK_VIDEO_CIF:
				dev_cfg->struCompressChannel[channel].struRecordPara[0].nResolution = 0;
				break;
			case J_SDK_VIDEO_D1:
				dev_cfg->struCompressChannel[channel].struRecordPara[0].nResolution = 1;
			break;
			/*
			case J_SDK_VIDEO_D1:
				dev_cfg->struCompressChannel[channel].struRecordPara[0].nResolution = DVR_CAP_RESOLUTION_QCIF;
				break;
			case J_SDK_VIDEO_QVGA:
				dev_cfg->struCompressChannel[channel].struRecordPara[0].nResolution = DVR_CAP_RESOLUTION_QVGA;
				break;
			case J_SDK_VIDEO_VGA:
				dev_cfg->struCompressChannel[channel].struRecordPara[0].nResolution = DVR_CAP_RESOLUTION_VGA;
				break;

			case J_SDK_VIDEO_SVGA:
				dev_cfg->struCompressChannel[channel].struRecordPara[0].nResolution = DVR_CAP_RESOLUTION_SVGA;
				break;

			case J_SDK_VIDEO_UXGA:
				dev_info->resolution = DVR_CAP_RESOLUTION_UXGA;
				break;

			case J_SDK_VIDEO_720P:
				dev_info->resolution = DVR_CAP_RESOLUTION_HD720;
				break;

			case J_SDK_VIDEO_1080P:
				dev_info->resolution = DVR_CAP_RESOLUTION_HD1080;
				break;
			*/
			default :
				
				break;
		}
		break;

    default:
        break;
    }

    return ;

}

void hie_swap_osd_info(HY_DVR_PIC_CFG *dev_cfg, JOSDParameter *dev_info, int flag, int channel)
{
    NMP_ASSERT(dev_cfg && dev_info);

    switch (flag)
    {
    case SWAP_PACK:
		switch(dev_cfg->struOsd[channel].nShowOsd)
		{
			case 0:
				dev_info->time_enable = 0;
				dev_info->text_enable = 0; 
				dev_info->stream_enable = 0;
				break;
			case 1:
				dev_info->time_enable = 0;
				dev_info->text_enable = 1; 
				dev_info->stream_enable = 0;

				dev_info->text_display_x = dev_cfg->struOsd[channel].nNameTopLeftX;
				dev_info->text_display_y = dev_cfg->struOsd[channel].nNameTopLeftY;
				dev_info->max_width = 703;
				dev_info->max_height = 575;
				break;
			case 2:
				dev_info->time_enable = 1;
				dev_info->text_enable = 0; 
				dev_info->stream_enable = 0;

				dev_info->time_display_x = dev_cfg->struOsd[channel].nTimeTopLeftX;
				dev_info->time_display_y = dev_cfg->struOsd[channel].nTimeTopLeftY;
				dev_info->max_width = 703;
				dev_info->max_height = 575;
				break;
			case 3:
				dev_info->time_enable = 1;
				dev_info->text_enable = 1; 
				dev_info->stream_enable = 0;
				
				dev_info->text_display_x = dev_cfg->struOsd[channel].nNameTopLeftX;
				dev_info->text_display_y = dev_cfg->struOsd[channel].nNameTopLeftY;

				dev_info->time_display_x = dev_cfg->struOsd[channel].nTimeTopLeftX;
				dev_info->time_display_y = dev_cfg->struOsd[channel].nTimeTopLeftY;
				dev_info->max_width = 703;
				dev_info->max_height = 575;
				break;
			default:
				break;
		}
		break;

    case SWAP_UNPACK:
		if(dev_info->time_enable == 0 && dev_info->text_enable == 0)
		{
			dev_cfg->struOsd[channel].bValid = 1;
			dev_cfg->struOsd[channel].nShowOsd = 0;
		}
		else if(dev_info->time_enable == 1 && dev_info->text_enable == 0)
		{
			dev_cfg->struOsd[channel].bValid = 1;
			dev_cfg->struOsd[channel].nShowOsd = 2;

			dev_cfg->struOsd[channel].nTimeTopLeftX = dev_info->time_display_x;
			dev_cfg->struOsd[channel].nTimeTopLeftY = dev_info->time_display_y;
		}
		else if(dev_info->time_enable == 0 && dev_info->text_enable == 1)
		{
			dev_cfg->struOsd[channel].bValid = 1;
			dev_cfg->struOsd[channel].nShowOsd = 1;
			dev_cfg->struOsd[channel].nNameTopLeftX = dev_info->text_display_x;
			dev_cfg->struOsd[channel].nNameTopLeftY = dev_info->text_display_y;

		}
		else if(dev_info->time_enable == 1 && dev_info->text_enable == 1)
		{
			dev_cfg->struOsd[channel].bValid = 1;
			dev_cfg->struOsd[channel].nShowOsd = 3;

			dev_cfg->struOsd[channel].nTimeTopLeftX = dev_info->time_display_x;
			dev_cfg->struOsd[channel].nTimeTopLeftY = dev_info->time_display_y;

			dev_cfg->struOsd[channel].nNameTopLeftX = dev_info->text_display_x;
			dev_cfg->struOsd[channel].nNameTopLeftY = dev_info->text_display_y;
		}
		else
		{
			dev_cfg->struOsd[channel].bValid = 0;
		}
		break;

    default:
        break;
    }

    return ;

}


void hie_swap_record_info(HY_DVR_RECORD_SCHED *dev_cfg, JRecordParameter *dev_info, int flag, int channel)
{
#if 0
	time_t timep; 
	struct tm *tm_p;
	int i, j, k;
	
	NMP_ASSERT(dev_cfg && dev_info);

    switch (flag)
    {
    case SWAP_PACK:
		switch(dev_cfg->struRecordStream.uStreamIndex[channel])
		{
			case 0:		//主码流
				dev_info->level = 0;
				break;
			case 1:		//子码流
				dev_info->level = 2;
				break;
			case 2:		//高清(VGA分辨率)子码流
				dev_info->level = 2;
				break;
			default:
				break;
		}

		time(&timep); 
		tm_p = localtime(&timep); /*取得当地时间*/
		for(i = 0; i < 6; i++)
		{
			for(j = 0; j < J_SDK_MAX_SEG_SZIE; j++)
			{
				for(k = 0; k < DVR_MAX_TIMESEGMENT; k++)
				{
					if(dev_cfg->struRecordTime[channel].bySetType[i][k] == 1)
					{
	
					}
				}
				//tm_p->tm_wday
				//dev_info->week.days[i].seg[j].time_start.year = 
			}
			
		}
		break;

    case SWAP_UNPACK:
		
		break;

    default:
        break;
    }
#endif
    return ;
}
