
#include "nmp_xmt_swap.h"


static __inline__ void 
xmt_swap_h264_dvr_time(H264_DVR_TIME *x_time, JTime *nmp_time, int flag)
{
    NMP_ASSERT(x_time && nmp_time);

    switch(flag)
    {
        case SWAP_PACK:
            nmp_time->year    = x_time->dwYear - 1900;
            nmp_time->month   = x_time->dwMonth;
            nmp_time->date    = x_time->dwDay;
            nmp_time->hour    = x_time->dwHour;
            nmp_time->minute  = x_time->dwMinute;
            nmp_time->second  = x_time->dwSecond;
            break;

        case SWAP_UNPACK:
            x_time->dwYear   = nmp_time->year + 1900;
            x_time->dwMonth  = nmp_time->month;
            x_time->dwDay    = nmp_time->date;
            x_time->dwHour   = nmp_time->hour;
            x_time->dwMinute = nmp_time->minute;
            x_time->dwSecond = nmp_time->second;
            break;

        default:
            break;
    }
}

static __inline__ void 
xmt_swap_sdk_system_time(SDK_SYSTEM_TIME *x_time, JTime *nmp_time, int flag)
{
    NMP_ASSERT(x_time && nmp_time);

    switch(flag)
    {
        case SWAP_PACK:
            nmp_time->year    = x_time->year - 1900;
            nmp_time->month   = x_time->month;
            nmp_time->date    = x_time->day;
            nmp_time->hour    = x_time->hour;
            nmp_time->minute  = x_time->minute;
            nmp_time->second  = x_time->second;
            break;

        case SWAP_UNPACK:
            x_time->year   = nmp_time->year + 1900;
            x_time->month  = nmp_time->month;
            x_time->day    = nmp_time->date;
            x_time->hour   = nmp_time->hour;
            x_time->minute = nmp_time->minute;
            x_time->second = nmp_time->second;
            break;

        default:
            break;
    }
}

static __inline__ void 
xmt_process_build_date(SDK_SYSTEM_TIME *x_time, char *nmp_date, int flag)
{
    switch (flag)
    {
        case SWAP_PACK:
            sprintf(nmp_date, "%d.%d.%d", x_time->year, x_time->month, x_time->day);
            break;
        case SWAP_UNPACK:
            sscanf(nmp_date, "%d.%d.%d", &x_time->year, &x_time->month, &x_time->day);
            break;
    }
}


static __inline__ void 
xmt_process_ip_addr(CONFIG_IPAddress *addr, char *ip, int flag)
{
    int c0, c1, c2, c3;

    switch (flag)
    {
        case SWAP_PACK:
            sprintf(ip, "%d.%d.%d.%d", (int)addr->c[0], 
                (int)addr->c[1], (int)addr->c[2], (int)addr->c[3]);
            break;
        case SWAP_UNPACK:
            sscanf(ip, "%d.%d.%d.%d", &c0, &c1, &c2, &c3);
            addr->c[0] = c0;
            addr->c[1] = c1;
            addr->c[2] = c2;
            addr->c[3] = c3;
            break;
    }
}

void xmt_swap_device_info(H264_DVR_DEVICEINFO *dev_cfg, JDeviceInfo *dev_info, int flag)
{
    NMP_ASSERT(dev_cfg && dev_info);

    switch (flag)
    {
        case SWAP_PACK:
            strncpy((char*)dev_info->dev_version, dev_cfg->sSoftWareVersion, 
                sizeof(dev_info->dev_version)-1);
            strncpy((char*)dev_info->hw_version, dev_cfg->sHardWareVersion, 
                sizeof(dev_info->hw_version)-1);
            xmt_process_build_date(&dev_cfg->tmBuildTime, 
                (char*)dev_info->release_date, flag);
            dev_info->pu_type     = (JPuType)dev_cfg->deviceTye;
            dev_info->di_num      = (int)dev_cfg->byAlarmInPortNum;
            dev_info->do_num      = (int)dev_cfg->byAlarmOutPortNum;
            dev_info->channel_num = (int)dev_cfg->byChanNum;
            break;

        case SWAP_UNPACK:
            strncpy(dev_cfg->sSoftWareVersion, (const char*)dev_info->dev_version, 
                sizeof(dev_cfg->sSoftWareVersion)-1);
            strncpy(dev_cfg->sHardWareVersion, (const char*)dev_info->hw_version, 
                sizeof(dev_cfg->sHardWareVersion)-1);
            xmt_process_build_date(&dev_cfg->tmBuildTime, 
                (char*)dev_info->release_date, flag);
            dev_cfg->deviceTye         = (SDK_DeviceType)dev_info->pu_type;
            dev_cfg->byAlarmInPortNum  = dev_info->di_num;
            dev_cfg->byAlarmOutPortNum = dev_info->do_num;
            dev_cfg->byChanNum         = dev_info->channel_num;
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_serial_info(SDK_STR_CONFIG_PTZ *ptz_cfg, JSerialParameter *serial_info, int flag)
{
    int i;
    int baud_rate[10] = {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
    NMP_ASSERT(ptz_cfg && serial_info);

    switch (flag)
    {
        case SWAP_PACK:
            serial_info->baud_rate = baud_rate[ptz_cfg->dstComm.iBaudRate];
            serial_info->data_bit  = ptz_cfg->dstComm.iDataBits;
            serial_info->stop_bit  = ptz_cfg->dstComm.iStopBits;
            serial_info->verify    = ptz_cfg->dstComm.iParity;
            break;

        case SWAP_UNPACK:
            for (i=0; i<10; i++)
            {
                if ((int)serial_info->baud_rate <= baud_rate[i])
                {
                    ptz_cfg->dstComm.iBaudRate = i;
                    break;
                }
            }
            ptz_cfg->dstComm.iDataBits = serial_info->data_bit;
            ptz_cfg->dstComm.iStopBits = serial_info->stop_bit;
            ptz_cfg->dstComm.iParity   = serial_info->verify;
            break;

        default:
            break;
    }

    printf("iBaudRate: %02d   |  ", ptz_cfg->dstComm.iBaudRate);
    printf("baud_rate    : %d\n", serial_info->baud_rate);
    printf("iDataBits: %02d   |  ", ptz_cfg->dstComm.iDataBits);
    printf("data_bit     : %d\n", serial_info->data_bit);
    printf("iStopBits: %02d   |  ", ptz_cfg->dstComm.iStopBits);
    printf("stop_bit     : %d\n", serial_info->stop_bit);
    printf("iParity  : %02d   |  ", ptz_cfg->dstComm.iParity);
    printf("verify       : %d\n", serial_info->verify);

    return ;
}


void xmt_swap_sys_time(SDK_SYSTEM_TIME *sys_time, JDeviceTime *dev_time, int flag)
{
    NMP_ASSERT(sys_time && dev_time);

    switch (flag)
    {
        case SWAP_PACK:
            dev_time->time.year   = sys_time->year;
            dev_time->time.month  = sys_time->month;
            dev_time->time.date   = sys_time->day;
            dev_time->time.hour   = sys_time->hour;
            dev_time->time.minute = sys_time->minute;
            dev_time->time.second = sys_time->second;
            break;

        case SWAP_UNPACK:
            sys_time->year   = dev_time->time.year;
            sys_time->month  = dev_time->time.month;
            sys_time->day    = dev_time->time.date;
            sys_time->hour   = dev_time->time.hour;
            sys_time->minute = dev_time->time.minute;
            sys_time->second = dev_time->time.second;
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_ntp_info(SDK_NetNTPConfig *ntp_cfg, JDeviceNTPInfo *ntp_info, int flag)
{
    NMP_ASSERT(ntp_cfg && ntp_info);
    
    switch (flag)
    {
        case SWAP_PACK:
            sprintf((char*)ntp_info->ntp_server_ip, 
                "%d.%d.%d.%d", 
                ntp_cfg->Server.ip.c[0],
                ntp_cfg->Server.ip.c[1],
                ntp_cfg->Server.ip.c[2],
                ntp_cfg->Server.ip.c[3]);
            ntp_info->ntp_enable    = ntp_cfg->Enable;
            ntp_info->time_interval = ntp_cfg->UpdatePeriod;
            ntp_info->time_zone     = ntp_cfg->TimeZone;
            break;

        case SWAP_UNPACK:
            ntp_cfg->Enable       = ntp_info->ntp_enable;
            ntp_cfg->UpdatePeriod = ntp_info->time_interval;
            ntp_cfg->TimeZone    = ntp_info->time_zone;
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_network_info(SDK_CONFIG_NET_COMMON *net_cfg, JNetworkInfo *net_info, int flag)
{
    NMP_ASSERT(net_cfg && net_info);

    switch (flag)
    {
        case SWAP_PACK:
            xmt_process_ip_addr(&net_cfg->HostIP, (char*)net_info->network[J_SDK_ETH0].ip, flag);
            xmt_process_ip_addr(&net_cfg->Submask, (char*)net_info->network[J_SDK_ETH0].netmask, flag);
            xmt_process_ip_addr(&net_cfg->Gateway, (char*)net_info->network[J_SDK_ETH0].gateway, flag);

            strncpy((char*)net_info->network[J_SDK_ETH0].mac, 
                net_cfg->sMac, 
                sizeof(net_info->network[J_SDK_ETH0].mac)-1);

            net_info->network[J_SDK_ETH0].type = J_SDK_ETH0;
            //net_info->network[J_SDK_ETH0].dhcp_enable = 
              //  (int)((net_cfg->stEtherNet[0].bValid & 0x000000f0) >> 1);

            net_info->cmd_port = net_cfg->TCPPort;
            net_info->web_port = net_cfg->HttpPort;
            break;

        case SWAP_UNPACK:
            xmt_process_ip_addr(&net_cfg->HostIP, (char*)net_info->network[J_SDK_ETH0].ip, flag);
            xmt_process_ip_addr(&net_cfg->Submask, (char*)net_info->network[J_SDK_ETH0].netmask, flag);
            xmt_process_ip_addr(&net_cfg->Gateway, (char*)net_info->network[J_SDK_ETH0].gateway, flag);

            strncpy((char*)net_cfg->sMac, 
                (const char*)net_info->network[J_SDK_ETH0].mac, 
                sizeof(net_cfg->sMac-1));

            /*strncpy((char*)net_cfg->struDnsServer1IpAddr.sIpV4, 
                (const char*)net_info->main_dns, 
                sizeof(net_cfg->struDnsServer1IpAddr.sIpV4)-1);
            strncpy((char*)net_cfg->struDnsServer2IpAddr.sIpV4, 
                (const char*)net_info->backup_dns, 
                sizeof(net_cfg->struDnsServer2IpAddr.sIpV4)-1);*/

            //net_cfg->stEtherNet[0].bValid = net_info->network[J_SDK_ETH0].dhcp_enable;

            net_cfg->TCPPort  = net_info->cmd_port;
            net_cfg->HttpPort = net_info->web_port;
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_dns_info(SDK_NetDNSConfig *dns_cfg, JNetworkInfo *net_info, int flag)
{
    NMP_ASSERT(dns_cfg && net_info);

    switch (flag)
    {
        case SWAP_PACK:
            xmt_process_ip_addr(&dns_cfg->PrimaryDNS, (char*)net_info->main_dns, flag);
            xmt_process_ip_addr(&dns_cfg->SecondaryDNS, (char*)net_info->backup_dns, flag);
            break;

        case SWAP_UNPACK:
            xmt_process_ip_addr(&dns_cfg->PrimaryDNS, (char*)net_info->main_dns, flag);
            xmt_process_ip_addr(&dns_cfg->SecondaryDNS, (char*)net_info->backup_dns, flag);
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_pppoe_info(SDK_NetPPPoEConfig *pppoe_cfg, JPPPOEInfo *pppoe_info, int flag)
{
    NMP_ASSERT(pppoe_cfg && pppoe_info);

    switch (flag)
    {
        case SWAP_PACK:
            pppoe_info->enable = pppoe_cfg->Enable;
            //pppoe_info->type   = pppoe_cfg->byAssistant;
            xmt_process_ip_addr(&pppoe_cfg->Server.ip, (char*)pppoe_info->ip, flag);
            strncpy((char*)pppoe_info->account, (const char*)pppoe_cfg->Server.UserName, 
                    sizeof(pppoe_info->account)-1);
            strncpy((char*)pppoe_info->passwd, (const char*)pppoe_cfg->Server.Password, 
                    sizeof(pppoe_info->passwd)-1);
            break;

        case SWAP_UNPACK:
            pppoe_cfg->Enable = pppoe_info->enable;
            /*if (0 == pppoe_info->type)
                pppoe_cfg->byAssistant = 0;
            else
                pppoe_cfg->byAssistant = 1;*/
            xmt_process_ip_addr(&pppoe_cfg->Server.ip, (char*)pppoe_info->ip, flag);
            strncpy((char*)pppoe_cfg->Server.UserName, (const char*)pppoe_info->account, 
                    sizeof(pppoe_cfg->Server.UserName)-1);
            strncpy((char*)pppoe_cfg->Server.Password, (const char*)pppoe_info->passwd, 
                    sizeof(pppoe_cfg->Server.Password)-1);
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_ftp_info(SDK_FtpServerConfig *ftp_cfg, JFTPParameter *ftp_info, int flag)
{
    NMP_ASSERT(ftp_cfg && ftp_info);

    switch (flag)
    {
        case SWAP_PACK:
            ftp_info->ftp_port = ftp_cfg->bEnable;
            xmt_process_ip_addr(&ftp_cfg->Server.ip, (char*)ftp_info->ftp_ip, flag);
            strncpy((char*)ftp_info->ftp_usr, (const char*)ftp_cfg->Server.UserName, 
                    sizeof(ftp_info->ftp_usr)-1);
            strncpy((char*)ftp_info->ftp_pwd, (const char*)ftp_cfg->Server.Password, 
                    sizeof(ftp_info->ftp_pwd)-1);
            strncpy((char*)ftp_info->ftp_path, (const char*)ftp_cfg->cRemoteDir, 
                    sizeof(ftp_info->ftp_path)-1);
            break;

        case SWAP_UNPACK:
            ftp_cfg->bEnable = ftp_info->ftp_port;
            xmt_process_ip_addr(&ftp_cfg->Server.ip, (char*)ftp_info->ftp_ip, flag);
            strncpy((char*)ftp_cfg->Server.UserName, (const char*)ftp_info->ftp_usr, 
                    sizeof(ftp_cfg->Server.UserName)-1);
            strncpy((char*)ftp_cfg->Server.Password, (const char*)ftp_info->ftp_pwd, 
                    sizeof(ftp_cfg->Server.Password)-1);
            strncpy((char*)ftp_cfg->cRemoteDir, (const char*)ftp_info->ftp_path, 
                    sizeof(ftp_cfg->cRemoteDir)-1);
            break;

        default:
            break;
    }
    
    return ;
}


void xmt_swap_smtp_info(SDK_NetEmailConfig *email_cfg, JSMTPParameter *smtp_info, int flag)
{
    NMP_ASSERT(email_cfg && smtp_info);

    switch (flag)
    {
        case SWAP_PACK:
            xmt_process_ip_addr(&email_cfg->Server.ip, (char*)smtp_info->mail_ip, flag);
            strncpy((char*)smtp_info->mail_usr, (const char*)email_cfg->Server.UserName, 
                    sizeof(smtp_info->mail_usr)-1);
            strncpy((char*)smtp_info->mail_pwd, (const char*)email_cfg->Server.Password, 
                    sizeof(smtp_info->mail_pwd)-1);            
            strncpy((char*)smtp_info->mail_addr, (const char*)email_cfg->SendAddr, 
                    sizeof(smtp_info->mail_addr)-1);
            strncpy((char*)smtp_info->mail_rctp1, (const char*)email_cfg->Recievers[0], 
                    sizeof(smtp_info->mail_rctp1)-1);
            strncpy((char*)smtp_info->mail_rctp2, (const char*)email_cfg->Recievers[1], 
                    sizeof(smtp_info->mail_rctp2)-1);
            strncpy((char*)smtp_info->mail_rctp3, (const char*)email_cfg->Recievers[2], 
                    sizeof(smtp_info->mail_rctp3)-1);
            smtp_info->mail_port  = email_cfg->Server.Port;
            smtp_info->ssl_enable = email_cfg->bUseSSL;
            break;

        case SWAP_UNPACK:
            email_cfg->Enable = 1;
            xmt_process_ip_addr(&email_cfg->Server.ip, (char*)smtp_info->mail_ip, flag);
            strncpy((char*)email_cfg->Server.UserName, (const char*)smtp_info->mail_usr, 
                    sizeof(email_cfg->Server.UserName)-1);
            strncpy((char*)email_cfg->Server.Password, (const char*)smtp_info->mail_pwd, 
                    sizeof(email_cfg->Server.Password)-1);
            strncpy((char*)email_cfg->SendAddr, (const char*)smtp_info->mail_addr, 
                    sizeof(email_cfg->SendAddr)-1);
            strncpy((char*)email_cfg->Recievers[0], (const char*)smtp_info->mail_rctp1, 
                    sizeof(email_cfg->Recievers[0])-1);
            strncpy((char*)email_cfg->Recievers[1], (const char*)smtp_info->mail_rctp2, 
                    sizeof(email_cfg->Recievers[1])-1);
            strncpy((char*)email_cfg->Recievers[2], (const char*)smtp_info->mail_rctp3, 
                    sizeof(email_cfg->Recievers[2])-1);
            email_cfg->Server.Port = smtp_info->mail_port;
            email_cfg->bUseSSL = smtp_info->ssl_enable;
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_ddns_info(SDK_NetDDNSConfigALL *ddns_cfg, JDdnsConfig *ddns_info, int flag)
{
    NMP_ASSERT(ddns_cfg && ddns_info);

    switch (flag)
    {
        /*case SWAP_PACK:
            strncpy((char*)ddns_info->ddns_account, (const char*)ddns_cfg->szDomainName, 
                    sizeof(ddns_info->ddns_account)-1);
            strncpy((char*)ddns_info->ddns_usr, (const char*)ddns_cfg->szUserName, 
                    sizeof(ddns_info->ddns_usr)-1);
            strncpy((char*)ddns_info->ddns_pwd, (const char*)ddns_cfg->szUserPsw, 
                    sizeof(ddns_info->ddns_pwd)-1);
            ddns_info->ddns_open  = ddns_cfg->bEnable;
            //ddns_info->ddns_type  = ddns_cfg->;
            printf("Ddns type: %s\n", ddns_cfg->szServerType);
            ddns_info->ddns_port  = ddns_cfg->dwServerPort;
            ddns_info->ddns_times  = ddns_cfg->dwAlivePeriod;
            break;

        case SWAP_UNPACK:
            strncpy((char*)ddns_cfg->szDomainName, (const char*)ddns_info->ddns_account, 
                    sizeof(ddns_cfg->szDomainName)-1);
            strncpy((char*)ddns_cfg->szUserName, (const char*)ddns_info->ddns_usr, 
                    sizeof(ddns_cfg->szUserName)-1);
            strncpy((char*)ddns_cfg->szUserPsw, (const char*)ddns_info->ddns_pwd, 
                    sizeof(ddns_cfg->szUserPsw)-1);
            ddns_cfg->bEnable = ddns_info->ddns_open;
            //ddns_cfg->szServerType = ddns_info->;
            ddns_cfg->dwServerPort = ddns_info->ddns_port;
            ddns_cfg->dwAlivePeriod = ddns_info->ddns_times;
            break;*/

        default:
            break;
    }

    return ;
}


void xmt_swap_upnp_info(SDK_NetUPNPConfig *upnp_cfg, JUPNPParameter *upnp_info, int flag)
{
    NMP_ASSERT(upnp_cfg&& upnp_info);

    switch (flag)
    {
        case SWAP_PACK:
            //strncpy((char*)upnp_info->upnp_ip, (const char*)upnp_cfg->, 
              //      sizeof(upnp_info->upnp_ip)-1);
            upnp_info->upnp_enable = upnp_cfg->bEnable;
            //upnp_info->upnp_eth_no  = upnp_cfg->;
            //upnp_info->upnp_model  = upnp_cfg->;
            //upnp_info->upnp_refresh_time  = upnp_cfg->;
            upnp_info->upnp_data_port = upnp_cfg->iMediaPort;
            upnp_info->upnp_web_port  = upnp_cfg->iHTTPPort;
            upnp_info->upnp_data_port_result = upnp_cfg->bState;
            upnp_info->upnp_web_port_result = upnp_cfg->bState;
            /*upnp_info->upnp_cmd_port  = upnp_cfg->;
            upnp_info->upnp_talk_port  = upnp_cfg->;
            upnp_info->upnp_cmd_port_result  = upnp_cfg->;
            upnp_info->upnp_talk_port_result  = upnp_cfg->;*/
            break;

        case SWAP_UNPACK:
            upnp_cfg->bEnable    = upnp_info->upnp_enable;
            upnp_cfg->iMediaPort = upnp_info->upnp_data_port;
            upnp_cfg->iHTTPPort  = upnp_info->upnp_web_port;
            if (upnp_info->upnp_web_port_result && 
                upnp_info->upnp_data_port_result)
                upnp_cfg->bState = 1;
            else
                upnp_cfg->bState = 0;
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_disk_list(SDK_StorageDeviceInformationAll *xmt_disk, JDeviceDiskInfo *dev_disk, int flag)
{
    int i, j, count = 0;
    SDK_DriverInformation *partitions, *tmp;
    NMP_ASSERT(xmt_disk && dev_disk);

    switch (flag)
    {
        case SWAP_PACK:
            for (i=0; i<xmt_disk->iDiskNumber; i++)
            {
                for (j=0; j<xmt_disk->vStorageDeviceInfoAll[i].iPartNumber; j++)
                {
                    partitions = &xmt_disk->vStorageDeviceInfoAll[i].diPartitions[j];
                    if (j)
                    {
                        tmp = &xmt_disk->vStorageDeviceInfoAll[i].diPartitions[j];
                        partitions->uiTotalSpace  += tmp->uiTotalSpace;
                        partitions->uiRemainSpace += tmp->uiRemainSpace;
                    }
                }
                if (partitions)
                {
                    count++;
                    dev_disk->disk[i].disk_no    = xmt_disk->vStorageDeviceInfoAll[i].iPhysicalNo;
                    dev_disk->disk[i].disk_type  = partitions->iDriverType;
                    dev_disk->disk[i].status     = partitions->bIsCurrent;
                    dev_disk->disk[i].total_size = partitions->uiTotalSpace;
                    dev_disk->disk[i].free_size  = partitions->uiRemainSpace;
                    //dev_disk->disk[i].is_backup = hd_cfg->struHDInfo[i].byMode;
                    //dev_disk->disk[i].sys_file_type = hd_cfg->struHDInfo[i].byMode;*/
                }
            }
            dev_disk->disk_num = count;
            break;

        case SWAP_UNPACK:
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_format_info(SDK_StorageDeviceControl *storage_ctl, 
    SDK_StorageDeviceInformationAll *dh_disk, JFormatDisk *fmt_disk)
{
    int i;
    NMP_ASSERT(storage_ctl && dh_disk && fmt_disk);

    storage_ctl->iAction   = SDK_STORAGE_DEVICE_CONTROL_CLEAR;
    storage_ctl->iSerialNo = fmt_disk->disk_no;
    storage_ctl->iType     = SDK_STORAGE_DEVICE_CLEAR_DATA;

    for (i=0; i<dh_disk->iDiskNumber; i++)
    {
        if ((int)fmt_disk->disk_no == dh_disk->vStorageDeviceInfoAll[i].iPhysicalNo)
        {
            storage_ctl->iPartSize[0] = dh_disk->vStorageDeviceInfoAll[i].diPartitions[0].uiTotalSpace;
            storage_ctl->iPartSize[1] = dh_disk->vStorageDeviceInfoAll[i].diPartitions[1].uiTotalSpace;
        }
    }

    return ;
}


void xmt_swap_encode_info(SDK_CONFIG_ENCODE *enc_cfg, JEncodeParameter *enc_info, int flag)
{
    SDK_MEDIA_FORMAT *mf;
    NMP_ASSERT(enc_cfg && enc_info);

    switch (flag)
    {
        case SWAP_PACK:
            mf = &enc_cfg->dstMainFmt[0];
            enc_info->frame_rate = mf->vfFormat.nFPS;

            switch (mf->vfFormat.iResolution)
            {
            case SDK_CAPTURE_SIZE_QCIF:
                enc_info->resolution = J_SDK_VIDEO_QCIF;
                break;
            case SDK_CAPTURE_SIZE_CIF:
                enc_info->resolution = J_SDK_VIDEO_CIF;
                break;
            case SDK_CAPTURE_SIZE_HD1:
                enc_info->resolution = J_SDK_VIDEO_HD1;
                break;
            case SDK_CAPTURE_SIZE_D1:
                enc_info->resolution = J_SDK_VIDEO_D1;
                break;
            case SDK_CAPTURE_SIZE_QQVGA:
                enc_info->resolution = J_SDK_VIDEO_QQVGA;
                break;
            case SDK_CAPTURE_SIZE_QVGA:
                enc_info->resolution = J_SDK_VIDEO_QVGA;
                break;
            case SDK_CAPTURE_SIZE_VGA:
                enc_info->resolution = J_SDK_VIDEO_VGA;
                break;
            case SDK_CAPTURE_SIZE_SVCD:
                enc_info->resolution = J_SDK_VIDEO_SVGA;
                break;
            case SDK_CAPTURE_SIZE_UXGA:
                enc_info->resolution = J_SDK_VIDEO_UXGA;
                break;
            case SDK_CAPTURE_SIZE_720P:
                enc_info->resolution = J_SDK_VIDEO_720P;
                break;
            case SDK_CAPTURE_SIZE_1_3M:
                enc_info->resolution = J_SDK_VIDEO_960P;
                break;
            case SDK_CAPTURE_SIZE_1080P:
                enc_info->resolution = J_SDK_VIDEO_1080P;
                break;
            }

            if (mf->vfFormat.iQuality)
                enc_info->qp_value = mf->vfFormat.iQuality - 1;
            else
                enc_info->qp_value = 0;

            /*switch (enc_cfg->byEncodeMode)
            {
            case DH_CAPTURE_COMP_H264:
                enc_info->video_type = J_SDK_AV_VIDEO_H264;
                break;
            case DH_CAPTURE_COMP_DIVX_MPEG4:
            case DH_CAPTURE_COMP_MS_MPEG4:
            case DH_CAPTURE_COMP_MPEG2:
            case DH_CAPTURE_COMP_MPEG1:
                enc_info->video_type = J_SDK_AV_VIDEO_MPEG4;
                break;
            case DH_CAPTURE_COMP_MJPG:
                enc_info->video_type = J_SDK_AV_VIDEO_MJPEG;
                break;
            }*/

            //enc_info->audio_type       = mf->;
            enc_info->audio_enble      = mf->bAudioEnable;
            enc_info->i_frame_interval = mf->vfFormat.iGOP;
            enc_info->code_rate        = mf->vfFormat.nBitRate;

            switch (mf->vfFormat.iBitRateControl)
            {
                case SDK_CAPTURE_BRC_CBR:
                    enc_info->bit_rate = J_SDK_CBR;
                    break;
                case SDK_CAPTURE_BRC_VBR:
                    enc_info->bit_rate = J_SDK_VBR;
                    break;
                case SDK_CAPTURE_BRC_MBR:
                    enc_info->bit_rate = J_SDK_ABR;
                    break;
            }
            break;

        case SWAP_UNPACK:
            mf = &enc_cfg->dstMainFmt[0];
            mf->vfFormat.nFPS = enc_info->frame_rate;

            switch (enc_info->resolution)
            {
            case J_SDK_VIDEO_QCIF:
                mf->vfFormat.iResolution = SDK_CAPTURE_SIZE_QCIF;
                break;
            case J_SDK_VIDEO_CIF:
                mf->vfFormat.iResolution = SDK_CAPTURE_SIZE_CIF;
                break;
            case J_SDK_VIDEO_HD1:
                mf->vfFormat.iResolution = SDK_CAPTURE_SIZE_HD1;
                break;
            case J_SDK_VIDEO_D1:
                mf->vfFormat.iResolution = SDK_CAPTURE_SIZE_D1;
                break;
            case J_SDK_VIDEO_QQVGA:
                mf->vfFormat.iResolution = SDK_CAPTURE_SIZE_QQVGA;
                break;
            case J_SDK_VIDEO_QVGA:
                mf->vfFormat.iResolution = SDK_CAPTURE_SIZE_QVGA;
                break;
            case J_SDK_VIDEO_VGA:
                mf->vfFormat.iResolution = SDK_CAPTURE_SIZE_VGA;
                break;
            case J_SDK_VIDEO_SVGA:
                mf->vfFormat.iResolution = SDK_CAPTURE_SIZE_SVCD;
                break;
            case J_SDK_VIDEO_UXGA:
                mf->vfFormat.iResolution = SDK_CAPTURE_SIZE_UXGA;
                break;
            case J_SDK_VIDEO_720P:
                mf->vfFormat.iResolution = SDK_CAPTURE_SIZE_720P;
                break;
            case J_SDK_VIDEO_960P:
                mf->vfFormat.iResolution = SDK_CAPTURE_SIZE_1_3M;
                break;
            case J_SDK_VIDEO_1080P:
                mf->vfFormat.iResolution = SDK_CAPTURE_SIZE_1080P;
                break;
            }
            
            mf->vfFormat.iQuality = enc_info->qp_value + 1;
            mf->bAudioEnable      = enc_info->audio_enble;
            mf->vfFormat.iGOP     = enc_info->i_frame_interval;
            mf->vfFormat.nBitRate = enc_info->code_rate;

            switch (enc_info->bit_rate)
            {
                case J_SDK_CBR:
                    mf->vfFormat.iBitRateControl = SDK_CAPTURE_BRC_CBR;
                    break;
                case J_SDK_VBR:
                    mf->vfFormat.iBitRateControl = SDK_CAPTURE_BRC_VBR;
                    break;
                case J_SDK_ABR:
                    mf->vfFormat.iBitRateControl = SDK_CAPTURE_BRC_MBR;
                    break;
            }
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_display_info(SDK_VIDEOCOLOR_PARAM *color, JDisplayParameter *disp_info, int flag)
{
    NMP_ASSERT(color && disp_info);

    switch (flag)
    {
        case SWAP_PACK:
            disp_info->bright     = color->nBrightness*2.5;
            disp_info->contrast   = color->nContrast*2.5;
            disp_info->saturation = color->nSaturation*2.5;
            disp_info->hue        = color->nHue*2.5;
            disp_info->sharpness  = color->nAcutance*17;
            break;

        case SWAP_UNPACK:
            color->nBrightness = disp_info->bright/2.5;
            color->nContrast   = disp_info->contrast/2.5;
            color->nSaturation = disp_info->saturation/2.5;
            color->nHue        = disp_info->hue/2.5;
            color->nAcutance   = disp_info->sharpness/17;
            break;

        default:
            break;
    }
    printf("nBrightness: %02d  |  ", color->nBrightness);
    printf("bright    : %d\n", disp_info->bright);
    printf("nContrast  : %02d  |  ", color->nContrast);
    printf("contrast  : %d\n", disp_info->contrast);
    printf("nSaturation: %02d  |  ", color->nSaturation);
    printf("saturation: %d\n", disp_info->saturation);
    printf("nHue       : %02d  |  ", color->nHue);
    printf("hue       : %d\n", disp_info->hue);
    printf("nAcutance  : %02d  |  ", color->nAcutance);
    printf("sharpness : %d\n", disp_info->sharpness);

    return ;
}


void xmt_swap_osd_info(SDK_CONFIG_VIDEOWIDGET *osd_cfg, JOSDParameter *osd_info, int flag)
{
    NMP_ASSERT(osd_cfg && osd_info);

    switch (flag)
    {
        case SWAP_PACK:
            osd_info->time_enable    = osd_cfg->TimeTitle.bEncodeBlend;
            osd_info->time_display_x = osd_cfg->TimeTitle.rcRelativePos.left * 704/8191;
            osd_info->time_display_y = osd_cfg->TimeTitle.rcRelativePos.top * 576/8191;
            osd_info->text_enable    = osd_cfg->ChannelTitle.bEncodeBlend;
            osd_info->text_display_x = osd_cfg->ChannelTitle.rcRelativePos.left  * 704/8191;
            osd_info->text_display_y = osd_cfg->ChannelTitle.rcRelativePos.top * 576/8191;
            osd_info->max_width      = 704;
            osd_info->max_height     = 576;
            strncpy((char*)osd_info->text_data, (const char*)osd_cfg->ChannelName.strName, 
                sizeof(osd_info->text_data)-1);
            break;

        case SWAP_UNPACK:
            osd_cfg->TimeTitle.bEncodeBlend          = osd_info->time_enable;
            osd_cfg->TimeTitle.rcRelativePos.left    = osd_info->time_display_x * 8191/704;
            osd_cfg->TimeTitle.rcRelativePos.top     = osd_info->time_display_y * 8191/576;
            osd_cfg->ChannelTitle.bEncodeBlend       = osd_info->text_enable;
            osd_cfg->ChannelTitle.rcRelativePos.left = osd_info->text_display_x * 8191/704;
            osd_cfg->ChannelTitle.rcRelativePos.top  = osd_info->text_display_y * 8191/576;
            strncpy((char*)osd_cfg->ChannelName.strName, (const char*)osd_info->text_data, 
                sizeof(osd_cfg->ChannelName.strName)-1);
            break;

        default:
            break;
    }
    printf("TimShow    : %d  |  ", osd_cfg->TimeTitle.bEncodeBlend);
    printf("time_enable    : %d\n", osd_info->time_enable);
    printf("ChannelShow: %d  |  ", osd_cfg->ChannelTitle.bEncodeBlend);
    printf("text_enable    : %d\n", osd_info->text_enable);

    return ;
}


static __inline__ void
xmt_process_sched_time(SDK_TIMESECTION (*xmt_sched_time)[NET_N_TSECT], 
        JWeek *week, int flag)
{
    int day_index, time_seg_index;
    JTime *time_start, *time_end;
    SDK_TIMESECTION *time_section;

    switch (flag)
    {
        case SWAP_PACK:         
            for (day_index=0; day_index<NET_N_WEEKS; day_index++)
            {
            printf("day: %d\n", day_index);
                for (time_seg_index=0; time_seg_index<J_SDK_MAX_SEG_SZIE; 
                    time_seg_index++)
                {
                    week->days[day_index].seg[time_seg_index].enable = 1;

                    time_start = &(week->days[day_index].seg[time_seg_index].time_start);
                    time_end = &(week->days[day_index].seg[time_seg_index].time_end);

                    time_section = &xmt_sched_time[day_index][time_seg_index];

                    if (time_section)
                    {
                        time_start->hour   = time_section->startHour;
                        time_start->minute = time_section->startMinute;
                        time_start->second = time_section->startSecond;

                        time_end->hour   = time_section->endHour;
                        time_end->minute = time_section->endMinute;
                        time_end->second = time_section->endSecond;
                        
                        printf("AlarmTime: %02d:%02d - %02d:%02d  |  ", 
                            time_section->startHour, time_section->startMinute, 
                            time_section->endHour, time_section->endMinute);
                        printf("time_seg: %02d:%02d - %02d:%02d\n", 
                            time_start->hour, time_start->minute, 
                            time_end->hour, time_end->minute);
                    }
                    else
                        break;
                }
                week->days[day_index].day_id = day_index;
                week->days[day_index].count = time_seg_index;
            }
            week->count = day_index;
            break;

        case SWAP_UNPACK:
            for (day_index=0; day_index<NET_N_WEEKS; day_index++)
            {
            printf("UNPACK--->day: %d\n", day_index);
                int seg = week->days[day_index].count;
                for (time_seg_index=0; (time_seg_index<J_SDK_MAX_SEG_SZIE && 0 < seg--); 
                    time_seg_index++)
                {
                    time_start = &(week->days[day_index].seg[time_seg_index].time_start);
                    time_end = &(week->days[day_index].seg[time_seg_index].time_end);

                    time_section = &xmt_sched_time[day_index][time_seg_index];

                    time_section->startHour   = time_start->hour;
                    time_section->startMinute = time_start->minute;
                    time_section->startSecond   = time_start->minute;

                    time_section->endHour   = time_end->hour;
                    time_section->endMinute = time_end->minute;
                    time_section->endSecond   = time_end->minute;

                    printf("AlarmTime: %02d:%02d - %02d:%02d  |  ", 
                        time_section->startHour, time_section->startMinute, 
                        time_section->endHour, time_section->endMinute);
                    printf("time_seg: %02d:%02d - %02d:%02d\n", 
                        time_start->hour, time_start->minute, 
                        time_end->hour, time_end->minute);
                }
            }
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_record_info(SDK_RECORDCONFIG *rec_cfg, JRecordParameter *rec_info, int flag)
{
    NMP_ASSERT(rec_cfg && rec_info);

    switch (flag)
    {
        case SWAP_PACK:
            rec_info->pre_record = rec_cfg->iPreRecord;
            rec_info->auto_cover = rec_cfg->bRedundancy;
            xmt_process_sched_time(rec_cfg->wcWorkSheet.tsSchedule, &rec_info->week, flag);
            break;

        case SWAP_UNPACK:
            rec_cfg->iPreRecord  = rec_info->pre_record;
            rec_cfg->bRedundancy = rec_info->auto_cover;
            rec_cfg->iRecordMode = 2;
            memset(&rec_cfg->wcWorkSheet, 0, sizeof(rec_cfg->wcWorkSheet));
            xmt_process_sched_time(rec_cfg->wcWorkSheet.tsSchedule, &rec_info->week, flag);
            memset(&rec_cfg->typeMask, 7, sizeof(rec_cfg->typeMask));
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_move_alarm_info(SDK_MOTIONCONFIG *xmt_motion, JMoveAlarm *move_alarm, int flag)
{
    NMP_ASSERT(xmt_motion && move_alarm);

    switch (flag)
    {
        case SWAP_PACK:
            move_alarm->move_enable     = xmt_motion->bEnable;
            move_alarm->sensitive_level = xmt_motion->iLevel-1;
            //move_alarm->detect_interval = xmt_motion->;
            //move_alarm->detect_area = xmt_motion->byDetected;// 检测区域，最多32*32块区域
            move_alarm->max_width       = 704;
            move_alarm->max_height      = 576;
            xmt_process_sched_time(xmt_motion->hEvent.schedule.tsSchedule, &move_alarm->week, flag);
            break;

        case SWAP_UNPACK:
            xmt_motion->bEnable = move_alarm->move_enable;
            xmt_motion->iLevel  = move_alarm->sensitive_level+1;
            memset(&xmt_motion->hEvent.schedule, 0, sizeof(xmt_motion->hEvent.schedule));
            xmt_process_sched_time(xmt_motion->hEvent.schedule.tsSchedule, &move_alarm->week, flag);
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_video_lost_info(SDK_VIDEOLOSSCONFIG *xmt_video_lost, JLostAlarm *lost_alarm, int flag)
{
    NMP_ASSERT(xmt_video_lost && lost_alarm);

    switch (flag)
    {
        case SWAP_PACK:
            lost_alarm->lost_enable     = xmt_video_lost->bEnable;
            xmt_process_sched_time(xmt_video_lost->hEvent.schedule.tsSchedule, &lost_alarm->week, flag);
            break;

        case SWAP_UNPACK:
            xmt_video_lost->bEnable = lost_alarm->lost_enable;
            memset(&xmt_video_lost->hEvent.schedule, 0, sizeof(xmt_video_lost->hEvent.schedule));
            xmt_process_sched_time(xmt_video_lost->hEvent.schedule.tsSchedule, &lost_alarm->week, flag);
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_hide_alarm_info(SDK_BLINDDETECTCONFIG *xmt_hide_alarm, JHideAlarm *hide_alarm, int flag)
{
    NMP_ASSERT(xmt_hide_alarm && hide_alarm);

    switch (flag)
    {
        case SWAP_PACK:
            hide_alarm->hide_enable     = xmt_hide_alarm->bEnable;
            hide_alarm->sensitive_level = xmt_hide_alarm->iLevel;
            hide_alarm->max_width       = 704;
            hide_alarm->max_height      = 576;
            xmt_process_sched_time(xmt_hide_alarm->hEvent.schedule.tsSchedule, &hide_alarm->week, flag);
            break;

        case SWAP_UNPACK:
            xmt_hide_alarm->bEnable = hide_alarm->hide_enable;
            xmt_hide_alarm->iLevel = 3;//hide_alarm->sensitive_level;
            memset(&xmt_hide_alarm->hEvent.schedule, 0, sizeof(xmt_hide_alarm->hEvent.schedule));
            xmt_process_sched_time(xmt_hide_alarm->hEvent.schedule.tsSchedule, &hide_alarm->week, flag);
            break;

        default:
            break;
    }

    return ;
}


void xmt_swap_alarm_in_info(SDK_ALARM_INPUTCONFIG *xmt_alarm_in, 
        JIoAlarm *io_alarm, int flag)
{
    NMP_ASSERT(xmt_alarm_in && io_alarm);

    switch (flag)
    {
        case SWAP_PACK:
            io_alarm->alarm_enable = xmt_alarm_in->bEnable;
            xmt_process_sched_time(xmt_alarm_in->hEvent.schedule.tsSchedule, &io_alarm->week, flag);
            break;
            
        case SWAP_UNPACK:
            xmt_alarm_in->bEnable = io_alarm->alarm_enable;
            xmt_alarm_in->iSensorType = io_alarm->alarm_enable;
            memset(&xmt_alarm_in->hEvent.schedule, 0, sizeof(xmt_alarm_in->hEvent.schedule));
            xmt_process_sched_time(xmt_alarm_in->hEvent.schedule.tsSchedule, &io_alarm->week, flag);
            break;
            
        default:
            break;
    }

    return ;
}


void xmt_swap_alarm_out_info(SDK_AlarmOutConfig *xmt_alarm_out, 
        JIoAlarm *io_alarm, int flag)
{
    NMP_ASSERT(xmt_alarm_out && io_alarm);

    switch (flag)
    {
        case SWAP_PACK:
            break;
            
        case SWAP_UNPACK:
            break;
            
        default:
            break;
    }

    return ;
}


void xmt_swap_hide_info(SDK_BLINDDETECTCONFIG *vc_cfg, JHideParameter *hide_info, int flag)
{
    /*int i;
    NMP_ASSERT(vc_cfg && hide_info);

    switch (flag)
    {
        case SWAP_PACK:
            for (i=0; i<(int)vc_cfg->bCoverCount; i++)
            {
                //if (vc_cfg->CoverBlock[i].bPriview)
                if (vc_cfg->CoverBlock[i].bEncode)
                    hide_info->hide_enable = 1;
                
                hide_info->hide_area.rect[i].left   = vc_cfg->CoverBlock[i].rcBlock.left * 704/8192;
                hide_info->hide_area.rect[i].top    = vc_cfg->CoverBlock[i].rcBlock.top * 576/8192;
                hide_info->hide_area.rect[i].right  = vc_cfg->CoverBlock[i].rcBlock.right * 704/8192;
                hide_info->hide_area.rect[i].bottom = vc_cfg->CoverBlock[i].rcBlock.bottom * 576/8192;
            }
            hide_info->hide_area.count = i;
            hide_info->max_width       = 704;
            hide_info->max_height      = 576;
            break;

        case SWAP_UNPACK:
            memset(vc_cfg->CoverBlock, 0, sizeof(vc_cfg->CoverBlock));
            for (i=0; i<(int)hide_info->hide_area.count; i++)
            {
                vc_cfg->CoverBlock[i].bBlockType = 0;
                vc_cfg->CoverBlock[i].bEncode    = hide_info->hide_enable;
                vc_cfg->CoverBlock[i].bPriview   = hide_info->hide_enable;

                vc_cfg->CoverBlock[i].rcBlock.left   = hide_info->hide_area.rect[i].left * 8192/704;
                vc_cfg->CoverBlock[i].rcBlock.top    = hide_info->hide_area.rect[i].top * 8192/576;
                vc_cfg->CoverBlock[i].rcBlock.right  = hide_info->hide_area.rect[i].right * 8192/704;
                vc_cfg->CoverBlock[i].rcBlock.bottom = hide_info->hide_area.rect[i].bottom * 8192/576;
            }
            vc_cfg->bCoverCount = i;
            break;

        default:
            break;
    }*/
//  printf("dwEnableHide: %02d   |  ", osd_cfg->byBlindEnable);
//  printf("hide_enable    : %d\n", hide_info->hide_enable);

    return ;
}


void xmt_swap_ptz_info(SDK_STR_CONFIG_PTZ *ptz_cfg, JPTZParameter *serial_info, int flag)
{
    int i;
    int baud_rate[] = {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
    NMP_ASSERT(ptz_cfg && serial_info);

    switch (flag)
    {
        case SWAP_PACK:
            if (!strcmp("PELCOD", ptz_cfg->sProtocolName))
                serial_info->protocol  = 0;
            else// if (!strcmp("PELCOP", ptz_cfg->sProtocolName))
                serial_info->protocol  = 1;
            serial_info->ptz_addr  = ptz_cfg->ideviceNo;
            serial_info->baud_rate = baud_rate[ptz_cfg->dstComm.iBaudRate];
            serial_info->data_bit  = ptz_cfg->dstComm.iDataBits;
            serial_info->stop_bit  = ptz_cfg->dstComm.iStopBits;
            serial_info->verify    = ptz_cfg->dstComm.iParity;
            break;

        case SWAP_UNPACK:
            ptz_cfg->ideviceNo = serial_info->ptz_addr;
            if (0 == serial_info->protocol)
                strcpy(ptz_cfg->sProtocolName, "PELCOD");
            else// if (1 == serial_info->protocol)
                strcpy(ptz_cfg->sProtocolName, "PELCOP");
            for (i=0; i<10; i++)
            {
                if ((int)serial_info->baud_rate <= baud_rate[i])
                {
                    ptz_cfg->dstComm.iBaudRate = i;
                    break;
                }
            }
            ptz_cfg->dstComm.iDataBits = serial_info->data_bit;
            ptz_cfg->dstComm.iStopBits = serial_info->stop_bit;
            ptz_cfg->dstComm.iParity   = serial_info->verify;
            break;

        default:
            break;
    }

    return ;
}



void xmt_swap_ptz_cmd(xmt_ptz_ctrl_t *xmt_ptz_ctrl, JPTZControl *ptz_ctrl, int flag)
{
    NMP_ASSERT(xmt_ptz_ctrl && ptz_ctrl);

    switch (flag)
    {
        case SWAP_UNPACK:
            switch (ptz_ctrl->action)
            {
                case PTZ_STOP:
                    xmt_ptz_ctrl->ptz_cmd = EXTPTZ_CLOSELINESCAN;
                    break;
                case PTZ_AUTO:
                    xmt_ptz_ctrl->ptz_cmd = EXTPTZ_STARTLINESCAN;
                    break;
                case PTZ_LEFT:
                    xmt_ptz_ctrl->ptz_cmd = PAN_LEFT;
                    break;
                case PTZ_RIGHT:
                    xmt_ptz_ctrl->ptz_cmd = PAN_RIGHT;
                    break;
                case PTZ_UP:
                    xmt_ptz_ctrl->ptz_cmd = TILT_UP;
                    break;
                case PTZ_DOWN:
                    xmt_ptz_ctrl->ptz_cmd = TILT_DOWN;
                    break;
                case PTZ_LEFT_UP:
                    xmt_ptz_ctrl->ptz_cmd = PAN_LEFTTOP;
                    break;
                case PTZ_LEFT_DOWN:
                    xmt_ptz_ctrl->ptz_cmd = PAN_LEFTDOWN;
                    break;
                case PTZ_RIGHT_UP:
                    xmt_ptz_ctrl->ptz_cmd = PAN_RIGTHTOP;
                    break;
                case PTZ_RIGHT_DOWN:
                    xmt_ptz_ctrl->ptz_cmd = PAN_RIGTHDOWN;
                    break;
                case PTZ_ADD_ZOOM:
                    xmt_ptz_ctrl->ptz_cmd = ZOOM_IN;
                    break;
                case PTZ_SUB_ZOOM:
                    xmt_ptz_ctrl->ptz_cmd = ZOOM_OUT;
                    break;
                case PTZ_ADD_FOCUS:
                    xmt_ptz_ctrl->ptz_cmd = FOCUS_FAR;
                    break;
                case PTZ_SUB_FOCUS:
                    xmt_ptz_ctrl->ptz_cmd = FOCUS_NEAR;
                    break;
                case PTZ_TURN_ON:
                    xmt_ptz_ctrl->ptz_cmd = EXTPTZ_LAMP_ON;
                    break;
                case PTZ_TURN_OFF:
                    xmt_ptz_ctrl->ptz_cmd = EXTPTZ_LAMP_OFF;
                    break;
                /*case PTZ_WIPERS_ON:
                    xmt_ptz_ctrl->ptz_cmd = ;
                    break;
                case PTZ_WIPERS_OFF:
                    xmt_ptz_ctrl->ptz_cmd = ;*/
                    break;
            }
            break;
        default:
            break;
    }
    
    return ;
}


void xmt_swap_store_log_info(xmt_query_t *query_record, JStoreLog *store_log, int flag)
{
    int i, type, count = 0;
    H264_DVR_FILE_DATA *file_data;

    NMP_ASSERT(query_record && store_log);

    switch (flag)
    {
        case SWAP_PACK:
            file_data = (H264_DVR_FILE_DATA*)query_record->buffer;
            for (i=0; i<query_record->filecount; i++)
            {
                type = (int)file_data[i].hWnd;
                if ((int)store_log->beg_node <= i && 
                    (int)store_log->end_node >= i && 
                    (int)store_log->rec_type & type)
                {
                    if (count >= J_SDK_MAX_STORE_LOG_SIZE)
                        break;
                    store_log->store[count].rec_type = type;
                    store_log->store[count].file_size = file_data[i].size * 1024;    //byte

                    xmt_swap_sdk_system_time(&file_data[i].stBeginTime, 
                        &store_log->store[count].beg_time, SWAP_PACK);
                    xmt_swap_sdk_system_time(&file_data[i].stEndTime, 
                        &store_log->store[count].end_time, SWAP_PACK);
                    count++;
                }
            }
            store_log->node_count = count;
            store_log->total_count = query_record->filecount;
            break;

        case SWAP_UNPACK:
            xmt_swap_h264_dvr_time(&query_record->start, &store_log->beg_time, SWAP_UNPACK);
            xmt_swap_h264_dvr_time(&query_record->end, &store_log->end_time, SWAP_UNPACK);
            break;

        default:
            break;
    }

    return ;
}








